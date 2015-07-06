#include "Project.h"
#include "Track.h"
#include "JSONParser.h"


Project::Project()
	: next_new_id(1)
{
	master = new Track(this, new_id());
}


Project::~Project()
{
	delete master;
}


void Project::read_json(JSONParser* parser)
{
	parser->start_object();
	while (true) {
		std::string field_name = parser->next_field();
		if (field_name.empty())
			break;
		if (field_name == "master")
			master->read_json(parser);
		else {
			// Must be something from the future; ignore it.
			parser->ignore_value();
			}
		}
	next_new_id = max_used_id() + 1;
}


void Project::run(AudioBuffer* buffer_out)
{
	master->run(buffer_out);
}


int Project::new_id()
{
	return ++next_new_id;
}


int Project::max_used_id()
{
	return master->max_used_id();
}



