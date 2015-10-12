#include "RestoreTrackProcess.h"
#include "Project.h"
#include "Track.h"
#include "DAW.h"
#include "web/Connection.h"


bool RestoreTrackProcess::is_done()
{
	return state == Done;
}


void RestoreTrackProcess::next()
{
	switch (state) {
		case InEngine:
			in_engine();
			state = BackInDAW;
			break;
		case BackInDAW:
			back_in_daw();
			state = Done;
			break;
		}
}


void RestoreTrackProcess::in_engine()
{
	Project* project = daw->cur_project();
	parent->add_child_before(track, before_track);
	project->restore_track_id(track);
}


void RestoreTrackProcess::back_in_daw()
{
	connection->send_json_reply(track->api_json());
	mutation_done();
}



