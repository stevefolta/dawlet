#ifndef Message_h
#define Message_h

struct Message {
	enum {
		None,

		EngineStart,
		EngineStop,
		EngineStarted,
		EngineStopped,
		EngineKill,
		EngineDied,

		ContinueProcess,

		// Engine -> DAW.
		NeedMoreReadRequests,
		};

	int	type;
	union {
		void*	param;
		int	num;
		};
	};


#endif 	// !Message_h

