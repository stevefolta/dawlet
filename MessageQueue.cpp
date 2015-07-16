#include "MessageQueue.h"
#include <stdlib.h>

enum {
	defaultCapacity = 100,
	};


MessageQueue::MessageQueue()
{
	capacity = defaultCapacity;
	ring = new Message[capacity];
	head = tail = 0;
}


MessageQueue::~MessageQueue()
{
	delete ring;
}


Message* MessageQueue::back()
{
	if (is_full())
		return nullptr;

	return &ring[tail];
}


void MessageQueue::push()
{
	tail = (tail + 1) % capacity;
}


void MessageQueue::send(int message)
{
	Message* msg = back();
	if (msg) {
		msg->type = message;
		push();
		}
}


void MessageQueue::send(int message, void* param)
{
	Message* msg = back();
	if (msg) {
		msg->type = message;
		msg->param = param;
		push();
		}
}


void MessageQueue::send(int message, int num)
{
	Message* msg = back();
	if (msg) {
		msg->type = message;
		msg->num = num;
		push();
		}
}


Message* MessageQueue::front()
{
	if (is_empty())
		return nullptr;

	return &ring[head];
}


void MessageQueue::pop()
{
	head = (head + 1) % capacity;
}


bool MessageQueue::is_empty()
{
	return tail == head;
}


bool MessageQueue::is_full()
{
	return (tail + 1) % capacity == head;
}



