#include "Recorder.h"
#include "RecordBuffers.h"
#include "RecordingClip.h"
#include "StartRecordingProcess.h"
#include "Track.h"
#include "Project.h"
#include "Clip.h"
#include "AudioSystem.h"
#include "AudioInterface.h"
#include "SetTrackStateProcesses.h"
#include "DAW.h"
#include "AudioEngine.h"
#include "Date.h"
#include "Exception.h"
#include "RIFF.h"
#include "WAVFile.h"
#include "SampleConversion.h"
#include "Logger.h"
#include <sstream>
#include <string.h>

enum {
	num_record_buffers = 16,
	};


Recorder::Recorder()
{
}


Recorder::~Recorder()
{
}


void Recorder::arm_track(Track* track, Web::Connection* reply_connection)
{
	auto& armed_track = armed_tracks.emplace(track->id, track).first->second;
	std::vector<int>* capture_channels = nullptr;
	if (armed_track.capture_channels)
		capture_channels = new std::vector<int>(*armed_track.capture_channels);
	engine->start_process(
		new ArmTrackProcess(track, reply_connection, capture_channels));
}


void Recorder::unarm_track(Track* track, Web::Connection* reply_connection)
{
	armed_tracks.erase(track->id);
	engine->start_process(new UnarmTrackProcess(track, reply_connection));
}


void Recorder::set_track_input(Track* track, std::string input, Web::Connection* reply_connection)
{
	std::vector<int>* capture_channels = nullptr;
	auto it = armed_tracks.find(track->id);
	if (it != armed_tracks.end()) {
		auto& armed_track = it->second;
		delete armed_track.capture_channels;
		armed_track.capture_channels = nullptr;
		AudioInterface* interface = audio_system->selected_interface();
		if (interface) {
			armed_track.capture_channels =
				interface->capture_channels_for_input_name(input);
			if (armed_track.capture_channels)
				capture_channels = new std::vector<int>(*armed_track.capture_channels);
			}
		}
	engine->start_process(
		new SetTrackInputProcess(track, input, reply_connection, capture_channels));
}


void Recorder::start()
{
	std::vector<RecordingClip>* recording_clips = new std::vector<RecordingClip>;
	Project* project = daw->cur_project();

	// Open the files.
	std::string date = compact_iso8601_date();
	for (auto& track_pair: armed_tracks) {
		ArmedTrack& track = track_pair.second;

		// Build the file name.
		// Someday we'll allow this to be user-configurable, but for now we use:
		// "Audio/<track-name>-<track-id>-<iso-8601-date>.wav".
		// The track-id is included to disambiguate tracks with the same name.
		std::stringstream file_name;
		file_name << "Audio/";
		file_name << track.track->get_name() << '-' << track.track->id << '-';
		file_name << date << ".wav";

		// Open the file.
		track.create_wav_file(project->get_dir_path() + "/" + file_name.str());
		if (track.file == nullptr)
			continue;

		// Create a Clip for the file.
		AudioFile* audio_file = new AudioFile(project, project->new_id());
		project->add_file(audio_file);
		audio_file->path = file_name.str();
		Clip* clip = new Clip(audio_file);
		clip->start = 0;	// Will be filled in by the engine.
		clip->length_in_frames = 0;	// Will be filled in by the engine.
		recording_clips->emplace_back(track.track, clip);
		}

	StartRecordingProcess* process = new StartRecordingProcess(recording_clips);

	// Supply the engine with RecordBuffers.
	for (int i = 0; i < num_record_buffers; ++i)
		process->add_recording_buffers(new RecordBuffers(armed_tracks.size()));

	// Start recording.
	engine->start_process(process);
}


void Recorder::stop()
{
	// Close the files.
	for (auto& track_pair: armed_tracks) {
		ArmedTrack& track = track_pair.second;
		track.finish_wav_file();
		/***/
		}
}


void Recorder::interface_changed()
{
	Project* project = daw->cur_project();
	armed_tracks.clear();
	project->arm_armed_tracks(this);
}


void Recorder::project_changed()
{
	Project* project = daw->cur_project();
	armed_tracks.clear();
	project->arm_armed_tracks(this);
}


