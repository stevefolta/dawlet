#ifndef StartRecordingProcess_h
#define StartRecordingProcess_h

#include "Process.h"
#include <vector>

struct RecordingClip;
class RecordBuffers;


class StartRecordingProcess : public Process {
	public:
		StartRecordingProcess(std::vector<RecordingClip>* recording_clips_in);
		~StartRecordingProcess();

		bool	is_done();
		void	next();

		void	add_recording_buffers(RecordBuffers* new_buffers)
			{ record_bufferses.push_back(new_buffers); }

	protected:
		enum {
			Starting,
			Done,
			};
		int state;

		std::vector<RecordingClip>*	recording_clips;
		std::vector<RecordBuffers*>	record_bufferses;

		void	start_recording();
	};


#endif	// !StartRecordingProcess_h

