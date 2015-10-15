#ifndef SeekProcess_h
#define SeekProcess_h

#include "Process.h"
#include "AudioEngine.h"


class SeekProcess : public Process {
	public:
		SeekProcess(ProjectPosition positionIn)
			: position(positionIn), state(InEngine) {}

		bool	is_done();
		void	next();

	protected:
		enum {
			InEngine,
			Done,
			};

		ProjectPosition	position;
		int	state;
	};


#endif	// !SeekProcess_h

