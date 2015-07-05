#include "AudioBuffer.h"
#include "AudioEngine.h"
#include <string.h>


void AudioBuffer::clear()
{
	memset(samples, 0, audioEngine->buffer_size() * sizeof(AudioSample));
}


