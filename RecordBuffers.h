#ifndef RecordBuffers_h
#define RecordBuffers_h

#include "Process.h"

class AudioBuffer;


// The "RecordBuffers" object manages the passing of AudioBuffers from the DAW
// thread to the Engine thread (and back) for recording.  It holds buffers for
// all the channels that are being recorded for a single capture period.

class RecordBuffers : public Process {
	public:
		RecordBuffers(int num_channels);
		~RecordBuffers();

		bool	is_done();
		void	next();
		bool	return_immediately();

		// Called by the engine.
		void	add(int capture_channel, AudioBuffer* buffer);
		void	start_write();
		void	dispose();

		struct Buffer {
			int	capture_channel;
			AudioBuffer*	buffer;
			};

		Buffer*	begin() { return buffers; }
		Buffer*	end() { return &buffers[num_buffers]; }

		RecordBuffers*	next_free;

	protected:
		enum {
			Filling,
			Writing,
			Resupplying,
			Disposing,
			Done
			};

		int	state;
		Buffer*	buffers;
		int	num_buffers;

		void	write();
		void	resupply();
		void	free_buffers();
	};


#endif	// !RecordBuffers_h

