#ifndef Clip_h
#define Clip_h

#include "AudioTypes.h"
#include <stdint.h>

class ProjectReader;
class AudioFile;
class AudioFileRead;
class AudioBuffer;


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
		unsigned long	file_end_frame()
			{ return file_start_frame + length_in_frames; }

		void	prepare_to_play();
		void	run(AudioBuffer** buffers_out, int num_channels);
		void	read_ahead();

		bool	contains_time(ProjectPosition time);
		ProjectPosition	end();

	protected:
		enum {
			num_reads = 2,
			};

		// Used during playback.
		AudioFileRead*	reads[num_reads];
		bool	playing;
		unsigned long	play_frame;

		void	init();
		void	start_read(unsigned long start_frame, unsigned long num_frames);
	};


#endif	// !Clip_h

