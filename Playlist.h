#ifndef Playlist_h
#define Playlist_h

#include <vector>

class Clip;
class AudioBuffer;
class ProjectReader;


class Playlist {
	public:
		Playlist();
		~Playlist();

		void	read_json(ProjectReader* reader);

		void	run(AudioBuffer* buffer_out);

	protected:
		std::vector<Clip*>	clips;
	};


#endif 	// !Playlist_h

