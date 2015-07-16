#include "OpenAudioFile.h"
#include "Exception.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


OpenAudioFile::OpenAudioFile(std::string path)
{
	fd = open(path.c_str(), O_RDONLY);
	if (fd == -1)
		throw Exception("open-file-error");
}


OpenAudioFile::~OpenAudioFile()
{
	if (fd != -1)
		close(fd);
}



