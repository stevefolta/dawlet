#ifndef AudioInterface_h
#define AudioInterface_h

#include "AudioTypes.h"
#include <string>
#include <vector>
class AudioBuffer;


class AudioInterface {
	public:
		AudioInterface()
			: armed_channels(nullptr) {}
		virtual ~AudioInterface();

		virtual void	setup(int num_channels, int sample_rate, int buffer_size) = 0;
		virtual void	wait_until_ready() = 0;
		virtual void	send_data(AudioBuffer** buffers, int num_channels) = 0;

		virtual bool	capture_is_ready() = 0;
		virtual void	capture_data(AudioBuffer** buffers, int num_channels) = 0;

		virtual int	get_num_capture_channels() = 0;

		// DAW thread only:
		virtual std::string	input_names_json() = 0;
		virtual std::vector<int>*	capture_channels_for_input_name(std::string name) = 0;

		void	setup_armed_channels();
		bool	channel_is_armed(int capture_channel)
			{ return armed_channels[capture_channel]; }
		void	set_channel_armed(int capture_channel, bool armed)
			{ armed_channels[capture_channel] = armed; }

	protected:
		bool*	armed_channels;
	};


#endif	// !AudioInterface_h

