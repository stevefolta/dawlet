#ifndef SampleConversion_h
#define SampleConversion_h

#include "AudioTypes.h"


extern void to_float(const AudioSample* in, char* out, int frames, int step);
extern void to_32(const AudioSample* in, char* out, int frames, int step);
extern void to_32_be(const AudioSample* in, char* out, int frames, int step);
extern void to_32_le(const AudioSample* in, char* out, int frames, int step);
extern void to_24_le(const AudioSample* in, char* out, int frames, int step);
extern void to_24_be(const AudioSample* in, char* out, int frames, int step);
extern void to_16(const AudioSample* in, char* out, int frames, int step);
extern void to_16_be(const AudioSample* in, char* out, int frames, int step);
extern void to_16_le(const AudioSample* in, char* out, int frames, int step);


extern void from_float(const char* in, AudioSample* out, int frames, int step);
extern void from_32(const char* in, AudioSample* out, int frames, int step);
extern void from_32_be(const char* in, AudioSample* out, int frames, int step);
extern void from_32_le(const char* in, AudioSample* out, int frames, int step);
extern void from_24_le(const char* in, AudioSample* out, int frames, int step);
extern void from_24_be(const char* in, AudioSample* out, int frames, int step);
extern void from_16(const char* in, AudioSample* out, int frames, int step);
extern void from_16_be(const char* in, AudioSample* out, int frames, int step);
extern void from_16_le(const char* in, AudioSample* out, int frames, int step);

#endif	// !SampleConversion_h

