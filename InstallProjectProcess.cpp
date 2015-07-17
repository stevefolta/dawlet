#include "InstallProjectProcess.h"
#include "AudioEngine.h"
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
			state = Done;
			break;
		}
}


