#ifndef Track_h
#define Track_h

#include <vector>
#include <string>
class Project;
class Playlist;
class Send;
class AudioBuffer;
class ProjectReader;
class SetTrackGainProcess;


class Track {
	public:
		Track(Project* project, int id = -1);
		~Track();

		void	read_json(ProjectReader* reader);
		std::string	api_json();
		std::string	clips_json();

		void	prepare_to_play();
		void	run(AudioBuffer** buffers_out, int num_channels);

		int	id;
		int	max_used_id();

	protected:
		Project*	project;
		std::string	name;
		Playlist*	playlist;
		std::vector<Track*>	children;
		std::vector<Send*>	sends;
		std::vector<Send*>	receives;
		float	gain;
		bool	sends_to_parent;

		friend class SetTrackGainProcess;
	};


#endif 	// !Track_h

