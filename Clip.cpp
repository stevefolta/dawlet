#include "Clip.h"
#include "ProjectReader.h"


Clip::~Clip()
{
}


void Clip::read_json(ProjectReader* reader)
{
	reader->start_object();
	while (true) {
		std::string field_name = reader->next_field();
		if (field_name.empty())
			break;
		if (field_name == "file")
			file = reader->file_for_id(reader->next_int());
		else if (field_name == "start")
			start = reader->next_double();
		else if (field_name == "length")
			length_in_samples = reader->next_int();
		else
			reader->ignore_value();
		}
}




