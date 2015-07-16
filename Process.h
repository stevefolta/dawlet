#ifndef Process_h
#define Process_h


class Process {
	public:
		virtual bool	is_done() = 0;
		virtual void	next() = 0;

	protected:
	};


#endif	// !Process_h

