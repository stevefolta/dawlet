#ifndef InstallProjectProcess_h
#define InstallProjectProcess_h

#include "Process.h"
class Project;


class InstallProjectProcess : public Process {
	public:
		InstallProjectProcess(Project* project_in)
			: project(project_in), state(Installing) {}
		~InstallProjectProcess();

		bool	is_done();
		void	next();

	protected:
		enum {
			Installing,
			Reporting,
			Done,
			};

		Project*	project;
		int	state;
	};


#endif	// !InstallProjectProcess_h

