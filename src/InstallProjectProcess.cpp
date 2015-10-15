#include "InstallProjectProcess.h"
#include "AudioEngine.h"
#include "web/Connection.h"
#include "DAW.h"
#include "Project.h"


InstallProjectProcess::~InstallProjectProcess()
{
	delete old_project;
}


bool InstallProjectProcess::is_done()
{
	return state == Done;
}


void InstallProjectProcess::next()
{
	switch (state) {
		case Installing:
			old_project = engine->install_project(new_project);
			state = Reporting;
			break;

		case Reporting:
			{
			Web::Connection* connection = daw->websocket_connection();
			if (connection && new_project)
				connection->send_websocket_message("project-loaded");
			state = Done;
			}
			break;
		}
}


