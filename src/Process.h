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


class SimpleProcess : public Process {
	public:
		SimpleProcess()
			: state(InEngine) {}
		bool	is_done();
		void	next();

	protected:
		enum {
			InEngine,
			BackInDAW,
			Done
			};
		int state;

		virtual void	in_engine() = 0;
		virtual void	back_in_daw() = 0;
	};


#endif	// !Process_h

