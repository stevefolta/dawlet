#ifndef Message_h
#define Message_h

struct Message {
	enum {
		None,

		EngineKill,
		EngineDied,

		ContinueProcess,

		// DAW -> Engine.
		Play,
		Stop,
		Pause,
		StopPlay,
		PausePlay,
		Rewind,

		// Engine -> DAW.
		NeedMoreReadRequests,
		NeedMoreMetering,
		Xrun,
		Error,
		};

	int	type;
	union {
		void*	param;
		int	num;
		};
	};


#endif 	// !Message_h

