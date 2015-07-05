#include "Project.h"
#include "Track.h"


Project::Project()
	: next_new_id(1)
{
	master = new Track(this, new_id());
}


Project::~Project()
{
	while (!tracks.empty()) {
		delete tracks.back();
		tracks.pop_back();
		}
	delete master;
}


void Project::run(AudioBuffer* bufferOut)
{
	/***/
}


int Project::new_id()
{
	return ++next_new_id;
}



