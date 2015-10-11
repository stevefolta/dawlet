#ifndef Track_h
#define Track_h

#include "AudioTypes.h"
#include <vector>
#include <string>
class Project;
class Playlist;
class Send;
class AudioBuffer;
class ProjectReader;
class SetTrackGainProcess;
class ArmTrackProcess;
class IndentedOStream;
class Recorder;
class Clip;


class Track {
	public:
		Track(Project* project, Track* parent_in, int id = -1);
		~Track();

		void	read_json(ProjectReader* reader);
		std::string	api_json();
		std::string	clips_json();
		void	write_to_file(IndentedOStream& stream);

		void	prepare_to_play();
		void	run(AudioBuffer** buffers_out, int num_channels);
		void	run_metering();
		void	arm_armed_tracks(Recorder* recorder);
		void	add_clip(Clip* clip);
		void	add_child(Track* track, Track* after_child = nullptr);
		void	remove_child(Track* track);

		int	id;
		int	max_used_id();
		int	total_num_tracks();
		std::string	get_name() { return name; }
		std::string	get_input() { return input; }
		Track*	get_parent() { return parent; }

		void	set_name(std::string new_name) { name = new_name; }

	protected:
		Project*	project;
		Track*	parent;
		std::string	name;
		Playlist*	playlist;
		std::vector<Track*>	children;
		std::vector<Send*>	sends;
		std::vector<Send*>	receives;
		std::string	input;
		float	gain;
		bool	sends_to_parent;
		bool record_armed, monitor_input;
		AudioSample	cur_peak;

		// Only kept up-to-date when armed:
		std::vector<int>*	capture_channels;

		friend class SetTrackGainProcess;
		friend class ArmTrackProcess;
		friend class UnarmTrackProcess;
		friend class SetTrackInputProcess;
		friend class SetTrackMonitorInputProcess;
	};


#endif 	// !Track_h

