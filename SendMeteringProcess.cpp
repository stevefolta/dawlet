#include "SendMeteringProcess.h"
#include "AudioEngine.h"
#include "DAW.h"
#include "web/Connection.h"
#include "Logger.h"
#include <sstream>


SendMeteringProcess::SendMeteringProcess()
	: next_peaks(nullptr), state(Installing), num_peaks(0)
{
}


bool SendMeteringProcess::is_done()
{
	return (state == Done);
}


void SendMeteringProcess::next()
{
	switch (state) {
		case Installing:
			engine->add_metering_process(this);
			state = Sending;
			break;
		case Sending:
			send();
			break;
		}
}


bool SendMeteringProcess::return_immediately()
{
	return false;
}


bool SendMeteringProcess::is_full()
{
	return num_peaks >= peaks_per_send;
}


void SendMeteringProcess::add_peak(int track_id, AudioSample peak)
{
	if (is_full())
		return;

	peaks[num_peaks].track_id = track_id;
	peaks[num_peaks].peak = peak;
	num_peaks += 1;
}


void SendMeteringProcess::send()
{
	std::stringstream message;
	message << "meters";
	for (int i = 0; i < num_peaks; ++i)
		message << ' ' << peaks[i].track_id << ' ' << peaks[i].peak;
	daw->websocket_connection()->send_websocket_message(message.str());
	state = Done;
}



