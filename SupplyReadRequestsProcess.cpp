#include "SupplyReadRequestsProcess.h"
#include "AudioFileReadRequest.h"
#include "AudioEngine.h"
#include "Logger.h"


SupplyReadRequestsProcess::SupplyReadRequestsProcess()
{
	for (int i = 0; i < num_requests; ++i)
		requests[i] = new AudioFileReadRequest();
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
	int num_taken =
		engine->receive_audio_file_read_requests(num_requests, requests);
	for (int i = 0; i < num_taken; ++i)
		requests[i] = nullptr;

	state = Done;
}



