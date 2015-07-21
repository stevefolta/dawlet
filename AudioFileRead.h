#ifndef AudioFileRead_h
#define AudioFileRead_h

#include "Process.h"
#include <aio.h>
class Clip;
class DAW;


class AudioFileRead : public Process {
	public:
		AudioFileRead(DAW* daw_in);
		~AudioFileRead();

		bool	is_done();
		void	next();

		void	request_read(
			Clip* clip, unsigned long start_frame, unsigned long num_frames);
		bool	read_is_complete();
		void	dispose();

		bool	is_playable();

		// When in a linked list.
		AudioFileRead*	next_read;

		// Valid after read requested.
		unsigned long	start_frame, num_frames;

		// Valid after read complete.
		char*	buffer;
		unsigned long	buffer_size;

		unsigned long	end_frame() { return start_frame + num_frames; }

	protected:
		DAW*	daw;

		enum {
			Waiting,
			StartingRead,
			Reading,
			Playing,
			Done
			};
		int state;

		Clip*	clip;

		void	start_read();
		struct aiocb	async_read;
	};


#endif	// !AudioFileRead_h

