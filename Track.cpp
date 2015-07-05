#include "Track.h"
#include "BufferManager.h"
#include "Project.h"
#include "Playlist.h"
#include "Send.h"
#include "AudioEngine.h"
#include "Amp.h"


Track::Track(Project* projectIn, int idIn)
	: project(projectIn)
{
	id = idIn >=0 ? idIn : project->new_id();
	gain = 1.0;
}


Track::~Track()
{
	while (!sends.empty()) {
		delete sends.back();
		sends.pop_back();
		}
	// Don't delete receives; their sends will do that.
	while (!children.empty()) {
		delete children.back();
		children.pop_back();
		}
}


void Track::run(AudioBuffer* bufferOut)
{
	// Get a buffer for the track.
	int buffer_size = audioEngine->buffer_size();
	AudioBuffer* track_buffer = audioEngine->get_buffer();
	track_buffer->clear();

	// Playlist.
	playlist->run(track_buffer);

	// Children.
	if (!children.empty()) {
		for (auto it = children.begin(); it != children.end(); ++it) {
			// Run the child track.
			Track* child = *it;
			child->run(track_buffer);
			}
		}

	// Receives.
	/***/

	// Mix the track into the output buffer.
	AudioSample* in = track_buffer->samples;
	AudioSample* out = bufferOut->samples;
	for (int samples_left = buffer_size; samples_left > 0; --samples_left) {
		*out += amp(gain, *in++);
		++out;
		}
}



