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
	/***/
}


void Recorder::stop()
{
	/***/
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
	for (auto& track: armed_tracks) {
		/***/
		}

	// Clear capture_buffers.
	for (auto it = capture_buffers.begin(); it != capture_buffers.end(); ++it)
		*it = nullptr;
}


Recorder::ArmedTrack::ArmedTrack(Track* track)
	: capture_channels(nullptr)
{
	AudioInterface* interface = audio_system->selected_interface();
	if (interface)
		capture_channels = interface->capture_channels_for_input_name("In 1" /*** track->input_name() ***/);
}


Recorder::ArmedTrack::~ArmedTrack()
{
	delete capture_channels;
}



