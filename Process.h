#ifndef Process_h
#define Process_h


class Process {
	public:
		virtual ~Process() {}
		virtual bool	is_done() = 0;
		virtual void	next() = 0;
		virtual bool	return_immediately() { return true; }

	protected:
	};


#endif	// !Process_h

