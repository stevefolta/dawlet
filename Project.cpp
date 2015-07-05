#include "Project.h"
#include "Track.h"


Project::Project()
	: next_new_id(1)
{
	master = new Track(this, new_id());
}


Project::~Project()
{
	delete master;
}


void Project::run(AudioBuffer* buffer_out)
{
	master->run(buffer_out);
}


int Project::new_id()
{
	return ++next_new_id;
}



