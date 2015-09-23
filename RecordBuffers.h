#ifndef RecordBuffers_h
#define RecordBuffers_h

#include "Process.h"

class AudioBuffer;


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

		RecordBuffers*	next_free;

	protected:
		enum {
			Filling,
			Writing,
			Resupplying,
			Disposing,
			Done
			};

		struct Buffer {
			int	capture_channel;
			AudioBuffer*	buffer;
			};

		int	state;
		Buffer*	buffers;
		int	num_buffers;

		void	write();
		void	resupply();
		void	free_buffers();
	};


#endif	// !RecordBuffers_h

