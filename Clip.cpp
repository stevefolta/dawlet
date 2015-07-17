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


void Clip::read_ahead()
{
	// Free any buffers we no longer need.
	int sample_rate = file->info.sample_rate;
	unsigned long next_play_frame =
		(engine->play_head - start) * sample_rate + file_start_frame;
	unsigned long future_limit =
		next_play_frame + engine->read_ahead_seconds * sample_rate;
	long first_loading_frame = -1;
	unsigned long last_loading_frame = 0;
	for (int i = 0; i < 2; ++i) {
		if (reads[i] == nullptr)
			continue;
		unsigned long end_frame = reads[i]->end_frame();
		bool unneeded =
			next_play_frame > end_frame ||
			reads[i]->start_frame > future_limit;
		if (unneeded) {
			reads[i]->dispose();
			reads[i] = nullptr;
			}
		else {
			if (reads[i]->start_frame > first_loading_frame)
				first_loading_frame = reads[i]->start_frame;
			if (end_frame - 1 > last_loading_frame)
				last_loading_frame = end_frame - 1;
			}
		}

	// Read from the current point if needed.
	// Hopefully we're not playing...
	if (first_loading_frame < 0 || first_loading_frame > next_play_frame) {
		if (first_loading_frame < 0) {
			// We have nothing.
			start_read(next_play_frame, engine->read_ahead_seconds * sample_rate);
			}
		else {
			unsigned long num_frames = engine->read_ahead_seconds * sample_rate;
			unsigned long max_length =
				length_in_frames - (first_loading_frame - file_start_frame);
			if (num_frames > max_length)
				num_frames = max_length;
			start_read(next_play_frame, num_frames);
			}
		}

	/***/
}


void Clip::start_read(unsigned long start_frame, unsigned long num_frames)
{
	// Find where to put it.
	int i = 0;
	for (; i < 2; ++i) {
		if (reads[i] == nullptr)
			break;
		}
	if (i >= 2) {
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




