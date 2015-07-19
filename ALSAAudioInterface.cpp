#include "ALSAAudioInterface.h"
#include "Exception.h"
#include "Logger.h"


ALSAAudioInterface::ALSAAudioInterface(std::string name_in)
	: name(name_in), out_buffer(nullptr)
{
	int err = snd_pcm_open(&playback, name.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0)
		throw Exception("alsa-open-fail");
	buffers_sent = 0;
}


ALSAAudioInterface::~ALSAAudioInterface()
{
	snd_pcm_close(playback);
	free(out_buffer);
}


void ALSAAudioInterface::setup(int num_channels, int sample_rate, int buffer_size)
{
	this->buffer_size = buffer_size;
	free(out_buffer);
	out_buffer = (char*) malloc(4 * buffer_size);

	// Set the hardware params.
	snd_pcm_hw_params_t* hw_params;
	int err = snd_pcm_hw_params_malloc(&hw_params);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	err = snd_pcm_hw_params_any(playback, hw_params);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	err = snd_pcm_hw_params_set_access(
		playback, hw_params, SND_PCM_ACCESS_RW_NONINTERLEAVED);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	err = snd_pcm_hw_params_set_format(playback, hw_params, SND_PCM_FORMAT_FLOAT_LE);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	err = snd_pcm_hw_params_set_rate(playback, hw_params, sample_rate, 0);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	err = snd_pcm_hw_params_set_channels(playback, hw_params, num_channels);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	err = snd_pcm_hw_params(playback, hw_params);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	snd_pcm_hw_params_free(hw_params);

	// Set the software params.
	snd_pcm_sw_params_t* sw_params;
	err = snd_pcm_sw_params_malloc(&sw_params);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	err = snd_pcm_sw_params_current(playback, sw_params);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	err = snd_pcm_sw_params_set_avail_min(playback, sw_params, buffer_size);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	err = snd_pcm_sw_params_set_start_threshold(playback, sw_params, 0);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	err = snd_pcm_sw_params(playback, sw_params);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	snd_pcm_sw_params_free(sw_params);

	err = snd_pcm_prepare(playback);
	if (err < 0)
		throw Exception("alsa-setup-fail");
}


void ALSAAudioInterface::wait_until_ready()
{
	int err = snd_pcm_wait(playback, 1000);
	if (err < 0) {
		snd_pcm_state_t state = snd_pcm_state(playback);
		log("snd_pcm_wait returned %d (\"%s\").", err, snd_strerror(err));
		log("snd_pcm_state() is %d.", state);
		log("buffers_sent: %d.", buffers_sent);
		throw Exception("playback-fail");
		}
}


void ALSAAudioInterface::send_data(AudioSample* samples)
{
	int frames_to_deliver = snd_pcm_avail_update(playback);
	if (frames_to_deliver == -EPIPE)
		throw Exception("playback-xrun");
	else if (frames_to_deliver < 0)
		throw Exception("alsa-playback-fail");
	if (frames_to_deliver < buffer_size) {
		// This happens often, but we'll be using a blocking write below.  So
		// really, the call to snd_pcm_avail_update() is just to detect xruns.
		// log("Short play write!: %d frames.", frames_to_deliver);
		}

	// Send the samples.
	void* channel_buffers[1];
	channel_buffers[0] = samples;
	int err = snd_pcm_writen(playback, channel_buffers, buffer_size);
	if (err < 0) {
		log("snd_pcm_writen() returned %d (\"%s\").", err, snd_strerror(err));
		log("state = %d", snd_pcm_state(playback));
		log("buffers_sent: %d.", buffers_sent);
		throw Exception("alsa-playback-fail");
		}

	buffers_sent += 1;
}



