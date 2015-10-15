#include "SetTrackStateProcesses.h"
#include "Track.h"
#include "web/Connection.h"
#include "Logger.h"


SetTrackStateProcess::SetTrackStateProcess(
	Track* track_in, Web::Connection* connection_in, bool really_mutating_in)
	: MutatingProcess(really_mutating_in),
	  track(track_in), connection(connection_in)
{
	if (connection)
		connection->start_replying();
}


void SetTrackStateProcess::in_engine()
{
	set();
}


void SetTrackStateProcess::back_in_daw()
{
	if (connection)
		connection->send_ok_reply();
	mutation_done();
}



