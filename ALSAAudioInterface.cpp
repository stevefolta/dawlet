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
		playback(nullptr), xruns(0),
		initialized(false), started(false)
{
	int err = snd_pcm_open(&playback, name.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		log("snd_pcm_open(\"%s\") returned %d (\"%s\").", name.c_str(), err, snd_strerror(err));
		throw Exception("alsa-open-fail");
		}
	capture = nullptr;
	err = snd_pcm_open(&capture, name.c_str(), SND_PCM_STREAM_CAPTURE, 0);
	if (err < 0)
		log("snd_pcm_open(\"%s\") for capture returned %d (\"%s\").", name.c_str(), err, snd_strerror(err));
	buffers_sent = 0;
}


ALSAAudioInterface::~ALSAAudioInterface()
{
	if (playback) {
		snd_pcm_drain(playback);
		snd_pcm_close(playback);
		}
	if (capture) {
		snd_pcm_drop(capture);
		snd_pcm_close(capture);
		}
}


#ifdef USE_ALSA_MMAP
bool try_format(snd_pcm_t* pcm, snd_pcm_hw_params_t* hw_params, snd_pcm_format_t format)
{
	int err = snd_pcm_hw_params_set_format(pcm, hw_params, format);
	return err >= 0;
}

static void play_float(const AudioSample* in, char* out, int frames, int step)
{
	while (frames-- > 0) {
		*(float*) out = *in++;
		out += step;
		}
}
static void play_32(const AudioSample* in, char* out, int frames, int step)
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
}
static void play_32_be(const AudioSample* in, char* out, int frames, int step)
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
}
static void play_32_le(const AudioSample* in, char* out, int frames, int step)
{
	while (frames-- > 0) {
		AudioSample sample = *in++;
		if (sample > 1.0)
			sample = 1.0;
		else if (sample < -1.0)
			sample = -1.0;
		int32_t sample_out = (int32_t) (sample * 0x7FFFFFFF);
		out[0] = sample_out;
		out[1] = sample_out >> 8;
		out[2] = sample_out >> 16;
		out[3] = sample_out >> 24;
		out += step;
		}
}
static void play_24_le(const AudioSample* in, char* out, int frames, int step)
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
}
static void play_24_be(const AudioSample* in, char* out, int frames, int step)
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
}
static void play_16(const AudioSample* in, char* out, int frames, int step)
{
	while (frames-- > 0) {
		AudioSample sample = *in++;
		if (sample > 1.0)
			sample = 1.0;
		else if (sample < -1.0)
			sample = -1.0;
		*out++ = (int16_t) (sample * 0x7FFF);
		}
}
static void play_16_be(const AudioSample* in, char* out, int frames, int step)
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
}
static void play_16_le(const AudioSample* in, char* out, int frames, int step)
{
	while (frames-- > 0) {
		AudioSample sample = *in++;
		if (sample > 1.0)
			sample = 1.0;
		else if (sample < -1.0)
			sample = -1.0;
		int16_t sample_out = (sample * 0x7FFF);
		out[0] = sample_out;
		out[1] = sample_out >> 8;
		out += step;
		}
}

static void capt_float(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		*out++ = *(float*) in;
		in += step;
		}
}
static void capt_32(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		*out++ = *(int32_t*) in / (AudioSample) 0x7FFFFFFF;
		in += step;
		}
}
static void capt_32_be(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		int32_t sample =
			(in[0] & 0xFF) << 24 |
			(in[1] & 0xFF) << 16 |
			(in[2] & 0xFF) << 8 |
			(in[3] & 0xFF);
		*out++ = sample / (AudioSample) 0x7FFFFFFF;
		in += step;
		}
}
static void capt_32_le(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		int32_t sample =
			(in[0] & 0xFF) |
			(in[1] & 0xFF) << 8 |
			(in[2] & 0xFF) << 16 |
			(in[3] & 0xFF) << 24;
		*out++ = sample / (AudioSample) 0x7FFFFFFF;
		in += step;
		}
}
static void capt_24_le(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		int32_t sample =
			(in[0] & 0xFF) |
			(in[1] & 0xFF) << 8 |
			(in[2] & 0xFF) << 16;
		if (sample & 0x00800000)
			sample -= 0x01000000;
		*out++ = sample / (AudioSample) 0x007FFFFF;
		in += step;
		}
}
static void capt_24_be(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		int32_t sample =
			(in[0] & 0xFF) << 16 |
			(in[1] & 0xFF) << 8 |
			(in[3] & 0xFF);
		if (sample & 0x00800000)
			sample -= 0x01000000;
		*out++ = sample / (AudioSample) 0x007FFFFF;
		in += step;
		}
}
static void capt_16(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		*out++ = *(int16_t*) in / (AudioSample) 0x7FFF;
		in += step;
		}
}
static void capt_16_be(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		int16_t sample =
			(in[0] & 0xFF) << 8 |
			(in[1] & 0xFF);
		*out++ = sample / (AudioSample) 0x7FFF;
		in += step;
		}
}
static void capt_16_le(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		int16_t sample =
			(in[0] & 0xFF) |
			(in[1] & 0xFF) << 8;
		*out++ = sample / (AudioSample) 0x7FFF;
		in += step;
		}
}
#endif 	// USE_ALSA_MMAP


