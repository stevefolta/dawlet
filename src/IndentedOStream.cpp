#include "IndentedOStream.h"


void IndentedOStream::write_indent()
{
	const char indents[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	const int available_indents = sizeof(indents) - 1;

	int indents_left = level;
	while (indents_left > 0) {
		int num_indents = indents_left;
		if (num_indents > available_indents)
			num_indents = available_indents;
		stream << indents + (available_indents - num_indents);
		indents_left -= num_indents;
		}
}



