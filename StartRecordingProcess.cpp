#include "StartRecordingProcess.h"
#include "RecordBuffers.h"
#include "AudioEngine.h"


StartRecordingProcess::StartRecordingProcess(std::vector<RecordingClip>* recording_clips_in)
	: recording_clips(recording_clips_in), state(Starting)
{
}


StartRecordingProcess::~StartRecordingProcess()
{
	// The engine thread took ownership of "recording_clips" and the contents of
	// "record_bufferses", so we have nothing to delete.
}


bool StartRecordingProcess::is_done()
{
	return state == Done;
}


void StartRecordingProcess::next()
{
	switch (state) {
		case Starting: 	start_recording(); 	break;
		}
}


void StartRecordingProcess::start_recording()
{
	// Have the "record_bufferses" supply themselves to the engine.
	for (auto& record_buffers: record_bufferses)
		record_buffers->next();

	engine->start_recording(recording_clips);

	state = Done;
}



