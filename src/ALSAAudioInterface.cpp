#include "ALSAAudioInterface.h"
#include "AudioBuffer.h"
#include "AudioEngine.h"
#include "SampleConversion.h"
#include "Exception.h"
#include "Logger.h"
#include <sstream>
#include <stdlib.h>
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
				play_move = to_float;
			else
			#endif
			if (try_format(playback, hw_params, SND_PCM_FORMAT_S32_LE))
				play_move = to_32;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S32_BE))
				play_move = to_32_be;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S24_3LE))
				play_move = to_24_le;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S24_3BE))
				play_move = to_24_be;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S16_LE))
				play_move = to_16;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S16_BE))
				play_move = to_16_be;
			else {
				err = -1;
				check_err("snd_pcm_hw_params_set_format", 0);
				}
		#else /* __BYTE_ORDER == __BIG_ENDIAN */
			#ifdef AUDIO_SAMPLE_FLOAT
			if (try_format(playback, hw_params, SND_PCM_FORMAT_FLOAT_BE))
				play_move = to_float;
			else
			#endif
			if (try_format(playback, hw_params, SND_PCM_FORMAT_S32_BE))
				play_move = to_32;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S32_LE))
				play_move = to_32_le;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S24_3BE))
				play_move = to_24_be;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S24_3LE))
				play_move = to_24_le;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S16_BE))
				play_move = to_16;
			else if (try_format(playback, hw_params, SND_PCM_FORMAT_S16_LE))
				play_move = to_16_le;
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
	unsigned int num_playback_channels = 0;
	snd_pcm_hw_params_get_channels_max(hw_params, &num_playback_channels);
	if (num_playback_channels > 1024) {
		// Dunno why this happens, or if it does for playback channels.  Assume
		// we only have two channels.
		num_playback_channels = 2;
		}
	this->num_playback_channels = num_playback_channels;
	snd_pcm_uframes_t actual_period_size = buffer_size;
	int dir = 0;
	err = snd_pcm_hw_params_set_period_size_near(playback, hw_params, &actual_period_size, &dir);
	check_err("snd_pcm_set_period_size", buffer_size);
	err = snd_pcm_hw_params_set_periods(playback, hw_params, 2, 0);
	check_err("snd_pcm_hw_params_set_periods", 2);
	snd_pcm_uframes_t actual_buffer_size = buffer_size * 2;
	err = snd_pcm_hw_params_set_buffer_size_near(playback, hw_params, &actual_buffer_size);
	check_err("snd_pcm_hw_params_set_buffer_size", actual_buffer_size);
	if ((int) actual_buffer_size != buffer_size)
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
					capture_move = from_float;
				else
				#endif
				if (try_format(capture, hw_params, SND_PCM_FORMAT_S32_LE))
					capture_move = from_32;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S32_BE))
					capture_move = from_32_be;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S24_3LE))
					capture_move = from_24_le;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S24_3BE))
					capture_move = from_24_be;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S16_LE))
					capture_move = from_16;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S16_BE))
					capture_move = from_16_be;
				else {
					err = -1;
					check_err("snd_pcm_hw_params_set_format", 0);
					}
			#else /* __BYTE_ORDER == __BIG_ENDIAN */
				#ifdef AUDIO_SAMPLE_FLOAT
				if (try_format(capture, hw_params, SND_PCM_FORMAT_FLOAT_BE))
					capture_move = from_float;
				else
				#endif
				if (try_format(capture, hw_params, SND_PCM_FORMAT_S32_BE))
					capture_move = from_32;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S32_LE))
					capture_move = from_32_le;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S24_3BE))
					capture_move = from_24_be;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S24_3LE))
					capture_move = from_24_le;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S16_BE))
					capture_move = from_16;
				else if (try_format(capture, hw_params, SND_PCM_FORMAT_S16_LE))
					capture_move = from_16_le;
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
		check_err("snd_pcm_hw_params_set_channels", num_capture_channels);
		actual_period_size = buffer_size;
		dir = 0;
		err = snd_pcm_hw_params_set_period_size_near(capture, hw_params, &actual_period_size, &dir);
		check_err("snd_pcm_set_period_size", buffer_size);
		err = snd_pcm_hw_params_set_periods(capture, hw_params, 2, 0);
		check_err("snd_pcm_hw_params_set_periods", 2);
		actual_buffer_size = buffer_size * 2;
		err = snd_pcm_hw_params_set_buffer_size_near(capture, hw_params, &actual_buffer_size);
		check_err("snd_pcm_hw_params_set_buffer_size", actual_buffer_size);
		if ((int) actual_buffer_size != buffer_size)
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

	// We'll start by sending one blank buffer.
#ifdef USE_ALSA_MMAP
	send_empty_buffer(num_channels);
#endif

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
			got_xrun(call);
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

	// Clear any channels not being used.
	// Ass-u-me'ing there's no blank space between areas or samples.
	int stride = areas[0].step >> 3;
	if (num_channels < num_playback_channels)
		memset((char*) areas[0].addr + stride * offset, 0, frames * stride);

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
	int which_channel = 0;
	for (; which_channel < num_channels; ++which_channel)
		channel_buffers[which_channel] = buffers[which_channel]->samples;
	void* empty_channel_buffer = nullptr;
	if (num_channels < num_playback_channels) {
		empty_channel_buffer = malloc(buffer_size * sizeof(float));
		memset(empty_channel_buffer, 0, buffer_size * sizeof(float));
		for (; which_channel < num_playback_channels; ++which_channel)
			channel_buffers[which_channel] = empty_channel_buffer;
		}
	int err = snd_pcm_writen(playback, channel_buffers, buffer_size);
	if (num_channels < num_playback_channels)
		free(empty_channel_buffer);
	if (err == -EPIPE)
		got_xrun("snd_pcm_writen");
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

	return started;
}


