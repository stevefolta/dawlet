#include "AudioFile.h"
#include "ProjectReader.h"


AudioFile::~AudioFile()
{
}


void AudioFile::read_json(ProjectReader* reader)
{
	reader->start_object();
	while (true) {
		std::string field_name = reader->next_field();
		if (field_name.empty())
			break;
		if (field_name == "path")
			path = reader->next_string();
		else
			reader->ignore_value();
		}
}



