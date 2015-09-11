#include "ALSAAudioInterface.h"
#include "AudioBuffer.h"
#include "AudioEngine.h"
#include "Exception.h"
#include "Logger.h"
#ifdef USE_LOCAL_H
	#include "local.h"
#endif


ALSAAudioInterface::ALSAAudioInterface(std::string name_in)
	:	name(name_in),
		playback(nullptr), out_buffer(nullptr), xruns(0),
		initialized(false), started(false)
{
	int err = snd_pcm_open(&playback, name.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0)
		throw Exception("alsa-open-fail");
	buffers_sent = 0;
}


ALSAAudioInterface::~ALSAAudioInterface()
{
	if (playback) {
		snd_pcm_drain(playback);
		snd_pcm_close(playback);
		}
	free(out_buffer);
}


#ifdef USE_ALSA_MMAP
bool try_format(snd_pcm_t* pcm, snd_pcm_hw_params_t* hw_params, snd_pcm_format_t format)
{
	int err = snd_pcm_hw_params_set_format(pcm, hw_params, format);
	return err >= 0;
}

char* play_float(const AudioSample* in, char* out, int frames, int step)
{
	while (frames-- > 0) {
		*(float*) out = *in++;
		out += step;
		}
	return out;
}
char* play_32(const AudioSample* in, char* out, int frames, int step)
{
	while (frames-- > 0) {
		AudioSample sample = *in++;
		if (sample > 1.0)
			sample = 1.0;
		else if (sample < -1.0)
			sample = -1.0;
		*(int32_t*) out = (int32_t) (sample * 0x7FFFFFFF);
		out += step;
		}
	return out;
}
char* play_32_swap(const AudioSample* in, char* out, int frames, int step)
{
	while (frames-- > 0) {
		AudioSample sample = *in++;
		if (sample > 1.0)
			sample = 1.0;
		else if (sample < -1.0)
			sample = -1.0;
		int32_t sample_out = (int32_t) (sample * 0x7FFFFFFF);
		out[0] = sample_out >> 24;
		out[1] = sample_out >> 16;
		out[2] = sample_out >> 8;
		out[3] = sample_out;
		out += step;
		}
	return out;
}
char* play_24(const AudioSample* in, char* out, int frames, int step)
{
	while (frames-- > 0) {
		AudioSample sample = *in++;
		if (sample > 1.0)
			sample = 1.0;
		else if (sample < -1.0)
			sample = -1.0;
		int32_t sample_out = (int32_t) (sample * 0x007FFFFF);
		out[0] = sample_out;
		out[1] = sample_out >> 8;
		out[2] = sample_out >> 16;
		out += step;
		}
	return out;
}
char* play_24_swap(const AudioSample* in, char* out, int frames, int step)
{
	while (frames-- > 0) {
		AudioSample sample = *in++;
		if (sample > 1.0)
			sample = 1.0;
		else if (sample < -1.0)
			sample = -1.0;
		int32_t sample_out = (int32_t) (sample * 0x007FFFFF);
		out[0] = sample_out >> 16;
		out[1] = sample_out >> 8;
		out[2] = sample_out;
		out += step;
		}
	return out;
}
char* play_16(const AudioSample* in, char* out, int frames, int step)
{
	while (frames-- > 0) {
		AudioSample sample = *in++;
		if (sample > 1.0)
			sample = 1.0;
		else if (sample < -1.0)
			sample = -1.0;
		*out++ = (int16_t) (sample * 0x7FFF);
		}
	return out;
}
char* play_16_swap(const AudioSample* in, char* out, int frames, int step)
{
	while (frames-- > 0) {
		AudioSample sample = *in++;
		if (sample > 1.0)
			sample = 1.0;
		else if (sample < -1.0)
			sample = -1.0;
		int16_t sample_out = (sample * 0x7FFF);
		out[0] = sample_out >> 8;
		out[1] = sample_out;
		out += step;
		}
	return out;
}
#endif 	// USE_ALSA_MMAP