void ALSAAudioInterface::setup(int num_channels, int sample_rate, int buffer_size)
{
	this->buffer_size = buffer_size;
	int err;
	auto check_err = [&err](const char* call, int value) -> void {
		if (err >= 0)
			return;
		log("%s(%d) failed: %d (\"%s\").", call, value, err, snd_strerror(err));
		throw Exception("alsa-setup-fail");
		};

	// Playback.

	// Set the hardware params.
	snd_pcm_hw_params_t* hw_params = nullptr;
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
				play_move = play_32_be;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S24_3LE))
				play_move = play_24_le;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S24_3BE))
				play_move = play_24_be;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S16_LE))
				play_move = play_16;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S16_BE))
				play_move = play_16_be;
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
				play_move = play_32_le;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S24_3BE))
				play_move = play_24_be;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S24_3LE))
				play_move = play_24_le;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S16_BE))
				play_move = play_16;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S16_LE))
				play_move = play_16_le;
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

	// Set the software params.
	snd_pcm_sw_params_t* sw_params = nullptr;
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

	// Capture.
	if (capture) {
		// Set up hardware params.
		err = snd_pcm_hw_params_any(capture, hw_params);
		if (err < 0)
			throw Exception("alsa-setup-fail");
		#ifdef USE_ALSA_MMAP
			err = snd_pcm_hw_params_set_access(
				capture, hw_params, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
			if (err < 0) {
				err = snd_pcm_hw_params_set_access(
					capture, hw_params, SND_PCM_ACCESS_MMAP_INTERLEAVED);
				}
			check_err("snd_pcm_hw_params_set_access", SND_PCM_ACCESS_MMAP_INTERLEAVED);
			#if __BYTE_ORDER == __LITTLE_ENDIAN
				#ifdef AUDIO_SAMPLE_FLOAT
				if (try_format(capture, hw_params, SND_PCM_FORMAT_FLOAT_LE))
					capture_move = capt_float;
				else
				#endif
				if (try_format(capture, hw_params, SND_PCM_FORMAT_S32_LE))
					capture_move = capt_32;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S32_BE))
					capture_move = capt_32_be;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S24_3LE))
					capture_move = capt_24_le;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S24_3BE))
					capture_move = capt_24_be;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S16_LE))
					capture_move = capt_16;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S16_BE))
					capture_move = capt_16_be;
				else {
					err = -1;
					check_err("snd_pcm_hw_params_set_format", 0);
					}
			#else /* __BYTE_ORDER == __BIG_ENDIAN */
				#ifdef AUDIO_SAMPLE_FLOAT
				if (try_format(capture, hw_params, SND_PCM_FORMAT_FLOAT_BE))
					capture_move = capt_float;
				else
				#endif
				if (try_format(capture, hw_params, SND_PCM_FORMAT_S32_BE))
					capture_move = capt_32;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S32_LE))
					capture_move = capt_32_le;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S24_3BE))
					capture_move = capt_24_be;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S24_3LE))
					capture_move = capt_24_le;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S16_BE))
					capture_move = capt_16;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S16_LE))
					capture_move = capt_16_le;
				else {
					err = -1;
					check_err("snd_pcm_hw_params_set_format", 0);
					}
			#endif 	// __BYTE_ORDER == __BIG_ENDIAN
		#else 	// !USE_ALSA_MMAP
			err = snd_pcm_hw_params_set_access(
				capture, hw_params, SND_PCM_ACCESS_RW_NONINTERLEAVED);
			check_err("snd_pcm_hw_params_set_access", SND_PCM_ACCESS_RW_NONINTERLEAVED);
			err = snd_pcm_hw_params_set_format(capture, hw_params, SND_PCM_FORMAT_FLOAT_LE);
			check_err("snd_pcm_hw_params_set_format", SND_PCM_FORMAT_FLOAT_LE);
		#endif 	// !USE_ALSA_MMAP
		err = snd_pcm_hw_params_set_rate(capture, hw_params, sample_rate, 0);
		check_err("snd_pcm_hw_params_set_rate", sample_rate);
		unsigned int num_capture_channels = 0;
		snd_pcm_hw_params_get_channels_max(hw_params, &num_capture_channels);
		if (num_capture_channels > 1024) {
			// Dunno why this happens, but it can.  Assume we only have one
			// capture channel.
			num_capture_channels = 1;
			}
		this->num_capture_channels = num_capture_channels;
		err = snd_pcm_hw_params_set_channels(capture, hw_params, num_capture_channels);
		check_err("snd_pcm_hw_params_set_channels", num_channels);
		actual_period_size = buffer_size;
		dir = 0;
		err = snd_pcm_hw_params_set_period_size_near(capture, hw_params, &actual_period_size, &dir);
		check_err("snd_pcm_set_period_size", buffer_size);
		err = snd_pcm_hw_params_set_periods(capture, hw_params, 2, 0);
		check_err("snd_pcm_hw_params_set_periods", 2);
		actual_buffer_size = buffer_size * 2;
		err = snd_pcm_hw_params_set_buffer_size_near(capture, hw_params, &actual_buffer_size);
		check_err("snd_pcm_hw_params_set_buffer_size", actual_buffer_size);
		if (actual_buffer_size != buffer_size)
			log("actual buffer size: %d", actual_buffer_size);
		err = snd_pcm_hw_params(capture, hw_params);
		check_err("snd_pcm_hw_params", 0);

		// Set the software params.
		err = snd_pcm_sw_params_current(capture, sw_params);
		if (err < 0)
			throw Exception("alsa-setup-fail");
		err = snd_pcm_sw_params_set_avail_min(capture, sw_params, buffer_size);
		check_err("snd_pcm_sw_params_set_avail_min", buffer_size);
		err = snd_pcm_sw_params(capture, sw_params);
		if (err < 0)
			throw Exception("alsa-setup-fail");
		}


	// Sync.
	synced = playback && capture && (snd_pcm_link(playback, capture) >= 0);


	// Finish up.

	if (hw_params)
		snd_pcm_hw_params_free(hw_params);
	if (sw_params)
		snd_pcm_sw_params_free(sw_params);

	err = snd_pcm_prepare(playback);
	if (err < 0)
		throw Exception("alsa-setup-fail");
	if (capture)

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
			throw Exception("alsa-playback-fail");
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
		if (capture && !synced) {
			err = snd_pcm_start(capture);
			if (err < 0)
				log("snd_pcm_start() for capture  returned %d (\"%s\").", err, snd_strerror(err));
			}
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


