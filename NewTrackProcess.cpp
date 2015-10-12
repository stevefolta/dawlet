#include "NewTrackProcess.h"
#include "Track.h"
#include "Project.h"
#include "DAW.h"
#include "web/Connection.h"
#include "Logger.h"


NewTrackProcess::NewTrackProcess(Track* after_track_in, Web::Connection* connection_in)
	: after_track(after_track_in), connection(connection_in)
{
	Project* project = daw->cur_project();

	if (after_track)
		parent = after_track->get_parent();
	else
		parent = project->get_master();

	track = new Track(project, parent, project->new_id());
	project->add_track_by_id(track);
}


void NewTrackProcess::in_engine()
{
	parent->add_child(track, after_track);
}


void NewTrackProcess::back_in_daw()
{
	connection->send_json_reply(track->api_json());
	mutation_done();
}