void ALSAAudioInterface::setup(int num_channels, int sample_rate, int buffer_size)
{
	this->buffer_size = buffer_size;
	free(out_buffer);
	out_buffer = (char*) malloc(4 * buffer_size);
	int err;
	auto check_err = [&err](const char* call, int value) -> void {
		if (err >= 0)
			return;
		log("%s(%d) failed: %d (\"%s\").", call, value, err, snd_strerror(err));
		throw Exception("alsa-setup-fail");
		};

	// Set the hardware params.
	snd_pcm_hw_params_t* hw_params;
	err = snd_pcm_hw_params_malloc(&hw_params);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	err = snd_pcm_hw_params_any(playback, hw_params);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	#ifdef USE_ALSA_MMAP
		err = snd_pcm_hw_params_set_access(
			playback, hw_params, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
		if (err < 0) {
			err = snd_pcm_hw_params_set_access(
				playback, hw_params, SND_PCM_ACCESS_MMAP_INTERLEAVED);
			}
		check_err("snd_pcm_hw_params_set_access", SND_PCM_ACCESS_MMAP_INTERLEAVED);
		#if __BYTE_ORDER == __LITTLE_ENDIAN
			#ifdef AUDIO_SAMPLE_FLOAT
			if (try_format(playback, hw_params, SND_PCM_FORMAT_FLOAT_LE))
				play_move = play_float;
			else
			#endif
			if (try_format(playback, hw_params, SND_PCM_FORMAT_S32_LE))
				play_move = play_32;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S32_BE))
				play_move = play_32_swap;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S24_3LE))
				play_move = play_24;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S24_3BE))
				play_move = play_24_swap;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S16_LE))
				play_move = play_16;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S16_BE))
				play_move = play_16_swap;
			else {
				err = -1;
				check_err("snd_pcm_hw_params_set_format", 0);
				}
		#else /* __BYTE_ORDER == __BIG_ENDIAN */
			#ifdef AUDIO_SAMPLE_FLOAT
			if (try_format(playback, hw_params, SND_PCM_FORMAT_FLOAT_BE))
				play_move = play_float;
			else
			#endif
			if (try_format(playback, hw_params, SND_PCM_FORMAT_S32_BE))
				play_move = play_32;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S32_LE))
				play_move = play_32_swap;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S24_3BE))
				play_move = play_24;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S24_3LE))
				play_move = play_24_swap;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S16_BE))
				play_move = play_16;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S16_LE))
				play_move = play_16_swap;
			else {
				err = -1;
				check_err("snd_pcm_hw_params_set_format", 0);
				}
		#endif
	#else 	// !USE_ALSA_MMAP
		err = snd_pcm_hw_params_set_access(
			playback, hw_params, SND_PCM_ACCESS_RW_NONINTERLEAVED);
		check_err("snd_pcm_hw_params_set_access", SND_PCM_ACCESS_RW_NONINTERLEAVED);
		err = snd_pcm_hw_params_set_format(playback, hw_params, SND_PCM_FORMAT_FLOAT_LE);
		check_err("snd_pcm_hw_params_set_format", SND_PCM_FORMAT_FLOAT_LE);
	#endif
	err = snd_pcm_hw_params_set_rate(playback, hw_params, sample_rate, 0);
	check_err("snd_pcm_hw_params_set_rate", sample_rate);
	err = snd_pcm_hw_params_set_channels(playback, hw_params, num_channels);
	check_err("snd_pcm_hw_params_set_channels", num_channels);
	snd_pcm_uframes_t actual_period_size = buffer_size;
	int dir = 0;
	err = snd_pcm_hw_params_set_period_size_near(playback, hw_params, &actual_period_size, &dir);
	check_err("snd_pcm_set_period_size", buffer_size);
	err = snd_pcm_hw_params_set_periods(playback, hw_params, 2, 0);
	check_err("snd_pcm_hw_params_set_periods", 2);
	snd_pcm_uframes_t actual_buffer_size = buffer_size * 2;
	err = snd_pcm_hw_params_set_buffer_size_near(playback, hw_params, &actual_buffer_size);
	check_err("snd_pcm_hw_params_set_buffer_size", actual_buffer_size);
	if (actual_buffer_size != buffer_size)
		log("actual buffer size: %d", actual_buffer_size);
	err = snd_pcm_hw_params(playback, hw_params);
	check_err("snd_pcm_hw_params", 0);
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
	check_err("snd_pcm_sw_params_set_avail_min", buffer_size);
	err = snd_pcm_sw_params_set_start_threshold(playback, sw_params, actual_period_size);
	check_err("snd_pcm_sw_params_set_start_threshold", buffer_size);
	err = snd_pcm_sw_params(playback, sw_params);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	snd_pcm_sw_params_free(sw_params);

	err = snd_pcm_prepare(playback);
	if (err < 0)
		throw Exception("alsa-setup-fail");

	initialized = true;
}


