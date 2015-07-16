#ifndef AudioFileReadRequest_h
#define AudioFileReadRequest_h


class AudioFileReadRequest {
	public:
		AudioFileReadRequest()
			: next_free(nullptr) {}

		AudioFileReadRequest*	next_free;

	protected:
	};


#endif	// !AudioFileReadRequest_h

