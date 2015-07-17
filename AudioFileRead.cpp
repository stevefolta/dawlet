#include "AudioFileRead.h"
#include "AudioEngine.h"
#include "Clip.h"
#include "OpenAudioFile.h"
#include "DAW.h"
#include "Exception.h"
#include <stdlib.h>
#include <string.h>


AudioFileRead::AudioFileRead(DAW* daw_in)
	: daw(daw_in), next_read(nullptr), state(Waiting), buffer(nullptr)
{
}


AudioFileRead::~AudioFileRead()
{
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
	daw->add_file_read(this);
}


bool AudioFileRead::read_is_complete()
{
	if (aio_error(&async_read) == EINPROGRESS)
		return false;

	state = Playing;
	engine->continue_process(this);
	return true;
}


void AudioFileRead::dispose()
{
	state = Done;
	engine->continue_process(this);
}



