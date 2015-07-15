#include "Playlist.h"
#include "Clip.h"
#include "ProjectReader.h"


Playlist::Playlist()
{
}


Playlist::~Playlist()
{
	while (!clips.empty()) {
		delete clips.back();
		clips.pop_back();
		}
}


void Playlist::read_json(ProjectReader* reader)
{
	reader->start_object();
	while (true) {
		std::string field_name = reader->next_field();
		if (field_name.empty())
			break;
		if (field_name == "clips") {
			reader->start_array();
			while (!reader->array_is_done()) {
				Clip* clip = new Clip();
				clip->read_json(reader);
				clips.push_back(clip);
				}
			}
		else
			reader->ignore_value();
		}
}


void Playlist::run(AudioBuffer* buffer_out)
{
	/***/
}



