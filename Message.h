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

