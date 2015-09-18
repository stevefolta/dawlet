#ifndef ALSAAudioInterface_h
#define ALSAAudioInterface_h

#include "AudioInterface.h"
#include <alsa/asoundlib.h>
#include <string>
#ifdef USE_LOCAL_H
	#include "local.h"
#endif

#ifndef DONT_USE_ALSA_MMAP
	#define USE_ALSA_MMAP
#endif


class ALSAAudioInterface : public AudioInterface {
	public:
		ALSAAudioInterface(std::string name);
		~ALSAAudioInterface();

		void	setup(int num_channels, int sample_rate, int buffer_size);
		void	wait_until_ready();
		void	send_data(AudioBuffer** buffers, int num_channels);
		bool	capture_is_ready();
		void	capture_data(AudioBuffer** buffers, int num_channels);

		int	get_num_capture_channels() { return num_capture_channels; }

	protected:
		typedef void (*play_mover)(const AudioSample* in, char* out, int frames, int step);
		typedef void (*capt_mover)(const char* in, AudioSample* out, int frames, int step);

		std::string	name;
		snd_pcm_t*	playback;
		snd_pcm_t*	capture;
		int	buffer_size;
		int	buffers_sent;
		int	xruns;
		bool	initialized, started, synced;
		int	num_capture_channels;
		play_mover	play_move;
		capt_mover	capture_move;

		void	got_xrun(const char* call);
		void	got_capture_xrun(const char* call);
		void	send_empty_buffer(int num_channels);
	};



#endif	// !ALSAAudioInterface_h

