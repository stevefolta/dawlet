#include "SetTrackGainProcess.h"
#include "Track.h"
#include "web/Connection.h"
#include "AudioEngine.h"


SetTrackGainProcess::SetTrackGainProcess(
	Track* track_in, float gain_in, Web::Connection* connection_in)
	: track(track_in), gain(gain_in), connection(connection_in), state(Setting)
{
	connection->start_replying();
}


bool SetTrackGainProcess::is_done()
{
	return state == Done;
}


void SetTrackGainProcess::next()
{
	switch (state) {
		case Setting:
			track->gain = gain;
			state = Replying;
			break;
		case Replying:
			connection->send_ok_reply();
			state = Done;
			break;
		}
}



