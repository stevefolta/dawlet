#ifndef MessageQueue_h
#define MessageQueue_h

#include "Message.h"
#include <atomic>

// A ring buffer for sending messages to or from the realtime thread.


class MessageQueue {
	public:
		MessageQueue();
		~MessageQueue();

		// Sending.
		Message*	back();
		void	push();
		void	send(int message); 	// Send a simple message with no parameters.
		void	send(int message, void* param);
		void	send(int message, int num);

		// Receiving.
		Message*	front();
		void	pop();

		bool	is_empty();
		bool	is_full();

	protected:
		Message*	ring;
		int	capacity;
		std::atomic_uint head, tail;
	};


#endif 	// !MessageQueue_h

