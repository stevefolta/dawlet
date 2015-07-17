#include "Project.h"
#include "Track.h"
#include "AudioFile.h"
#include "ProjectReader.h"
#include "JSONParser.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


Project::Project(std::string path_in)
	: path(path_in), next_new_id(1), dirfd(-1)
{
	master = new Track(this, new_id());
}


Project::~Project()
{
	delete master;
	while (!files.empty()) {
		delete files.back();
		files.pop_back();
		}
}


void Project::read_json(ProjectReader* reader)
{
	reader->start_object();
	while (true) {
		std::string field_name = reader->next_field();
		if (field_name.empty())
			break;
		if (field_name == "master")
			master->read_json(reader);
		else if (field_name == "files") {
			reader->start_object();
			while (true) {
				std::string id_str = reader->next_field();
				if (id_str.empty())
					break;
				int id = reader->string_to_id(id_str);
				AudioFile* file = reader->file_for_id(id);
					// Will add the file to the project if it wasn't there already.
				file->read_json(reader);
				}
			}
		else {
			// Must be something from the future; ignore it.
			reader->ignore_value();
			}
		}
	next_new_id = max_used_id() + 1;
}


void Project::run(AudioBuffer* buffer_out)
{
	master->run(buffer_out);
}


void Project::read_ahead()
{
	master->read_ahead();
}


int Project::new_id()
{
	return ++next_new_id;
}


int Project::max_used_id()
{
	int max_id = master->max_used_id();
	for (auto it = files.begin(); it != files.end(); ++it) {
		int id = (*it)->id;
		if (id > max_id)
			max_id = id;
		}
}


void Project::add_file(AudioFile* file)
{
	files.push_back(file);
}


int Project::get_dirfd()
{
	if (dirfd == -1) {
		std::string dir_path = path;
		size_t slash_pos = path.rfind('/');
		if (slash_pos == std::string::npos)
			dir_path = ".";
		else
			dir_path = path.substr(0, slash_pos);
		dirfd = open(dir_path.c_str(), O_RDONLY | O_DIRECTORY);
		}
	return dirfd;
}


void Project::load_audio_file_info()
{
	for (auto it = files.begin(); it != files.end(); ++it)
		(*it)->load_info();
}



