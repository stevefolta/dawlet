#include "Recorder.h"
#include "RecordBuffers.h"
#include "AudioSystem.h"
#include "AudioInterface.h"


Recorder::Recorder()
	: num_armed_channels(0)
{
	/***/
}


Recorder::~Recorder()
{
	/***/
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
	/***/

	// Clear capture_buffers.
	for (auto it = capture_buffers.begin(); it != capture_buffers.end(); ++it)
		*it = nullptr;
}



