#include "StartRecordingProcess.h"
#include "RecordBuffers.h"
#include "AudioEngine.h"
#include "DAW.h"
#include <sstream>


StartRecordingProcess::StartRecordingProcess(
	std::vector<RecordingClip>* recording_clips_in,
	std::string clip_specs_in)
	: recording_clips(recording_clips_in), clip_specs(clip_specs_in)
{
}


StartRecordingProcess::~StartRecordingProcess()
{
	// The engine thread took ownership of "recording_clips" and the contents of
	// "record_bufferses", so we have nothing to delete.
}


void StartRecordingProcess::in_engine()
{
	start_recording();
}


void StartRecordingProcess::back_in_daw()
{
	std::stringstream reply_message;
	reply_message << "recording-started " << start_time << clip_specs;
	daw->send_websocket_message(reply_message.str());
}


void StartRecordingProcess::start_recording()
{
	// Have the "record_bufferses" supply themselves to the engine.
	for (auto& record_buffers: record_bufferses)
		record_buffers->next();

	engine->start_recording(recording_clips);

	start_time = engine->play_head;
	state = Done;
}



