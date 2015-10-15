#ifndef ALSAAudioSystem_h
#define ALSAAudioSystem_h

#include "AudioSystem.h"
class ALSAAudioInterface;


class ALSAAudioSystem : public AudioSystem {
	public:
		ALSAAudioSystem();
		~ALSAAudioSystem();

		std::vector<std::string>	list_interfaces();
		void	select_interface(std::string name);
		AudioInterface*	selected_interface();

	protected:
		struct Interface {
			std::string name;
			std::string device_name;
			};

		ALSAAudioInterface*	interface;
		std::vector<Interface>	interfaces;

		void	update_interfaces();
	};



#endif	// !ALSAAudioSystem_h

