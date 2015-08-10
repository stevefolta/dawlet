#ifndef Playlist_h
#define Playlist_h

#include <vector>
#include <string>

class Clip;
class AudioBuffer;
class ProjectReader;


class Playlist {
	public:
		Playlist();
		~Playlist();

		void	read_json(ProjectReader* reader);
		std::string	clips_json();

		void	prepare_to_play();
		void	run(AudioBuffer** buffers_out, int num_channels);

	protected:
		std::vector<Clip*>	clips;

		// While playing.
		std::vector<Clip*>::iterator	first_playing_clip;
	};


#endif 	// !Playlist_h

