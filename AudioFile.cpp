#include "AudioFile.h"
#include "ProjectReader.h"
#include "WAVFile.h"
#include "Project.h"
#include "AudioBuffer.h"
#include "Logger.h"
#include "Exception.h"


void AudioFile::init()
{
	info.sample_rate = 0;
}


AudioFile::~AudioFile()
{
	delete open_file;
}


void AudioFile::read_json(ProjectReader* reader)
{
	project = reader->get_project();

	reader->start_object();
	while (true) {
		std::string field_name = reader->next_field();
		if (field_name.empty())
			break;
		if (field_name == "path")
			path = reader->next_string();
		else
			reader->ignore_value();
		}
}


void AudioFile::load_info()
{
	OpenAudioFile* open_file = open();
	info = open_file->read_info();
	close();
}


OpenAudioFile* AudioFile::open()
{
	if (open_file == nullptr) {
		size_t dot_position = path.rfind('.');
		if (dot_position == std::string::npos)
			throw Exception("unsupported-audio-file-type");
		std::string suffix = path.substr(dot_position + 1);
		if (suffix == "wav" || suffix == "WAV")
			open_file = new WAVFile(path, project->get_dirfd());
		else
			throw Exception("unsupported-audio-file-type");
		if (info.sample_rate == 0)
			info = open_file->read_info();
		}
	num_opens += 1;
	return open_file;
}


void AudioFile::close()
{
	num_opens -= 1;
	if (num_opens <= 0) {
		delete open_file;
		open_file = nullptr;
		num_opens = false;
		}
}


void AudioFile::play_into_buffer(
	AudioBuffer* buffer_out, int start_out_frame,
	char* data, int start_in_frame, int num_frames)
{
	int bytes_per_sample = info.bits_per_sample / 8;
	int bytes_per_frame = bytes_per_sample * info.num_channels;
	data += start_in_frame * bytes_per_frame;
	AudioSample* out = buffer_out->samples + start_out_frame;
	AudioSample scale = 1.0 / 0x7FFFFFFF;
	for (; num_frames > 0; --num_frames) {
		//*** TODO: Support non-little-endian files.
		uint32_t sample = 0;
		int shift = 8 * (4 - bytes_per_sample);
		for (int bytes_left = bytes_per_sample; bytes_left > 0; --bytes_left) {
			sample |= ((uint8_t) *data++) << shift;
			shift += 8;
			}
		*out++ += (AudioSample) ((int32_t) sample) * scale;
		if (info.num_channels > 1)
			data += bytes_per_frame - bytes_per_sample;
		}
}



