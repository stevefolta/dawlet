#include "Recorder.h"
#include "RecordBuffers.h"
#include "Track.h"
#include "Project.h"
#include "AudioSystem.h"
#include "AudioInterface.h"
#include "SetTrackStateProcesses.h"
#include "DAW.h"
#include "AudioEngine.h"
#include "Logger.h"

enum {
	num_record_buffers = 4,
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
	// Open the files.
	for (auto& track_pair: armed_tracks) {
		ArmedTrack& track = track_pair.second;
		//... open file...
		/***/
		}

	// Supply the engine with RecordBuffers.
	for (int i = 0; i < num_record_buffers; ++i)
		engine->start_process(new RecordBuffers(armed_tracks.size()));

	// Start recording.
	engine->send(Message::Record);
}


void Recorder::stop()
{
	// Close the files.
	for (auto& track_pair: armed_tracks) {
		ArmedTrack& track = track_pair.second;
		//... close file...
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

	// Write to each file.
	for (auto& track_pair: armed_tracks) {
		ArmedTrack& track = track_pair.second;
		/***/
		}

	// Clear capture_buffers.
	for (auto it = capture_buffers.begin(); it != capture_buffers.end(); ++it)
		*it = nullptr;
}


Recorder::ArmedTrack::ArmedTrack(Track* track_in)
	: track(track_in), capture_channels(nullptr)
{
	update_capture_channels();
}


Recorder::ArmedTrack::~ArmedTrack()
{
	delete capture_channels;
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



