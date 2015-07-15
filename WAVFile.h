#ifndef WAVFile_h
#define WAVFile_h

#include "OpenAudioFile.h"


class WAVFile : public OpenAudioFile {
	public:
		WAVFile(std::string path);

		AudioFile::Info	read_info();

	protected:
		long	samples_offset, file_end, data_chunk_size;

		long	seek_chunk(const char* fourcc);
		void	throw_error();
	};


#endif	// !WAVFile_h


