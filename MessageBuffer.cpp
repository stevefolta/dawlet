#include "MessageBuffer.h"
#include <stdlib.h>

enum {
	defaultCapacity = 100,
	};


MessageBuffer::MessageBuffer()
{
	capacity = defaultCapacity;
	ring = new Message[capacity];
}


MessageBuffer::~MessageBuffer()
{
	delete ring;
}


Message* MessageBuffer::back()
{
	if (!available(head, tail))
		return NULL;

	return &ring[head % capacity];
}


void MessageBuffer::push()
{
	head += 1;
}


Message* MessageBuffer::front()
{
	if (tail >= head)
		return NULL;

	return &ring[tail % capacity];
}


void MessageBuffer::pop()
{
	tail += 1;
}


bool MessageBuffer::available(unsigned int head, unsigned int tail)
{
	if (head == tail)
		return true;
	else if (tail > head)
		return (tail - head) > capacity;
	else
		return tail + capacity - head > 0;
}



