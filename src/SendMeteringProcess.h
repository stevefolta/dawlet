#ifndef SendMeteringProcess_h
#define SendMeteringProcess_h

#include "Process.h"
#include "AudioTypes.h"


class SendMeteringProcess : public Process {
	public:
		SendMeteringProcess();

		bool	is_done();
		void	next();
		bool	return_immediately();

		bool	is_full();
		bool	is_empty() { return num_peaks == 0; }
		void	add_peak(int track_id, AudioSample peak);

		SendMeteringProcess*	next_peaks;

		enum {
			peaks_per_send = 20,
			};

	protected:
		enum {
			Installing,
			Sending,
			Done,
			};

		struct Peak {
			int	track_id;
			AudioSample	peak;
			};
		Peak peaks[peaks_per_send];

		int state;
		int num_peaks;

		void	send();
	};



#endif	// !SendMeteringProcess_h

