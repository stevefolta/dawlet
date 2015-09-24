#include "Track.h"
#include "BufferManager.h"
#include "Project.h"
#include "Playlist.h"
#include "Send.h"
#include "AudioEngine.h"
#include "Amp.h"
#include "ProjectReader.h"
#include "IndentedOStream.h"
#include "Recorder.h"
#include "Logger.h"
#include "Exception.h"
#include <sstream>


Track::Track(Project* projectIn, int idIn)
	: project(projectIn), playlist(nullptr), cur_peak(0.0),
	  capture_channels(nullptr)
{
	id = idIn >=0 ? idIn : project->new_id();
	gain = 1.0;
	sends_to_parent = true;
	record_armed = false;
	monitor_input = true;
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
	delete capture_channels;
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
		else if (field_name == "record_armed")
			record_armed = reader->next_bool();
		else if (field_name == "monitor_input")
			monitor_input = reader->next_bool();
		else if (field_name == "input")
			input = reader->next_string();
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
	result << "\"record_armed\": " << record_armed;
	result << ", ";
	result << "\"monitor_input\": " << monitor_input;
	result << ", ";
	if (!input.empty()) {
		result << "\"input\": \"" << input << "\"";
		result << ", ";
		}

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


void Track::write_to_file(IndentedOStream& stream)
{
	bool compact = playlist->is_empty() && children.empty();
	char separator = (compact ? ' ' : '\n');

	stream << "{" << separator;
	IndentedOStream::Indenter indenter(stream);

	stream << "\"id\": " << id << "," << separator;
	stream << "\"name\": \"" << name << "\"," << separator;
	stream << "\"gain\": " << gain << "," << separator;
	stream << "\"sends_to_parent\": " << (sends_to_parent ? "true" : "false") << "," << separator;
	stream << "\"record_armed\": " << (record_armed ? "true" : "false") << "," << separator;
	stream << "\"monitor_input\": " << (monitor_input ? "true" : "false") << "," << separator;
	if (!input.empty())
		stream << "\"input\": \"" << input << "\"," << separator;

	stream << "\"playlist\": ";
	playlist->write_to_file(stream);

	if (!children.empty()) {
		stream << "," << separator;
		stream << "\"children\": [" << '\n';
		IndentedOStream::Indenter indenter(stream);
		bool first_one = true;
		for (auto it = children.begin(); it != children.end(); ++it) {
			if (first_one)
				first_one = false;
			else
				stream << ',' << '\n';
			(*it)->write_to_file(stream);
			}
		stream << '\n' << "]" << '\n';
		}

	stream << "}";
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
	AudioSample min_sample = 0, max_sample = 0;

	// Get buffers for the track.
	int buffer_size = engine->buffer_size();
	AudioBuffer* track_buffers[num_channels];
	int which_channel;
	for (which_channel = 0; which_channel < num_channels; ++which_channel) {
		AudioBuffer* buffer = engine->get_buffer();
		buffer->clear();
		track_buffers[which_channel] = buffer;
		}

	// Record-arm.
	if (record_armed && capture_channels) {
		int num_capture_channels = capture_channels->size();
		bool mono_capture = num_capture_channels == 1;
		for (which_channel = 0; which_channel < num_channels; ++which_channel) {
			if (!mono_capture && which_channel >= num_capture_channels)
				break;
			int capture_channel_index = (mono_capture ? 0 : which_channel);
			AudioBuffer* in_buffer =
				engine->get_capture_buffer(
					(*capture_channels)[capture_channel_index]);
			if (in_buffer) {
				AudioSample* in = in_buffer->samples;
				AudioSample* out = track_buffers[which_channel]->samples;
				for (int samples_left = buffer_size; samples_left > 0; --samples_left) {
					AudioSample sample = *in++;
					if (sample < min_sample)
						min_sample = sample;
					else if (sample > max_sample)
						max_sample = sample;
					if (monitor_input) {
						*out += sample;
						++out;
						}
					}
				}
			}
		}

	// Playlist.
	if (engine->is_playing() && playlist)
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
	// Collect metering info at the same time.
	if (sends_to_parent) {
		for (which_channel = 0; which_channel < num_channels; ++which_channel) {
			AudioSample* in = track_buffers[which_channel]->samples;
			AudioSample* out = buffers_out[which_channel]->samples;
			for (int samples_left = buffer_size; samples_left > 0; --samples_left) {
				AudioSample sample = amp(gain, *in++);
				if (record_armed) {
					// We're showing input rather than output, and we got the peaks
					// above.
					}
				else if (sample > max_sample)
					max_sample = sample;
				else if (sample < min_sample)
					min_sample = sample;
				*out += sample;
				++out;
				}
			}
		}

	// If not using the output buffers, we still want the metering info.
	else if (!record_armed) {
		for (which_channel = 0; which_channel < num_channels; ++which_channel) {
			AudioSample* in = track_buffers[which_channel]->samples;
			for (int samples_left = buffer_size; samples_left > 0; --samples_left) {
				AudioSample sample = amp(gain, *in++);
				if (sample > max_sample)
					max_sample = sample;
				else if (sample < min_sample)
					min_sample = sample;
				}
			}
		}

	// Finish processing metering.
	min_sample = -min_sample;
	if (min_sample > max_sample)
		max_sample = min_sample;
	if (max_sample > cur_peak)
		cur_peak = max_sample;

	for (which_channel = 0; which_channel < num_channels; ++which_channel)
		engine->free_buffer(track_buffers[which_channel]);
}


void Track::run_metering()
{
	engine->add_peak(id, cur_peak);
	cur_peak = 0;

	for (auto& child : children)
		child->run_metering();
}


void Track::arm_armed_tracks(Recorder* recorder)
{
	if (record_armed)
		recorder->arm_track(this);
	for (auto& child : children)
		child->arm_armed_tracks(recorder);
}


int Track::max_used_id()
{
	int max_id = id;
	for (auto& child : children) {
		int child_id = child->id;
		if (child_id > max_id)
			max_id = child_id;
		}
	return max_id;
}


int Track::total_num_tracks()
{
	int num_tracks = 1;
	for (auto& child : children)
		num_tracks += child->total_num_tracks();
	return num_tracks;
}



