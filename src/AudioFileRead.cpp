#include "AudioFileRead.h"
#include "AudioEngine.h"
#include "Clip.h"
#include "OpenAudioFile.h"
#include "DAW.h"
#include "Exception.h"
#include "Logger.h"
#include <stdlib.h>
#include <string.h>


AudioFileRead::AudioFileRead(DAW* daw_in)
	: daw(daw_in), next_read(nullptr), state(Waiting), is_reading(false),
	  buffer(nullptr)
{
}


AudioFileRead::~AudioFileRead()
{
	if (is_reading) {
		OpenAudioFile* open_file = clip->file->get_open_file();
		if (open_file) {
			int result = aio_cancel(open_file->fd, &async_read);
			if (result != AIO_CANCELED)
				log("Couldn't cancel AIO read!  (%d: %s).", errno, strerror(errno));
			}
		daw->remove_file_read(this);
		}
	if (buffer)
		free(buffer);
}


bool AudioFileRead::is_done()
{
	return state == Done;
}


void AudioFileRead::next()
{
	switch (state) {
		case StartingRead:
			start_read();
			break;
		}
}


bool AudioFileRead::return_immediately()
{
	return false;
}


void AudioFileRead::request_read(
	Clip* clip_in,
	unsigned long start_frame_in, unsigned long num_frames_in)
{
	clip = clip_in;
	start_frame = start_frame_in;
	num_frames = num_frames_in;

	state = StartingRead;
	engine->return_process(this);
}


void AudioFileRead::start_read()
{
	// Allocate the buffer.
	OpenAudioFile* open_file = clip->file->open();
	buffer_size = open_file->size_of_frames(num_frames);
	buffer = (char*) malloc(buffer_size);

	// Start the read.
	memset(&async_read, 0, sizeof(async_read));
	async_read.aio_fildes = open_file->fd;
	async_read.aio_offset = open_file->offset_for_frame(start_frame);
	async_read.aio_buf = buffer;
	async_read.aio_nbytes = buffer_size;
	async_read.aio_reqprio = 0;
	async_read.aio_sigevent.sigev_notify = SIGEV_NONE;
	int result = aio_read(&async_read);
	if (result == -1)
		throw Exception("aio-read-fail");

	state = Reading;
	is_reading = true;
	daw->add_file_read(this);
}


bool AudioFileRead::read_is_complete()
{
	if (aio_error(&async_read) == EINPROGRESS)
		return false;

	clip->file->close();

	state = Playing;
	is_reading = false;
	engine->continue_process(this);
	return true;
}


void AudioFileRead::dispose()
{
	state = Done;
	engine->return_process(this);
}


bool AudioFileRead::is_playable()
{
	return state == Playing;
}



