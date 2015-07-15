#include "OpenAudioFile.h"
#include "Exception.h"


OpenAudioFile::OpenAudioFile(std::string path)
{
	file = fopen(path.c_str(), "r");
	if (file == nullptr)
		throw Exception("open-file-error");
}


OpenAudioFile::~OpenAudioFile()
{
	fclose(file);
}



