#include "ProjectReader.h"
#include "Project.h"
#include "AudioFile.h"
#include "Exception.h"


ProjectReader::ProjectReader(std::string text, Project* project_in)
	: project(project_in)
{
	parser = new JSONParser(text);
}


ProjectReader::~ProjectReader()
{
	delete parser;
}


AudioFile* ProjectReader::file_for_id(int id)
{
	AudioFile* file = files[id];
	if (file == nullptr) {
		file = new AudioFile(project, id);
		project->add_file(file);
		files[id] = file;
		}
	return file;
}


int ProjectReader::string_to_id(std::string str_in)
{
	const char* str = str_in.c_str();
	char* end_ptr = nullptr;
	int result = strtol(str, &end_ptr, 0);
	if (end_ptr == str || *end_ptr != 0)
		throw Exception("invalid-project-file");
	return result;
}



