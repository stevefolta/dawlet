#include "SampleConversion.h"
#include <stdint.h>


void to_float(const AudioSample* in, char* out, int frames, int step)
{
	while (frames-- > 0) {
		*(float*) out = *in++;
		out += step;
		}
}

void to_32(const AudioSample* in, char* out, int frames, int step)
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

void to_32_be(const AudioSample* in, char* out, int frames, int step)
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

void to_32_le(const AudioSample* in, char* out, int frames, int step)
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

void to_24_le(const AudioSample* in, char* out, int frames, int step)
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

void to_24_be(const AudioSample* in, char* out, int frames, int step)
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

void to_16(const AudioSample* in, char* out, int frames, int step)
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

void to_16_be(const AudioSample* in, char* out, int frames, int step)
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

void to_16_le(const AudioSample* in, char* out, int frames, int step)
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



void from_float(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		*out++ = *(float*) in;
		in += step;
		}
}

void from_32(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		*out++ = *(int32_t*) in / (AudioSample) 0x7FFFFFFF;
		in += step;
		}
}

void from_32_be(const char* in, AudioSample* out, int frames, int step)
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

void from_32_le(const char* in, AudioSample* out, int frames, int step)
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

void from_24_le(const char* in, AudioSample* out, int frames, int step)
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

void from_24_be(const char* in, AudioSample* out, int frames, int step)
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

void from_16(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		*out++ = *(int16_t*) in / (AudioSample) 0x7FFF;
		in += step;
		}
}

void from_16_be(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		int16_t sample =
			(in[0] & 0xFF) << 8 |
			(in[1] & 0xFF);
		*out++ = sample / (AudioSample) 0x7FFF;
		in += step;
		}
}

void from_16_le(const char* in, AudioSample* out, int frames, int step)
{
	while (frames-- > 0) {
		int16_t sample =
			(in[0] & 0xFF) |
			(in[1] & 0xFF) << 8;
		*out++ = sample / (AudioSample) 0x7FFF;
		in += step;
		}
}

