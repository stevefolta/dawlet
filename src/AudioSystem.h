#ifndef AudioSystem_h
#define AudioSystem_h

#include <string>
#include <vector>
class AudioInterface;


class AudioSystem {
	public:
		virtual std::vector<std::string>	list_interfaces() = 0;
		virtual void	select_interface(std::string name) = 0;
		virtual AudioInterface*	selected_interface() = 0;

	protected:
	};

extern AudioSystem* audio_system;


#endif	// !AudioSystem_h

