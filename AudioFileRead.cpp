#include "AudioFileRead.h"
#include <stdlib.h>


AudioFileRead::AudioFileRead()
	: next_free(nullptr), state(Waiting), buffer(nullptr)
{
}


AudioFileRead::~AudioFileRead()
{
	if (buffer)
		free(buffer);
}


bool AudioFileRead::is_done()
{
	return state == Done;
}


void AudioFileRead::next()
{
	switch (state) {
		}
}



