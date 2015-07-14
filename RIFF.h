#ifndef RIFF_h
#define RIFF_h

#include <stdint.h>

extern bool fourcc_eq(const void* data, const char* code);
extern int32_t read_dword(const void* data);
extern int16_t read_word(const void* data);


#endif	// !RIFF_h
