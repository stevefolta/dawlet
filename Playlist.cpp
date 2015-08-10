#include "Playlist.h"
#include "Clip.h"
#include "ProjectReader.h"
#include "AudioEngine.h"
#include "web/Connection.h"
#include "Logger.h"
#include <sstream>


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


std::string Playlist::clips_json()
{
	std::stringstream result;
	result << "[ ";
	bool did_one = false;
	for (auto it = clips.begin(); it != clips.end(); ++it) {
		if (did_one)
			result << ", ";
		else
			did_one = true;
		(*it)->build_api_json(result);
		}
	result << " ]";
	return result.str();
}


void Playlist::prepare_to_play()
{
	// All clips will "read_ahead()"; this will also cause them to dispose reads
	// they don't need.  Also find "first_playing_clip".
	first_playing_clip = clips.end();
	for (auto it = clips.begin(); it != clips.end(); ++it) {
		Clip* clip = *it;
		clip->prepare_to_play();
		if (first_playing_clip == clips.end() && clip->end() > engine->play_head)
			first_playing_clip = it;
		}
}


void Playlist::run(AudioBuffer** buffers_out, int num_channels)
{
	std::vector<Clip*>::iterator it = first_playing_clip;
	ProjectPosition read_ahead_point =
		engine->play_head + engine->read_ahead_seconds;
	for (; it != clips.end(); ++it) {
		Clip* clip = *it;

		// Handle reads.
		clip->read_ahead();
		if (engine->play_head > clip->end())
			first_playing_clip = it + 1;

		// Are we done?
		if (read_ahead_point < clip->start)
			break;

		// Playback.
		clip->run(buffers_out, num_channels);
		}
}



