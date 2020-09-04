#ifndef AudioBuffer_h
#define AudioBuffer_h

#include "AudioTypes.h"


struct AudioBuffer {
	AudioSample	samples[1];

	void	clear();
	};


#endif 	// !AudioBuffer_h
