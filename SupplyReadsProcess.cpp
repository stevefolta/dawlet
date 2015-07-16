#include "SupplyReadsProcess.h"
#include "AudioFileRead.h"
#include "AudioEngine.h"
#include "Logger.h"


SupplyReadsProcess::SupplyReadsProcess(DAW* daw_in, int num_requests)
	: daw(daw_in)
{
	AudioFileRead* last_request = nullptr;
	for (; num_requests > 0; --num_requests) {
		next_request = new AudioFileRead(daw);
		next_request->next_read = last_request;
		last_request = next_request;
		}

	state = Supplying;
}


SupplyReadsProcess::~SupplyReadsProcess()
{
}


bool SupplyReadsProcess::is_done()
{
	return state = Done;
}


void SupplyReadsProcess::next()
{
	switch (state) {
		case Supplying:
			supplying();
			break;
		}
}


void SupplyReadsProcess::supplying()
{
	while (next_request) {
		AudioFileRead* request = next_request;
		next_request = request->next_read;
		request->next_read = nullptr;
		engine->receive_audio_file_read(request);
		}

	state = Done;
}



