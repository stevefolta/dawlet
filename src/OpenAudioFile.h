#ifndef OpenAudioFile_h
#define OpenAudioFile_h

#include "AudioFile.h"
#include <string>


class OpenAudioFile {
	public:
		OpenAudioFile(std::string path, int dirfd);
		virtual ~OpenAudioFile();

		virtual AudioFile::Info	read_info() = 0;
		virtual unsigned long	offset_for_frame(unsigned long which_frame) = 0;
		virtual unsigned long	size_of_frames(unsigned long num_frames) = 0;

		int	fd;

	protected:
	};


#endif	// !OpenAudioFile_h

