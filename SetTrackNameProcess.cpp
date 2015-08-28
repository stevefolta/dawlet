#include "SetTrackNameProcess.h"
#include "Track.h"
#include "web/Connection.h"


SetTrackNameProcess::SetTrackNameProcess(
	Track* track_in, std::string name_in, Web::Connection* connection_in)
	: track(track_in), name(name_in), connection(connection_in), state(Setting)
{
	connection->start_replying();
}


bool SetTrackNameProcess::is_done()
{
	return state == Done;
}


void SetTrackNameProcess::next()
{
	switch (state) {
		case Setting:
			track->set_name(name);
			state = Replying;
			break;
		case Replying:
			connection->send_ok_reply();
			state = Done;
			break;
		}
}



