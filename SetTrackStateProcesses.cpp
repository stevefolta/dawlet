#include "SetTrackStateProcesses.h"
#include "Track.h"
#include "web/Connection.h"
#include "Logger.h"


SetTrackStateProcess::SetTrackStateProcess(
	Track* track_in, Web::Connection* connection_in)
	: track(track_in), connection(connection_in), state(Setting)
{
	if (connection)
		connection->start_replying();
}


bool SetTrackStateProcess::is_done()
{
	return state == Done;
}


void SetTrackStateProcess::next()
{
	switch (state) {
		case Setting:
			set();
			state = Replying;
			break;
		case Replying:
			// If there wasn't a reply connection, this is happening on project
			// load (eg. ArmTrackProcess) and doesn't represent a real mutation,
			// so we don't call mutation_done() in that case.
			if (connection) {
				connection->send_ok_reply();
				mutation_done();
				}
			state = Done;
			break;
		}
}




