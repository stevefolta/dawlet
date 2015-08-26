#ifndef Project_h
#define Project_h

#include <vector>
#include <string>
#include <map>
class Track;
class AudioFile;
class AudioBuffer;
class ProjectReader;
class IndentedOStream;


class Project {
	public:
		Project(std::string path_in);
		~Project();

		void	read_json(ProjectReader* reader);
		std::string	api_json();
		void	write_to_file(IndentedOStream& stream);

		void	prepare_to_play();
		void	run(AudioBuffer** buffers_out, int num_channels);

		int	new_id();
		int	max_used_id();

		void	add_file(AudioFile* file);

		int	get_dirfd();
		void	load_audio_file_info();

		void	add_track_by_id(Track* track);
		void	remove_track_by_id(int id) { tracks_by_id.erase(id); }
		Track*	track_by_id(int id) { return tracks_by_id[id]; }

	protected:
		std::string	path;
		Track*	master;
		std::vector<AudioFile*>	files;
		int	next_new_id;
		int	dirfd;
		std::map<int, Track*>	tracks_by_id;
	};


#endif 	// !Project_h

