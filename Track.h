#ifndef Track_h
#define Track_h

#include <vector>
class Project;
class Playlist;
class Send;
class AudioBuffer;


class Track {
	public:
		Track(Project* project, int id = -1);
		~Track();

		void	run(AudioBuffer* bufferOut);

		int	id;

	protected:
		Project*	project;
		Playlist*	playlist;
		std::vector<Track*>	children;
		std::vector<Send*>	sends;
		std::vector<Send*>	receives;
		float	gain;
	};


#endif 	// !Track_h

