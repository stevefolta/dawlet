#include "Track.h"
#include "BufferManager.h"
#include "Project.h"
#include "Playlist.h"
#include "Send.h"
#include "AudioEngine.h"
#include "Amp.h"
#include "ProjectReader.h"
#include "Logger.h"
#include "Exception.h"
#include <sstream>


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
		else if (field_name == "name")
			name = reader->next_string();
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
					project->add_track_by_id(track);
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


std::string Track::api_json()
{
	std::stringstream result;
	result << "{ ";
	result << "\"name\": \"" << name << "\"";
	result << ", ";
	result << "\"id\": " << id;
	result << ", ";
	result << "\"gain\": " << gain;
	result << ", ";
	result << "\"sends_to_parent\": " << sends_to_parent;
	result << ", ";

	// Children.
	result << "\"children\": [";
	bool did_child = false;
	for (auto it = children.begin(); it != children.end(); ++it) {
		if (did_child)
			result << ", ";
		else
			did_child = true;
		result << (*it)->id;
		}
	result << "] ";

	result << " }";
	return result.str();
}


std::string Track::clips_json()
{
	return playlist->clips_json();
}


void Track::prepare_to_play()
{
	if (playlist)
		playlist->prepare_to_play();

	for (auto it = children.begin(); it != children.end(); ++it)
		(*it)->prepare_to_play();
}


void Track::run(AudioBuffer** buffers_out, int num_channels)
{
	// Get buffers for the track.
	int buffer_size = engine->buffer_size();
	AudioBuffer* track_buffers[num_channels];
	int which_channel;
	for (which_channel = 0; which_channel < num_channels; ++which_channel) {
		AudioBuffer* buffer = engine->get_buffer();
		buffer->clear();
		track_buffers[which_channel] = buffer;
		}

	// Playlist.
	if (playlist)
		playlist->run(track_buffers, num_channels);

	// Children.
	if (!children.empty()) {
		for (auto it = children.begin(); it != children.end(); ++it) {
			// Run the child track.
			Track* child = *it;
			child->run(track_buffers, num_channels);
			}
		}

	// Receives.
	/***/

	// Mix the track into the output buffers.
	if (sends_to_parent) {
		for (which_channel = 0; which_channel < num_channels; ++which_channel) {
			AudioSample* in = track_buffers[which_channel]->samples;
			AudioSample* out = buffers_out[which_channel]->samples;
			for (int samples_left = buffer_size; samples_left > 0; --samples_left) {
				*out += amp(gain, *in++);
				++out;
				}
			}
		}

	for (which_channel = 0; which_channel < num_channels; ++which_channel)
		engine->free_buffer(track_buffers[which_channel]);
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



