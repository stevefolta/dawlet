#ifndef AudioFileRead_h
#define AudioFileRead_h


class AudioFileRead {
	public:
		AudioFileRead()
			: next_free(nullptr) {}

		AudioFileRead*	next_free;

	protected:
	};


#endif	// !AudioFileRead_h

