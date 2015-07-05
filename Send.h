#ifndef Send_h
#define Send_h

class Track;
class AudioBuffer;


class Send {
	public:
		Send(Track* fromIn, Track* toIn)
			: from(fromIn), to(toIn) {}

		void	run(AudioBuffer* bufferOut);

	protected:
		Track*	from;
		Track*	to;
	};


#endif 	// !Send_h

