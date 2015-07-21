#include "Clip.h"
#include "ProjectReader.h"
#include "AudioFileRead.h"
#include "AudioFile.h"
#include "AudioEngine.h"
#include "Logger.h"


void Clip::init()
{
	file_start_frame = 0;
	reads[0] = reads[1] = nullptr;
}


Clip::~Clip()
{
	if (reads[0])
		delete reads[0];
	if (reads[1])
		delete reads[1];
}


void Clip::read_json(ProjectReader* reader)
{
	reader->start_object();
	while (true) {
		std::string field_name = reader->next_field();
		if (field_name.empty())
			break;
		if (field_name == "file")
			file = reader->file_for_id(reader->next_int());
		else if (field_name == "start")
			start = reader->next_double();
		else if (field_name == "length")
			length_in_frames = reader->next_int();
		else if (field_name == "file-start-frame")
			file_start_frame = reader->next_int();
		else
			reader->ignore_value();
		}
}


void Clip::prepare_to_play()
{
	playing = false;
	read_ahead();
}


void Clip::run(AudioBuffer* buffer_out)
{
	int start_out_frame = 0;
	int buffer_size = engine->buffer_size();

	// Are we starting playback?
	if (!playing) {
		ProjectPosition buffer_end_time =
			engine->play_head +
			(ProjectPosition) buffer_size / engine->sample_rate();
		if (buffer_end_time < start) {
			// This clip will not start in this buffer.
			return;
			}

		// We're starting playback.
		int sample_rate = file->info.sample_rate;
		if (engine->play_head > start) {
			// Starting playback in the middle of the clip.
			play_frame =
				file_start_frame + (engine->play_head - start) * sample_rate;
			}
		else {
			// The clip starts in the middle (or the beginning) of the buffer.
			play_frame = file_start_frame;
			start_out_frame = (start - engine->play_head) * sample_rate;
			}
		playing = true;
		}

	// Copy samples into the buffer.
	while (start_out_frame < buffer_size) {
		// Are we done with playback?
		if (play_frame >= file_end_frame()) {
			playing = false;
			return;
			}

		// Find the read containing play_frame.
		int which_read = -1;
		for (int i = 0; i < num_reads; ++i) {
			AudioFileRead* read = reads[i];
			if (read == nullptr)
				continue;
			if (play_frame >= read->start_frame && play_frame < read->end_frame()) {
				which_read = i;
				break;
				}
			}
		if (which_read < 0 || !reads[which_read]->is_playable()) {
			// We didn't read it in time, we'll have to skip it.  Try to go to the
			// next available read.
			log("Missing read at %d!", play_frame);
			unsigned long buffer_end_frame =
				play_frame + buffer_size - start_out_frame;
			which_read = -1;
			for (int i = 0; i < num_reads; ++i) {
				AudioFileRead* read = reads[i];
				if (read == nullptr)
					continue;
				if (read->start_frame > play_frame && read->end_frame() < buffer_end_frame) {
					start_out_frame += read->start_frame - play_frame;
					play_frame = read->start_frame;
					which_read = i;
					break;
					}
				}
			if (which_read < 0 || !reads[which_read]->is_playable()) {
				// We've got nothing for this entire buffer.
				play_frame += buffer_size;
				break;
				}
			}

		// Copy the frames.
		AudioFileRead* read = reads[which_read];
		int num_frames = buffer_size - start_out_frame;
		int frames_in_read = read->end_frame() - play_frame;
		if (frames_in_read < num_frames)
			num_frames = frames_in_read;
		file->play_into_buffer(
			buffer_out, start_out_frame,
			read->buffer, play_frame - read->start_frame, num_frames);
		play_frame += num_frames;
		start_out_frame += num_frames;
		}
}


void Clip::read_ahead()
{
	// Free any buffers we no longer need.
	int sample_rate = file->info.sample_rate;
	unsigned long next_play_frame =
		(engine->play_head - start) * sample_rate + file_start_frame;
	unsigned long read_ahead_point =
		next_play_frame + engine->read_ahead_seconds * sample_rate;
	long first_loading_frame = -1;
	unsigned long last_loading_frame = 0;
	for (int i = 0; i < num_reads; ++i) {
		if (reads[i] == nullptr)
			continue;
		unsigned long end_frame = reads[i]->end_frame();
		bool unneeded =
			next_play_frame > end_frame ||
			reads[i]->start_frame > read_ahead_point;
		if (unneeded) {
			reads[i]->dispose();
			reads[i] = nullptr;
			}
		else {
			if (first_loading_frame < 0 || (long) reads[i]->start_frame < first_loading_frame)
				first_loading_frame = reads[i]->start_frame;
			if (end_frame - 1 > last_loading_frame)
				last_loading_frame = end_frame - 1;
			}
		}

	// Are we over with?
	if (play_frame >= file_end_frame())
		return;

	// Read from the current point if needed.
	// Hopefully we're not playing...
	if (first_loading_frame < 0 || first_loading_frame > next_play_frame) {
		unsigned long num_frames = engine->read_ahead_seconds * sample_rate;
		if (first_loading_frame < 0) {
			// We have nothing, read all the way to the read-ahead point.
			}
		else {
			unsigned long max_length =
				length_in_frames - (first_loading_frame - file_start_frame);
			if (num_frames > max_length)
				num_frames = max_length;
			}
		start_read(next_play_frame, num_frames);
		unsigned long read_end = next_play_frame + num_frames - 1;
		if (read_end > last_loading_frame)
			last_loading_frame = read_end;
		}

	// Read past the read-ahead point.
	unsigned long read_start = last_loading_frame + 1;
	if (read_start < read_ahead_point && read_start < file_end_frame()) {
		unsigned long num_frames = engine->read_ahead_seconds * sample_rate;
		unsigned long max_length = file_end_frame() - read_start;
		if (num_frames > max_length)
			num_frames = max_length;
		start_read(read_start, num_frames);
		}
}


void Clip::start_read(unsigned long start_frame, unsigned long num_frames)
{
	// Find where to put it.
	int i = 0;
	for (; i < num_reads; ++i) {
		if (reads[i] == nullptr)
			break;
		}
	if (i >= num_reads) {
		log("Read slot allocation fail!");
		return;
		}

	// Start it.
	AudioFileRead* read = engine->next_audio_file_read();
	if (read == nullptr) {
		log("Reads exhausted!");
		return;
		}
	reads[i] = read;
	read->request_read(this, start_frame, num_frames);
}


ProjectPosition Clip::end()
{
	return start + ((ProjectPosition) length_in_frames / file->info.sample_rate);
}


bool Clip::contains_time(ProjectPosition time)
{
	return (time >= start && time < end());
}




