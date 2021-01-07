#ifndef Playlist_h
#define Playlist_h

#include "List.h"
#include <string>

class Clip;
class AudioBuffer;
class ProjectReader;
class IndentedOStream;


class Playlist {
	public:
		Playlist();
		~Playlist();

		void	read_json(ProjectReader* reader);
		std::string	clips_json();
		void	write_to_file(IndentedOStream& stream);

		void	prepare_to_play();
		void	run(AudioBuffer** buffers_out, int num_channels);

		bool	is_empty() { return clips.empty(); }
		int	max_used_id();

		void	add_clip(Clip* clip);

	protected:
		List<Clip>	clips;

		// While playing.
		List<Clip>::iterator	first_playing_clip;
	};


#endif 	// !Playlist_h

