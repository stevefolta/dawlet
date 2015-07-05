#ifndef Project_h
#define Project_h

#include <vector>
class Track;
class AudioBuffer;


class Project {
	public:
		Project();
		~Project();

		void	run(AudioBuffer* bufferOut);

		int	new_id();

	protected:
		Track*	master;
		std::vector<Track*>	tracks;
		int	next_new_id;
	};


#endif 	// !Project_h

