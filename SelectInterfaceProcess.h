#ifndef SelectInterfaceProcess_h
#define SelectInterfaceProcess_h

#include "Process.h"
#include <string>


class SelectInterfaceProcess : public Process {
	public:
		SelectInterfaceProcess(std::string name_in)
			: name(name_in), state(Selecting) {}

		bool	is_done();
		void	next();

	protected:
		enum {
			Selecting,
			Done,
			};

		std::string	name;
		int state;
	};



#endif	// !SelectInterfaceProcess_h

