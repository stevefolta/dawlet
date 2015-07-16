#include "SupplyReadRequestsProcess.h"
#include "AudioFileReadRequest.h"
#include "AudioEngine.h"
#include "Logger.h"


SupplyReadRequestsProcess::SupplyReadRequestsProcess(int num_requests)
{
	AudioFileReadRequest* last_request = nullptr;
	for (; num_requests > 0; --num_requests) {
		next_request = new AudioFileReadRequest();
		next_request->next_free = last_request;
		last_request = next_request;
		}

	state = Supplying;
}


SupplyReadRequestsProcess::~SupplyReadRequestsProcess()
{
	for (int i = 0; i < num_requests; ++i) {
		if (requests[i])
			delete requests[i];
		}
}


bool SupplyReadRequestsProcess::is_done()
{
	return state = Done;
}


void SupplyReadRequestsProcess::next()
{
	switch (state) {
		case Supplying:
			supplying();
			break;
		}
}


void SupplyReadRequestsProcess::supplying()
{
	while (next_request) {
		AudioFileReadRequest* request = next_request;
		next_request = request->next_free;
		request->next_free = nullptr;
		engine->receive_audio_file_read_request(request);
		}

	state = Done;
}



