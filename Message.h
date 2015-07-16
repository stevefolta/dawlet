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
		};

	int	type;
	void*	param;
	};


#endif 	// !Message_h

