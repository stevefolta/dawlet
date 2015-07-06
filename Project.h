#ifndef Project_h
#define Project_h

#include <vector>
class Track;
class AudioBuffer;
class JSONParser;


class Project {
	public:
		Project();
		~Project();

		void	read_json(JSONParser* parser);

		void	run(AudioBuffer* buffer_out);

		int	new_id();
		int	max_used_id();

	protected:
		Track*	master;
		int	next_new_id;
	};


#endif 	// !Project_h

