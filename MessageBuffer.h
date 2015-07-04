#ifndef MessageBuffer_h
#define MessageBuffer_h

#include "Message.h"
#include <atomic>

// A ring buffer for sending messages to or from the realtime thread.


class MessageBuffer {
	public:
		MessageBuffer();
		~MessageBuffer();

		// Sending.
		Message*	back();
		void	push();

		// Receiving.
		Message*	front();
		void	pop();

		bool is_empty() { return !available(head, tail); }

	protected:
		Message*	ring;
		int	capacity;
		std::atomic_uint head, tail;

		bool 	available(unsigned int head, unsigned int tail);
	};


#endif 	// !MessageBuffer_h

