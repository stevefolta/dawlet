#ifndef Clip_h
#define Clip_h

#include "AudioTypes.h"
#include <stdint.h>

class ProjectReader;
class AudioFile;
class AudioFileRead;


class Clip {
	public:
		Clip(AudioFile* file_in)
			: file(file_in) { init(); }
		Clip()
			: file(nullptr), length_in_frames(0) { init(); }
		~Clip();

		void	read_json(ProjectReader* reader);

		AudioFile*	file;
		ProjectPosition	start;
		unsigned long	file_start_frame;
		unsigned long	length_in_frames;

		void	read_ahead();

	protected:
		// Used during playback.
		AudioFileRead*	reads[2];

		void	init();
		void	start_read(unsigned long start_frame, unsigned long num_frames);
	};


#endif	// !Clip_h

