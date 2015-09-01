#ifndef ALSAAudioInterface_h
#define ALSAAudioInterface_h

#include "AudioInterface.h"
#include <alsa/asoundlib.h>
#include <string>


class ALSAAudioInterface : public AudioInterface {
	public:
		ALSAAudioInterface(std::string name);
		~ALSAAudioInterface();

		void	setup(int num_channels, int sample_rate, int buffer_size);
		void	wait_until_ready();
		void	send_data(AudioBuffer** buffers, int num_channels);

	protected:
		std::string	name;
		snd_pcm_t*	playback;
		int	buffer_size;
		char*	out_buffer;
		int	buffers_sent;
		bool	initialized, started;
	};



#endif	// !ALSAAudioInterface_h

