#ifndef WAVFile_h
#define WAVFile_h

#include "OpenAudioFile.h"


class WAVFile : public OpenAudioFile {
	public:
		WAVFile(std::string path, int dirfd);

		AudioFile::Info	read_info();
		unsigned long	offset_for_frame(unsigned long which_frame);
		unsigned long	size_of_frames(unsigned long num_frames);

	protected:
		long	samples_offset, file_end, data_chunk_size;
		AudioFile::Info	info;

		long	seek_chunk(const char* fourcc);
		void	throw_error();
	};


#endif	// !WAVFile_h