void Recorder::write_buffers(RecordBuffers* record_buffers)
{
	// Make sure capture_buffers is big enough.
	AudioInterface* interface = audio_system->selected_interface();
	if (!interface)
		return;
	capture_buffers.resize(interface->get_num_capture_channels());

	// Populate capture_buffers.
	for (auto& buffer: *record_buffers)
		capture_buffers[buffer.capture_channel] = buffer.buffer;

	// Allocate a write buffer.
	int max_channels = 0;
	for (auto& track_pair: armed_tracks) {
		ArmedTrack& track = track_pair.second;
		if (track.capture_channels == nullptr)
			continue;
		int num_channels = track.capture_channels->size();
		if (num_channels > max_channels)
			max_channels = num_channels;
		}
	int max_bytes_per_sample = 3;
	char* write_buffer =
		(char*) malloc(
			max_channels * max_bytes_per_sample * engine->buffer_size());

	// Write to each file.
	for (auto& track_pair: armed_tracks) {
		ArmedTrack& track = track_pair.second;
		if (track.file == nullptr || track.capture_channels == nullptr)
			continue;
		// Fill the write buffer.
		int bytes_per_sample = 3;
		char* channel_start = write_buffer;
		int num_channels = track.capture_channels->size();
		int step = num_channels * bytes_per_sample;
		for (int capture_channel: *track.capture_channels) {
			to_24_le(
				capture_buffers[capture_channel]->samples, channel_start,
				engine->buffer_size(), step);
			channel_start += bytes_per_sample;
			}
		// Write.
		int bytes_to_write =
			num_channels * bytes_per_sample * engine->buffer_size();
		fwrite(write_buffer, 1, bytes_to_write, track.file);
		}

	free(write_buffer);

	// Clear capture_buffers.
	for (auto it = capture_buffers.begin(); it != capture_buffers.end(); ++it)
		*it = nullptr;
}


Recorder::ArmedTrack::ArmedTrack(Track* track_in)
	: track(track_in), capture_channels(nullptr), file(nullptr)
{
	update_capture_channels();
}


Recorder::ArmedTrack::~ArmedTrack()
{
	delete capture_channels;
	if (file)
		fclose(file);
}


void Recorder::ArmedTrack::update_capture_channels()
{
	delete capture_channels;
	AudioInterface* interface = audio_system->selected_interface();
	if (interface) {
		capture_channels =
			interface->capture_channels_for_input_name(track->get_input());
		}
	else
		capture_channels = nullptr;
}


void Recorder::ArmedTrack::create_wav_file(std::string file_name)
{
	file = fopen(file_name.c_str(), "w");
	if (file == nullptr)
		return;

	try {
		auto write = [this](const void* data, size_t length) -> void {
			size_t result = fwrite(data, 1, length, file);
			if (result != length) {
				int error = ferror(file);
				log("WAV file write fail: %d (\"%s\").", error, strerror(error));
				throw Exception("wav-file-write-fail");
				}
			};
		char data[8];

		// "RIFF" chunk.
		write("RIFF", 4);
		write(write_dword(0, &data), 4); 	// Placeholder; will be updated later.
		write("WAVE", 4);

		// "fmt " chunk.
		const int bytes_per_sample = 3;
		write("fmt ", 4);
		write(write_dword(16, &data), 4); 	// Chunk size.
		write(write_word(WAVFile::WAVE_FORMAT_PCM, &data), 2);
		int num_channels = capture_channels->size();
		write(write_word(num_channels, &data), 2);
		write(write_dword(engine->sample_rate(), &data), 4);
		int byte_rate = num_channels * engine->sample_rate() * bytes_per_sample;
		write(write_dword(byte_rate, &data), 4);
		int block_align = num_channels * bytes_per_sample;
		write(write_word(block_align, &data), 2);
		write(write_word(bytes_per_sample * 8, &data), 2);

		// "data" chunk.
		data_chunk_start = ftell(file);
		write("data", 4);
		write(write_dword(0, &data), 4); 	// Placeholder; will be updated later.
		}
	catch (Exception& e) {
		fclose(file);
		file = nullptr;
		}
}


void Recorder::ArmedTrack::finish_wav_file()
{
	if (file == nullptr)
		return;

	uint32_t samples_end = ftell(file);
	uint32_t samples_size = samples_end - data_chunk_start - (2 * 4);
	char data[8];

	// Update the size of the "data" chunk.
	fseek(file, data_chunk_start + 4, SEEK_SET);
	fwrite(write_dword(samples_size, &data), 1, 4, file);

	// Update the size of the "RIFF" chunk.
	uint32_t riff_chunk_size = samples_end - 4;
	fseek(file, 4, SEEK_SET);
	fwrite(write_dword(riff_chunk_size, &data), 1, 4, file);

	fclose(file);
	file = nullptr;
}