void ALSAAudioInterface::wait_until_ready()
{
}


void ALSAAudioInterface::send_data(AudioBuffer** buffers, int num_channels)
{
	if (!initialized)
		return;

#ifdef USE_ALSA_MMAP
	int err;
	auto check_err = [&err, this](const char* call) -> void {
		if (err == -EPIPE)
			got_xrun();
		else if (err < 0) {
			log("%s failed: %d (\"%s\").", call, err, snd_strerror(err));
			throw Exception("alsa-setup-fail");
			}
		};

	// Wait for the interface to be ready.
	err = snd_pcm_wait(playback, 1000);
	check_err("snd_pcm_wait");
	err = snd_pcm_avail_update(playback);
	check_err("snd_pcm_avail_update");

	// Set up MMAP.
	const snd_pcm_channel_area_t* areas;
	snd_pcm_uframes_t offset, frames = buffer_size;
	err = snd_pcm_mmap_begin(playback, &areas, &offset, &frames);
	check_err("snd_pcm_mmap_begin");

	// Copy the samples.
	const snd_pcm_channel_area_t* area = areas;
	for (int which_channel = 0; which_channel < num_channels; ++which_channel, ++area) {
		char* out_buffer =
			(char*) area->addr + ((area->first + area->step * offset) >> 3);
		play_move(
			buffers[which_channel]->samples, out_buffer,
			buffer_size, area->step >> 3);
		}

	// Send the samples to the hardware.
	err = snd_pcm_mmap_commit(playback, offset, frames);
	check_err("snd_pcm_mmap_commit");

#else 	// !USE_ALSA_MMAP

	// Send the samples.
	void* channel_buffers[num_channels];
	for (int which_channel = 0; which_channel < num_channels; ++which_channel)
		channel_buffers[which_channel] = buffers[which_channel]->samples;
	int err = snd_pcm_writen(playback, channel_buffers, buffer_size);
	if (err == -EPIPE)
		got_xrun();
	else if (err < 0) {
		log("snd_pcm_writen() returned %d (\"%s\").", err, snd_strerror(err));
		log("state = %d", snd_pcm_state(playback));
		log("buffers_sent: %d.", buffers_sent);
		throw Exception("alsa-playback-fail");
		}

#endif 	//  !USE_ALSA_MMAP

	if (!started) {
		err = snd_pcm_start(playback);
		if (err < 0)
			log("snd_pcm_start() returned %d (\"%s\").", err, snd_strerror(err));
		started = true;
		}

#ifdef DEBUG_ALSA_PLAYBACK_LATENCY
	static int num_latencies = 600;
	if (num_latencies > 0) {
		snd_pcm_sframes_t delay;
		err = snd_pcm_delay(playback, &delay);
		if (err >= 0 && delay > 0)
			log("Expected playback latency: %ld.", delay);
		--num_latencies;
		}
#endif

	buffers_sent += 1;
}


void ALSAAudioInterface::got_xrun()
{
	log("xrun");
	xruns += 1;
	engine->got_xrun();
	int err = snd_pcm_recover(playback, -EPIPE, 1);
	if (err < 0)
		log("snd_pcm_recover() returned %d (\"%s\").", err, snd_strerror(err));
}



