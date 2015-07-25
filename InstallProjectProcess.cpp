#include "InstallProjectProcess.h"
#include "AudioEngine.h"
#include "web/Connection.h"
#include "DAW.h"
#include "Project.h"


InstallProjectProcess::~InstallProjectProcess()
{
	delete project;
}


bool InstallProjectProcess::is_done()
{
	return state == Done;
}


void InstallProjectProcess::next()
{
	switch (state) {
		case Installing:
			project = engine->install_project(project);
			state = Reporting;
			break;

		case Reporting:
			{
			Web::Connection* connection = daw->websocket_connection();
			if (connection)
				connection->send_websocket_message("project-loaded");
			state = Done;
			}
			break;
		}
}


