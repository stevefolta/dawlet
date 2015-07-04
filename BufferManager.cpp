#include "BufferManager.h"
#include <stdlib.h>

enum {
	initial_num_buffers = 100,
	free_buffers_wanted = 50,
	};


BufferManager::BufferManager()
{
	// Will not be usable until set_buffer_size() is called.
	first_free_buffer = NULL;
	num_free_buffers = 0;
	global_buffer_size = 0;
	had_overrun = false;

	needed_new_buffers = num_new_buffers = 0;
	first_new_buffer = last_new_buffer = NULL;
}


BufferManager::~BufferManager()
{
	clear();
}


void BufferManager::set_buffer_size(int new_buffer_size)
{
	// The engine must be stopped when this is called.

	clear();

	global_buffer_size = new_buffer_size;

	// Allocate new free buffers.
	FreeAudioBuffer* prev_buffer = NULL;
	for (int buffers_left = initial_num_buffers; buffers_left > 0; --buffers_left) {
		FreeAudioBuffer* buffer = (FreeAudioBuffer*) malloc(global_buffer_size);
		buffer->next = prev_buffer;
		prev_buffer = buffer;
		}
	first_free_buffer = prev_buffer;
}


AudioBuffer* BufferManager::get_buffer()
{
	if (first_free_buffer == NULL) {
		had_overrun = true;
		return NULL;
		}

	AudioBuffer* result = (AudioBuffer*) first_free_buffer;
	first_free_buffer = first_free_buffer->next;
	num_free_buffers -= 1;
	return result;
}


void BufferManager::free_buffer(AudioBuffer* buffer)
{
	FreeAudioBuffer* free_buffer = (FreeAudioBuffer*) buffer;
	free_buffer->next = first_free_buffer;
	first_free_buffer = free_buffer;
	num_free_buffers += 1;
}


void BufferManager::tick()
{
	// This is called in the non-realtime thread.

	if (needed_new_buffers && first_new_buffer == NULL) {
		num_new_buffers = 0;
		int buffers_to_build = needed_new_buffers;
		int buffers_left = buffers_to_build;
		last_new_buffer = (FreeAudioBuffer*) malloc(global_buffer_size);
		FreeAudioBuffer* prev_buffer = last_new_buffer;
		buffers_left -= 1;
		for (; buffers_left > 0; buffers_left -= 1) {
			FreeAudioBuffer* new_buffer = (FreeAudioBuffer*) malloc(global_buffer_size);
			new_buffer->next = prev_buffer;
			prev_buffer = new_buffer;
			}

		// Install the first buffer.  This will trigger the realtime thread to
		// accept these buffers.
		num_new_buffers = buffers_to_build;
		first_new_buffer = prev_buffer;
		}
}


void BufferManager::run()
{
	// This is called in the realtime thread.

	// Install new buffers allocated by the non-realtime thread.
	if (first_new_buffer) {
		last_new_buffer->next = first_free_buffer;
		first_free_buffer = first_new_buffer;
		num_free_buffers += num_new_buffers;
		needed_new_buffers -= num_new_buffers;
		first_new_buffer = NULL;
		}

	// Check that we have as many free buffers as we want.
	if (num_free_buffers <	free_buffers_wanted)
		needed_new_buffers += free_buffers_wanted;
}


void BufferManager::clear()
{
	// This must *only* be called when the engine is stopped.  That means that
	// all buffers will have been returned.  And it will be called in the
	// non-realtime thread.

	// Free the buffers in the free list.
	FreeAudioBuffer* buffer = first_free_buffer;
	while (buffer) {
		FreeAudioBuffer* next_buffer = buffer->next;
		free(buffer);
		buffer = next_buffer;
		}

	// Free allocated buffers that haven't yet been installed in the free list.
	buffer = first_new_buffer;
	while (buffer) {
		FreeAudioBuffer* next_buffer = buffer->next;
		free(buffer);
		buffer = next_buffer;
		}

	// Clean up variables.
	first_free_buffer = NULL;
	num_free_buffers = 0;
	had_overrun = false;
	first_new_buffer = last_new_buffer = NULL;
	needed_new_buffers = num_new_buffers = 0;
}



