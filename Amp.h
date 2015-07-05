#ifndef Amp_h
#define Amp_h

#include "AudioTypes.h"

// This potentially allows us to experiment with different types of amps by
// just replacing this one function.
inline AudioSample amp(float gain, AudioSample sample) {
	return gain * sample;
	}


#endif 	// !Amp_h

