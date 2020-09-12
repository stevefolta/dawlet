#ifndef StartRecordingProcess_h
#define StartRecordingProcess_h

#include "Process.h"
#include <vector>
#include <string>

struct RecordingClip;
class RecordBuffers;


class StartRecordingProcess : public SimpleProcess {
	public:
		StartRecordingProcess(
			std::vector<RecordingClip>* recording_clips_in,
			std::string clip_specs_in);
		~StartRecordingProcess();

		void	add_recording_buffers(RecordBuffers* new_buffers)
			{ record_bufferses.push_back(new_buffers); }

	protected:
		std::vector<RecordingClip>*	recording_clips;
		std::vector<RecordBuffers*>	record_bufferses;
		std::string 	clip_specs;
		float	start_time;

		void	in_engine();
		void	back_in_daw();
		void	start_recording();
	};


#endif	// !StartRecordingProcess_h

