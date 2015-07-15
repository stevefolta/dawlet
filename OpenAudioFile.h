#ifndef OpenAudioFile_h
#define OpenAudioFile_h

#include "AudioFile.h"
#include <string>
#include <stdio.h>


class OpenAudioFile {
	public:
		OpenAudioFile(std::string path);
		virtual ~OpenAudioFile();

		virtual AudioFile::Info	read_info() = 0;

	protected:
		FILE*	file;
	};


#endif	// !OpenAudioFile_h