void ALSAAudioInterface::capture_data(AudioBuffer** buffers, int num_channels)
{
	if (!initialized || !capture)
		return;

#ifdef USE_ALSA_MMAP
	int err;
	auto check_err = [&err, this](const char* call) -> void {
		if (err == -EPIPE)
			got_capture_xrun(call);
		else if (err < 0) {
			log("%s failed for capture: %d (\"%s\").", call, err, snd_strerror(err));
			throw Exception("alsa-capture-fail");
			}
		};

	// Wait for the interface to be ready.
	err = snd_pcm_wait(capture, 1000);
	if (err == -EPIPE) {
		got_capture_xrun("snd_pcm_wait");
		for (int which_channel = 0; which_channel < num_channels; ++which_channel)
			buffers[which_channel]->clear();
		return;
		}
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
		got_capture_xrun("snd_pcm_readn");
	else if (err < 0) {
		log("snd_pcm_readn() returned %d (\"%s\").", err, snd_strerror(err));
		log("state = %d", snd_pcm_state(capture));
		log("buffers_sent: %d.", buffers_sent);
		throw Exception("alsa-capture-fail");
		}

#endif 	//  !USE_ALSA_MMAP
}


std::string ALSAAudioInterface::input_names_json()
{
	std::stringstream json;
	json << "[";

	// Mono.
	if (num_capture_channels > 0) {
		json << "{ \"category\": \"Mono\", \"inputs\": [ ";
		bool started = false;
		for (int index = 0; index < num_capture_channels; ++index) {
			if (started)
				json << ", ";
			else
				started = true;
			json << "\"In " << (index + 1) << "\"";
			}
		json << " ] }";
		}

	// Stereo.
	if (num_capture_channels > 1) {
		json << ", ";
		json << "{ \"category\": \"Stereo\", \"inputs\": [ ";
		bool started = false;
		for (int index = 0; index < num_capture_channels - 1; index += 2) {
			if (started)
				json << ", ";
			else
				started = true;
			json << "\"Stereo " << (index + 1) << "/" << (index + 2)  << "\"";
			}
		json << " ] }";
		}

	json << "]";
	return json.str();
}


std::vector<int>* ALSAAudioInterface::capture_channels_for_input_name(std::string name)
{
	if (name.find("In ") == 0) {
		std::vector<int>* result = new std::vector<int>();
		result->push_back(atoi(name.substr(3).c_str()) - 1);
		return result;
		}

	else if (name.find("Stereo ") == 0) {
		std::string channels = name.substr(7);
		size_t index = channels.find('/');
		if (index == std::string::npos)
			return nullptr;
		int first_channel = atoi(channels.substr(0, index).c_str()) - 1;
		std::vector<int>* result = new std::vector<int>();
		result->push_back(first_channel);
		result->push_back(first_channel + 1);
		return result;
		}

	return nullptr;
}


void ALSAAudioInterface::got_xrun(const char* call)
{
	log("xrun from %s()", call);
	xruns += 1;
	engine->got_xrun();
	int err = snd_pcm_recover(playback, -EPIPE, 1);
	if (err < 0)
		log("snd_pcm_recover() returned %d (\"%s\").", err, snd_strerror(err));
#ifdef USE_ALSA_MMAP
	send_empty_buffer(2);
	started = false;
#endif
}


void ALSAAudioInterface::got_capture_xrun(const char* call)
{
	log("capture xrun from %s()", call);
	xruns += 1;
	engine->got_capture_xrun();

#ifdef USE_ALSA_MMAP

	int err = snd_pcm_prepare(playback);
	if (err < 0)
		log("snd_pcm_prepare(playback) returned %d (\"%s\").", err, snd_strerror(err));
	if (capture && !synced) {
		err = snd_pcm_prepare(capture);
		if (err < 0)
			log("snd_pcm_prepare(capture) returned %d (\"%s\").", err, snd_strerror(err));
		}

	started = false;
	send_empty_buffer(2);

#else 	// !USE_ALSA_MMAP

	int err = snd_pcm_recover(capture, -EPIPE, 1);
	if (err < 0)
		log("snd_pcm_recover() returned %d (\"%s\").", err, snd_strerror(err));
#ifdef USE_ALSA_MMAP
	err = snd_pcm_start(capture);
	if (err < 0)
		log("capture snd_pcm_start() returned %d (\"%s\").", err, snd_strerror(err));
#endif

#endif 	// !USE_ALSA_MMAP
}


void ALSAAudioInterface::send_empty_buffer(int num_channels)
{
#ifdef USE_ALSA_MMAP
	int err;
	auto check_err = [&err](const char* call) -> void {
		if (err >= 0)
			return;
		log("%s() failed: %d (\"%s\").", call, err, snd_strerror(err));
		throw Exception("alsa-setup-fail");
		};

	AudioBuffer* buffer = engine->get_buffer();
	buffer->clear();

	err = snd_pcm_wait(playback, 1000);
	check_err("snd_pcm_wait");

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
			buffer->samples, out_buffer,
			buffer_size, area->step >> 3);
		}

	// Send the samples to the hardware.
	err = snd_pcm_mmap_commit(playback, offset, frames);
	check_err("snd_pcm_mmap_commit");

	engine->free_buffer(buffer);
#endif
}



