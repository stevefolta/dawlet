#ifndef Project_h
#define Project_h

#include <vector>
#include <string>
class Track;
class AudioFile;
class AudioBuffer;
class ProjectReader;


class Project {
	public:
		Project(std::string path_in);
		~Project();

		void	read_json(ProjectReader* reader);
		std::string	api_json();

		void	prepare_to_play();
		void	run(AudioBuffer* buffer_out);

		int	new_id();
		int	max_used_id();

		void	add_file(AudioFile* file);

		int	get_dirfd();
		void	load_audio_file_info();

	protected:
		std::string	path;
		Track*	master;
		std::vector<AudioFile*>	files;
		int	next_new_id;
		int	dirfd;
	};


#endif 	// !Project_h

