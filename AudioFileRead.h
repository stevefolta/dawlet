#ifndef AudioFileRead_h
#define AudioFileRead_h

#include "Process.h"
#include <aio.h>
class AudioFile;
class DAW;


class AudioFileRead : public Process {
	public:
		AudioFileRead(DAW* daw_in);
		~AudioFileRead();

		bool	is_done();
		void	next();

		void	request_read(
			AudioFile* file, unsigned long start_frame, unsigned long num_frames);
		bool	read_is_complete();

		// When in a linked list.
		AudioFileRead*	next_read;

		// Valid after read.
		char*	buffer;
		unsigned long	buffer_size;

	protected:
		DAW*	daw;

		enum {
			Waiting,
			StartingRead,
			Reading,
			Installing,
			Playing,
			Done
			};
		int state;

		AudioFile*	file;
		unsigned long	start_frame, num_frames;

		void	start_read();
		struct aiocb	async_read;

		void	install();
	};


#endif	// !AudioFileRead_h

