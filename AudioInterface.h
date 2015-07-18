#ifndef AudioInterface_h
#define AudioInterface_h

#include "AudioTypes.h"


class AudioInterface {
	public:
		virtual ~AudioInterface() {}

		virtual void	setup(int num_channels, int sample_rate, int buffer_size) = 0;
		virtual void	wait_until_ready() = 0;
		virtual void	send_data(AudioSample* samples) = 0;

	protected:
	};


#endif	// !AudioInterface_h

