#include "Recorder.h"
#include "RecordBuffers.h"
#include "Track.h"
#include "AudioSystem.h"
#include "AudioInterface.h"


Recorder::Recorder()
{
	/***/
}


Recorder::~Recorder()
{
	/***/
}


void Recorder::arm_track(Track* track)
{
	armed_tracks.emplace(track->id, track);
}


void Recorder::unarm_track(Track* track)
{
	armed_tracks.erase(track->id);
}


void Recorder::start()
{
	for (auto& track_pair: armed_tracks) {
		ArmedTrack& track = track_pair.second;
		//... open file...
		/***/
		}
}


void Recorder::stop()
{
	for (auto& track_pair: armed_tracks) {
		ArmedTrack& track = track_pair.second;
		//... close file...
		/***/
		}
}


void Recorder::interface_changed()
{
	for (auto& track_pair: armed_tracks)
		track_pair.second.update_capture_channels();
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
	if (interface)
		capture_channels = interface->capture_channels_for_input_name("In 1" /*** track->input_name() ***/);
	else
		capture_channels = nullptr;
}



