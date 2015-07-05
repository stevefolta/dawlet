#include "AudioBuffer.h"
#include "AudioEngine.h"
#include <string.h>


void AudioBuffer::clear()
{
	memset(samples, 0, engine->buffer_size() * sizeof(AudioSample));
}


