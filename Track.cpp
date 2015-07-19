#include "Track.h"
#include "BufferManager.h"
#include "Project.h"
#include "Playlist.h"
#include "Send.h"
#include "AudioEngine.h"
#include "Amp.h"
#include "ProjectReader.h"
#include "Exception.h"


Track::Track(Project* projectIn, int idIn)
	: project(projectIn), playlist(nullptr)
{
	id = idIn >=0 ? idIn : project->new_id();
	gain = 1.0;
	sends_to_parent = true;
}


Track::~Track()
{
	while (!sends.empty()) {
		delete sends.back();
		sends.pop_back();
		}
	// Don't delete receives; their sends will do that.
	while (!children.empty()) {
		delete children.back();
		children.pop_back();
		}
	delete playlist;
}


void Track::read_json(ProjectReader* reader)
{
	reader->start_object();
	while (true) {
		std::string field_name = reader->next_field();
		if (field_name.empty())
			break;
		if (field_name == "id")
			id = reader->next_int();
		else if (field_name == "playlist") {
			if (!playlist)
				playlist = new Playlist();
			playlist->read_json(reader);
			}
		else if (field_name == "children") {
			reader->start_array();
			while (!reader->array_is_done()) {
				Track* track = new Track(project);
				try {
					track->read_json(reader);
					}
				catch (Exception e) {
					delete track;
					throw e;
					}
				children.push_back(track);
				}
			}
		else if (field_name == "sends") {
			// TODO: We don't know how to deal with this yet...
			reader->ignore_value();
			}
		else if (field_name == "gain")
			gain = reader->next_double();
		else if (field_name == "sends_to_parent")
			sends_to_parent = reader->next_bool();
		else {
			// This is something from the future; ignore it.
			reader->ignore_value();
			}
		}
}


void Track::prepare_to_play()
{
	if (playlist)
		playlist->prepare_to_play();

	for (auto it = children.begin(); it != children.end(); ++it)
		(*it)->prepare_to_play();
}


void Track::run(AudioBuffer* buffer_out)
{
	// Get a buffer for the track.
	int buffer_size = engine->buffer_size();
	AudioBuffer* track_buffer = engine->get_buffer();
	track_buffer->clear();

	// Playlist.
	if (playlist)
		playlist->run(track_buffer);

	// Children.
	if (!children.empty()) {
		for (auto it = children.begin(); it != children.end(); ++it) {
			// Run the child track.
			Track* child = *it;
			child->run(track_buffer);
			}
		}

	// Receives.
	/***/

	// Mix the track into the output buffer.
	if (sends_to_parent) {
		AudioSample* in = track_buffer->samples;
		AudioSample* out = buffer_out->samples;
		for (int samples_left = buffer_size; samples_left > 0; --samples_left) {
			*out += amp(gain, *in++);
			++out;
			}
		}

	engine->free_buffer(track_buffer);
}


int Track::max_used_id()
{
	int max_id = id;
	for (auto it = children.begin(); it != children.end(); ++it) {
		int child_id = (*it)->id;
		if (child_id > max_id)
			max_id = child_id;
		}
	return max_id;
}



