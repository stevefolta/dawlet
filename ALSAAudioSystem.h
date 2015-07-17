#ifndef ALSAAudioSystem_h
#define ALSAAudioSystem_h

#include "AudioSystem.h"


class ALSAAudioSystem : public AudioSystem {
	public:
		ALSAAudioSystem();
		~ALSAAudioSystem();

		std::vector<std::string>	list_interfaces();
		void	select_interface(std::string name);
		AudioInterface*	selected_interface();

	protected:
	};



#endif	// !ALSAAudioSystem_h

