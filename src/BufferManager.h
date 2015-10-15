#ifndef BufferManager_h
#define BufferManager_h

#include "AudioTypes.h"
#include "AudioBuffer.h"


class BufferManager {
	public:
		BufferManager();
		~BufferManager();

		void	reset();
		bool	overran() { return had_overrun; }

		AudioBuffer*	get_buffer();
		void	free_buffer(AudioBuffer* buffer);

		void	tick();	// Non-realtime processing.
		void	run();	// Realtime processing, called once per hardware buffer out.

	protected:
		struct FreeAudioBuffer {
			FreeAudioBuffer* next;
			};

		FreeAudioBuffer*	first_free_buffer;
		int	num_free_buffers;
		bool	had_overrun;

		// New buffer creation, requested by the realtime thread, created by the
		// non-realtime thread, then installed by the realtime thread.
		int	needed_new_buffers, num_new_buffers;
		FreeAudioBuffer*	first_new_buffer;
		FreeAudioBuffer*	last_new_buffer;

		void	clear();
	};


#endif 	// !BufferManager_h

