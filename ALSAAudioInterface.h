#ifndef ALSAAudioInterface_h
#define ALSAAudioInterface_h

#include "AudioInterface.h"
#include <alsa/asoundlib.h>
#include <string>
#ifdef USE_LOCAL_H
	#include "local.h"
#endif


class ALSAAudioInterface : public AudioInterface {
	public:
		ALSAAudioInterface(std::string name);
		~ALSAAudioInterface();

		void	setup(int num_channels, int sample_rate, int buffer_size);
		void	wait_until_ready();
		void	send_data(AudioBuffer** buffers, int num_channels);

	protected:
		typedef void (*play_mover)(const AudioSample* in, char* out, int frames, int step);
		typedef void (*capt_mover)(const char* in, AudioSample* out, int frames, int step);

		std::string	name;
		snd_pcm_t*	playback;
		int	buffer_size;
		int	buffers_sent;
		int	xruns;
		bool	initialized, started;
		play_mover	play_move;
		capt_mover	capture_move;

		void	got_xrun();
	};



#endif	// !ALSAAudioInterface_h

