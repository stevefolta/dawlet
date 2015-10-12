#include "RestoreTrackProcess.h"
#include "Project.h"
#include "Track.h"
#include "DAW.h"
#include "web/Connection.h"


void RestoreTrackProcess::in_engine()
{
	Project* project = daw->cur_project();
	parent->add_child_before(track, before_track);
	project->restore_track_id(track);
	track->prepare_to_play();
}


void RestoreTrackProcess::back_in_daw()
{
	connection->send_json_reply(track->api_json());
	mutation_done();
}



