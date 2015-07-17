#include "AudioFile.h"
#include "ProjectReader.h"
#include "WAVFile.h"
#include "Project.h"
#include "Exception.h"


void AudioFile::init()
{
	info.sample_rate = 0;
}


AudioFile::~AudioFile()
{
	delete open_file;
}


void AudioFile::read_json(ProjectReader* reader)
{
	project = reader->get_project();

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


void AudioFile::load_info()
{
	OpenAudioFile* open_file = open();
	info = open_file->read_info();
	close();
}


OpenAudioFile* AudioFile::open()
{
	if (open_file == nullptr) {
		size_t dot_position = path.rfind('.');
		if (dot_position == std::string::npos)
			throw Exception("unsupported-audio-file-type");
		std::string suffix = path.substr(dot_position + 1);
		if (suffix == "wav" || suffix == "WAV")
			open_file = new WAVFile(path, project->get_dirfd());
		else
			throw Exception("unsupported-audio-file-type");
		if (info.sample_rate == 0)
			info = open_file->read_info();
		}
	num_opens += 1;
	return open_file;
}


void AudioFile::close()
{
	num_opens -= 1;
	if (num_opens <= 0) {
		delete open_file;
		open_file = nullptr;
		num_opens = false;
		}
}