bool ALSAAudioInterface::capture_is_ready()
{
	if (!initialized || !capture)
		return false;

	return snd_pcm_avail(capture) >= buffer_size;
}


void ALSAAudioInterface::capture_data(AudioBuffer** buffers, int num_channels)
{
	if (!initialized || !capture)
		return;

#ifdef USE_ALSA_MMAP
	int err;
	auto check_err = [&err, this](const char* call) -> void {
		if (err == -EPIPE)
			got_xrun();
		else if (err < 0) {
			log("%s failed for capture: %d (\"%s\").", call, err, snd_strerror(err));
			throw Exception("alsa-capture-fail");
			}
		};

	// Wait for the interface to be ready.
	err = snd_pcm_wait(capture, 1000);
	check_err("snd_pcm_wait");

	// Set up MMAP.
	const snd_pcm_channel_area_t* areas;
	snd_pcm_uframes_t offset, frames = buffer_size;
	err = snd_pcm_mmap_begin(capture, &areas, &offset, &frames);
	check_err("snd_pcm_mmap_begin");

	// Copy the samples.
	const snd_pcm_channel_area_t* area = areas;
	for (int which_channel = 0; which_channel < num_channels; ++which_channel, ++area) {
		char* in_buffer =
			(char*) area->addr + ((area->first + area->step * offset) >> 3);
		capture_move(
			in_buffer, buffers[which_channel]->samples,
			buffer_size, area->step >> 3);
		}

	// Return the samples to the hardware.
	err = snd_pcm_mmap_commit(capture, offset, frames);
	check_err("snd_pcm_mmap_commit");

#else 	// !USE_ALSA_MMAP

	// Get the samples.
	void* channel_buffers[num_channels];
	for (int which_channel = 0; which_channel < num_channels; ++which_channel)
		channel_buffers[which_channel] = buffers[which_channel]->samples;
	int err = snd_pcm_readn(capture, channel_buffers, buffer_size);
	if (err == -EPIPE)
		got_xrun();
	else if (err < 0) {
		log("snd_pcm_readn() returned %d (\"%s\").", err, snd_strerror(err));
		log("state = %d", snd_pcm_state(capture));
		log("buffers_sent: %d.", buffers_sent);
		throw Exception("alsa-capture-fail");
		}

#endif 	//  !USE_ALSA_MMAP
}


void ALSAAudioInterface::got_xrun()
{
	log("xrun");
	xruns += 1;
	engine->got_xrun();
	int err = snd_pcm_recover(playback, -EPIPE, 1);
	if (err < 0)
		log("snd_pcm_recover() returned %d (\"%s\").", err, snd_strerror(err));
#ifdef USE_ALSA_MMAP
	started = false;
#endif
}


void ALSAAudioInterface::got_capture_xrun()
{
	log("capture xrun");
	xruns += 1;
	engine->got_xrun();
	int err = snd_pcm_recover(capture, -EPIPE, 1);
	if (err < 0)
		log("snd_pcm_recover() returned %d (\"%s\").", err, snd_strerror(err));
#ifdef USE_ALSA_MMAP
	snd_pcm_start(capture);
#endif
}



