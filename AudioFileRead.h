#ifndef AudioFileRead_h
#define AudioFileRead_h

#include "Process.h"


class AudioFileRead : public Process {
	public:
		AudioFileRead();
		~AudioFileRead();

		bool	is_done();
		void	next();

		AudioFileRead*	next_free;
		char*	buffer;

	protected:
		enum {
			Waiting,
			StartingRead,
			Reading,
			Playing,
			Done
			};
		int state;
	};


#endif	// !AudioFileRead_h

