#ifndef OpenAudioFile_h
#define OpenAudioFile_h

#include "AudioFile.h"
#include <string>


class OpenAudioFile {
	public:
		OpenAudioFile(std::string path);
		virtual ~OpenAudioFile();

		virtual AudioFile::Info	read_info() = 0;

	protected:
		int	fd;
	};


#endif	// !OpenAudioFile_h

