#include "DeleteTrackProcess.h"
#include "Project.h"
#include "DAW.h"
#include "web/Connection.h"
#include "Logger.h"


void DeleteTrackProcess::in_engine()
{
	Project* project = daw->cur_project();
	project->remove_track(track);
}


void DeleteTrackProcess::back_in_daw()
{
	connection->send_ok_reply();
	mutation_done();
}



