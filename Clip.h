#ifndef Clip_h
#define Clip_h

#include "AudioTypes.h"
#include <stdint.h>

class ProjectReader;
class AudioFile;


class Clip {
	public:
		Clip(AudioFile* file_in)
			: file(file_in) {}
		Clip()
			: file(nullptr), length_in_samples(0) {}
		~Clip();

		void	read_json(ProjectReader* reader);

		AudioFile*	file;
		ProjectPosition	start;
		unsigned long	length_in_samples;

	protected:
	};


#endif	// !Clip_h

