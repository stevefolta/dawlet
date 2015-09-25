#ifndef SetTrackStateProcesses_h
#define SetTrackStateProcesses_h

#include "MutatingProcess.h"
#include "Track.h"
namespace Web {
	class Connection;
	}


class SetTrackStateProcess : public MutatingProcess {
	public:
		SetTrackStateProcess(Track* track, Web::Connection* connection, bool really_mutating = true);
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


class ArmTrackProcess : public SetTrackStateProcess {
	public:
		ArmTrackProcess(
			Track* track_in, Web::Connection* connection_in, std::vector<int>* capture_channels_in)
			: SetTrackStateProcess(track_in, connection_in, (connection_in != nullptr)),
			  new_capture_channels(capture_channels_in), old_capture_channels(nullptr)
			{}
		~ArmTrackProcess() {
			delete old_capture_channels;
			}

	protected:
		std::vector<int>*	new_capture_channels;
		std::vector<int>*	old_capture_channels;

		void set() {
			track->record_armed = true;
			old_capture_channels = track->capture_channels;
			track->capture_channels = new_capture_channels;
			}
	};

class UnarmTrackProcess : public SetTrackStateProcess {
	public:
		UnarmTrackProcess(Track* track_in, Web::Connection* connection_in)
			: SetTrackStateProcess(track_in, connection_in) {}

	protected:
		void	set() { track->record_armed = false; }
	};

class SetTrackInputProcess : public SetTrackStateProcess {
	public:
		SetTrackInputProcess(
			Track* track_in, std::string input_in,
			Web::Connection* connection_in, std::vector<int>* capture_channels_in)
			: SetTrackStateProcess(track_in, connection_in),
			  input(input_in), new_capture_channels(capture_channels_in)
			{}
		~SetTrackInputProcess()
			{ delete old_capture_channels; }

	protected:
		std::string	input;
		std::vector<int>*	new_capture_channels;
		std::vector<int>*	old_capture_channels;

		void set() {
			track->input = input;
			old_capture_channels = track->capture_channels;
			track->capture_channels = new_capture_channels;
			}
	};

class SetTrackMonitorInputProcess : public SetTrackStateProcess {
	public:
		SetTrackMonitorInputProcess(
			Track* track_in, bool monitor_input_in, Web::Connection* connection_in)
			: SetTrackStateProcess(track_in, connection_in), monitor_input(monitor_input_in)
			{}

	protected:
		bool	monitor_input;

		void set() { track->monitor_input = monitor_input; }
	};


#endif	// !SetTrackStateProcesses_h

