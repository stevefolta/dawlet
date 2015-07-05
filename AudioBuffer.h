#ifndef AudioBuffer_h
#define AudioBuffer_h

#include "AudioTypes.h"


struct AudioBuffer {
	AudioSample	samples[];

	void	clear();
	};


#endif 	// !AudioBuffer_h
