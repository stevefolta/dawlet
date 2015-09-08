#ifndef SetTrackStateProcesses_h
#define SetTrackStateProcesses_h

#include "MutatingProcess.h"
#include "Track.h"
namespace Web {
	class Connection;
	}


class SetTrackStateProcess : public MutatingProcess {
	public:
		SetTrackStateProcess(Track* track, Web::Connection* connection);
		bool	is_done();
		void	next();

	protected:
		enum {
			Setting,
			Replying,
			Done
			};

		virtual void	set() = 0;

		int	state;
		Track*	track;
		Web::Connection*	connection;
	};


class SetTrackGainProcess : public SetTrackStateProcess {
	public:
		SetTrackGainProcess(
			Track* track_in, double gain_in, Web::Connection* connection_in)
			: SetTrackStateProcess(track_in, connection_in), gain(gain_in)
			{}

	protected:
		double gain;

		void set() { track->gain = gain; }
	};


class SetTrackNameProcess : public SetTrackStateProcess {
	public:
		SetTrackNameProcess(
			Track* track_in, std::string name_in, Web::Connection* connection_in)
			: SetTrackStateProcess(track_in, connection_in), name(name_in)
			{}

	protected:
		std::string	name;

		void set() { track->set_name(name); }
	};


class SetTrackRecordArmedProcess : public SetTrackStateProcess {
	public:
		SetTrackRecordArmedProcess(
			Track* track_in, bool new_armed_in, Web::Connection* connection_in)
			: SetTrackStateProcess(track_in, connection_in), new_armed(new_armed_in)
			{}

	protected:
		bool	new_armed;

		void	set() { track->record_armed = new_armed; }
	};


#endif	// !SetTrackStateProcesses_h

