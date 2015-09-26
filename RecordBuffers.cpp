#include "RecordBuffers.h"
#include "AudioEngine.h"
#include "DAW.h"
#include "Recorder.h"


RecordBuffers::RecordBuffers(int num_channels)
	: next_free(nullptr)
{
	buffers = new Buffer[num_channels];
	num_buffers = 0;
	state = Resupplying;
}


RecordBuffers::~RecordBuffers()
{
	delete buffers;
}


bool RecordBuffers::is_done()
{
	return state == Done;
}


void RecordBuffers::next()
{
	switch (state) {
		case Filling:	/* do nothing */	break;
		case Writing:	write();	break;
		case Resupplying:	resupply();	break;
		case Disposing:	state = Done;	break;
		}
}


bool RecordBuffers::return_immediately()
{
	return false;
}


void RecordBuffers::add(int capture_channel, AudioBuffer* buffer)
{
	buffers[num_buffers].capture_channel = capture_channel;
	buffers[num_buffers].buffer = buffer;
	num_buffers += 1;
}


void RecordBuffers::start_write()
{
	// Called from the engine.
	state = Writing;
	engine->return_process(this);
}


void RecordBuffers::write()
{
	// We're in the DAW thread.

	daw->get_recorder()->write_buffers(this);

	state = Resupplying;
	engine->continue_process(this);
}


void RecordBuffers::resupply()
{
	// We're in the engine.

	engine->add_free_record_buffers(this);
	state = Filling;
}


void RecordBuffers::dispose()
{
	// Called from the engine.

	// Return the buffers to the engine.
	free_buffers();

	state = Disposing;
	engine->return_process(this);
}


void RecordBuffers::free_buffers()
{
	for (int i = 0; i < num_buffers; ++num_buffers)
		engine->free_buffer(buffers[i].buffer);
	num_buffers = 0;
}



