#include "AudioInterface.h"
#include <string.h>


AudioInterface::~AudioInterface()
{
	delete armed_channels;
}


void AudioInterface::setup_armed_channels()
{
	delete armed_channels;
	int num_channels = get_num_capture_channels();
	armed_channels = new bool[num_channels];
	memset(armed_channels, 0, num_channels * sizeof(bool));
}



