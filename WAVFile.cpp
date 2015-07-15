#include "WAVFile.h"
#include "RIFF.h"
#include "Exception.h"

enum {
	WAVE_FORMAT_PCM = 1,
	};


WAVFile::WAVFile(std::string path)
	: OpenAudioFile(path)
{
	// Find where the samples are.
	char fourcc[4];
	int32_t chunk_length;
	rewind(file);
	int result = fread(fourcc, 1, sizeof(fourcc), file);
	if (result != sizeof(fourcc) || !fourcc_eq(fourcc, "WAVE"))
		throw_error();
	long uber_chunk_size;
	result = fread(&uber_chunk_size, 1, sizeof(uber_chunk_size), file);
	if (result != sizeof(uber_chunk_size))
		throw_error();
	uber_chunk_size = read_dword(&uber_chunk_size);
	file_end = RIFF::chunk_header_size + uber_chunk_size;
	data_chunk_size = seek_chunk("data");
	if (data_chunk_size < 0)
		throw_error();
	samples_offset = ftell(file);
}


AudioFile::Info WAVFile::read_info()
{
	long chunk_size = seek_chunk("fmt ");
	if (chunk_size < 14)
		throw_error();

	AudioFile::Info info;
	int16_t word;
	int result = fread(&word, 1, sizeof(word), file);
	if (result != sizeof(word))
		throw_error();
	int16_t format_tag = read_word(&word);
	if (format_tag != WAVE_FORMAT_PCM)
		throw Exception("unsupported-WAV-format");
	result = fread(&word, 1, sizeof(word), file);
	if (result != sizeof(word))
		throw_error();
	info.num_channels = read_word(&word);
	int32_t dword;
	result = fread(&dword, 1, sizeof(dword), file);
	if (result != sizeof(dword))
		throw_error();
	info.sample_rate = read_dword(&dword);
	// We don't care about "nAvgBytesPerSec" or "nBlockAlign", skip them.
	result = fseek(file, 4 + 2, SEEK_CUR);
	if (result < 0)
		throw_error();
	result = fread(&word, 1, sizeof(word), file);
	if (result != sizeof(word))
		throw_error();
	info.bits_per_sample = read_word(&word);
	if (info.bits_per_sample != 16 && info.bits_per_sample != 24)
		throw Exception("unspported-WAV-format");

	info.length_in_samples = data_chunk_size / (info.bits_per_sample / 8);
}


long WAVFile::seek_chunk(const char* fourcc)
{
	long position = RIFF::chunk_header_size;
	int result = fseek(file, position, SEEK_SET);
	if (result == -1)
		throw_error();

	while (position < file_end) {
		// Read the next chunk header.
		char chunk_fourcc[4];
		result = fread(chunk_fourcc, 1, sizeof(chunk_fourcc), file);
		if (result != sizeof(chunk_fourcc))
			throw_error();
		long chunk_size;
		result = fread(&chunk_size, 1, sizeof(chunk_size), file);
		if (result != sizeof(chunk_size))
			throw_error();
		chunk_size = read_dword(&chunk_size);
		position += RIFF::chunk_header_size;

		// Is this the one?
		if (fourcc_eq(chunk_fourcc, fourcc))
			return chunk_size;

		// Keep looking.
		position += chunk_size;
		result = fseek(file, position, SEEK_SET);
		if (result == -1)
			throw_error();
		}

	return false;
}


void WAVFile::throw_error()
{
	throw Exception("invalid-wav-file");
}



