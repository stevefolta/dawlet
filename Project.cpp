#include "Project.h"
#include "Track.h"
#include "AudioFile.h"
#include "ProjectReader.h"
#include "JSONParser.h"
#include "IndentedOStream.h"
#include "Logger.h"
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


Project::Project(std::string path_in)
	: path(path_in), next_new_id(1), dirfd(-1)
{
	master = new Track(this, new_id());
	add_track_by_id(master);
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


std::string Project::api_json()
{
	std::stringstream result;
	result << "{ ";
	result << "\"master\": " << master->id;
	result << " }";
	return result.str();
}


void Project::write_to_file(IndentedOStream& stream)
{
	stream << "{" << '\n';
	stream.indent();

	stream << "\"master\": ";
	master->write_to_file(stream);
	stream << ',' << '\n' << '\n';

	stream << "\"files\": {" << '\n';
	stream.indent();
	bool first_one = true;
	for (auto it = files.begin(); it != files.end(); ++it) {
		if (first_one)
			first_one = false;
		else
			stream << ',' << '\n';
		(*it)->write_to_file(stream);
		}
	stream << '\n' << "}" << '\n';
	stream.unindent();

	stream.unindent();
	stream << "}" << '\n';
}


void Project::prepare_to_play()
{
	master->prepare_to_play();
}


void Project::run(AudioBuffer** buffers_out, int num_channels)
{
	master->run(buffers_out, num_channels);
}


void Project::run_metering()
{
	master->run_metering();
}


void Project::arm_armed_tracks(Recorder* recorder)
{
	master->arm_armed_tracks(recorder);
}


int Project::new_id()
{
	return next_new_id++;
}


int Project::max_used_id()
{
	int max_id = master->max_used_id();
	for (auto it = files.begin(); it != files.end(); ++it) {
		int id = (*it)->id;
		if (id > max_id)
			max_id = id;
		}
	return max_id;
}


int Project::total_num_tracks()
{
	return master->total_num_tracks();
}


void Project::add_file(AudioFile* file)
{
	files.push_back(file);
}


std::string Project::get_dir_path()
{
	if (dir_path.empty()) {
		dir_path = path;
		size_t slash_pos = path.rfind('/');
		if (slash_pos == std::string::npos)
			dir_path = ".";
		else
			dir_path = path.substr(0, slash_pos);
		}

	return dir_path;
}


int Project::get_dirfd()
{
	if (dirfd == -1) {
		std::string dir_path = get_dir_path();
		dirfd = open(dir_path.c_str(), O_RDONLY | O_DIRECTORY);
		}
	return dirfd;
}


void Project::load_audio_file_info()
{
	for (auto it = files.begin(); it != files.end(); ++it)
		(*it)->load_info();
}


void Project::add_track_by_id(Track* track)
{
	tracks_by_id[track->id] = track;
}



