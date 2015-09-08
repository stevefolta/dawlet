#include "SetTrackRecordArmedProcess.h"
#include "Track.h"
#include "web/Connection.h"
#include "Logger.h"


SetTrackRecordArmedProcess::SetTrackRecordArmedProcess(
	Track* track_in, bool new_armed_in, Web::Connection* connection_in)
	: track(track_in), new_armed(new_armed_in), connection(connection_in), state(Setting)
{
	connection->start_replying();
}


bool SetTrackRecordArmedProcess::is_done()
{
	return state == Done;
}


void SetTrackRecordArmedProcess::next()
{
	switch (state) {
		case Setting:
			track->record_armed = new_armed;
			state = Replying;
			break;
		case Replying:
			connection->send_ok_reply();
			mutation_done();
			state = Done;
			break;
		}
}



