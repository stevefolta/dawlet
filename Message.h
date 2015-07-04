#ifndef Message_h
#define Message_h

struct Message {
	enum {
		EngineStart,
		EngineStop,
		EngineStarted,
		EngineStopped,
		EngineKill,
		EngineDied,
		};

	int	type;
	};


#endif 	// !Message_h

