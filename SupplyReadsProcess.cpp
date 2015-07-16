#include "SupplyReadsProcess.h"
#include "AudioFileRead.h"
#include "AudioEngine.h"
#include "Logger.h"


SupplyReadsProcess::SupplyReadsProcess(int num_requests)
{
	AudioFileRead* last_request = nullptr;
	for (; num_requests > 0; --num_requests) {
		next_request = new AudioFileRead();
		next_request->next_free = last_request;
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
		next_request = request->next_free;
		request->next_free = nullptr;
		engine->receive_audio_file_read(request);
		}

	state = Done;
}



