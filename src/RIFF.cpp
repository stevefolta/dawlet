#include "RIFF.h"


bool fourcc_eq(const void* data_in, const char* code)
{
	const char* data = (const char*) data_in;
	return
		data[0] == code[0] &&
		data[1] == code[1] &&
		data[2] == code[2] &&
		data[3] == code[3];
}


int32_t read_dword(const void* data_in)
{
	const uint8_t* data = (const uint8_t*) data_in;
	uint32_t result =
		((uint32_t) data[3]) << 24 |
		((uint32_t) data[2]) << 16 |
		((uint32_t) data[1]) << 8 |
		((uint32_t) data[0]);
	return (int32_t) result;
}


int16_t read_word(const void* data_in)
{
	const uint8_t* data = (const uint8_t*) data_in;
	uint16_t result =
		((uint16_t) data[1]) << 8 |
		((uint16_t) data[0]);
	return (int16_t) result;
}


void* write_dword(int32_t value, void* data_in)
{
	uint8_t* data = (uint8_t*) data_in;
	data[3] = value >> 24;
	data[2] = value >> 16;
	data[1] = value >> 8;
	data[0] = value;
	return data;
}


void* write_word(int16_t value, void* data_in)
{
	uint8_t* data = (uint8_t*) data_in;
	data[1] = value >> 8;
	data[0] = value;
	return data;
}



