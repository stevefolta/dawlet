#ifndef Project_h
#define Project_h

#include <vector>
class Track;
class AudioFile;
class AudioBuffer;
class ProjectReader;


class Project {
	public:
		Project();
		~Project();

		void	read_json(ProjectReader* reader);

		void	run(AudioBuffer* buffer_out);
		void	read_ahead();

		int	new_id();
		int	max_used_id();

		void	add_file(AudioFile* file);

	protected:
		Track*	master;
		std::vector<AudioFile*>	files;
		int	next_new_id;
	};


#endif 	// !Project_h

