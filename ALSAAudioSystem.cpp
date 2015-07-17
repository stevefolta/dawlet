#include "ALSAAudioSystem.h"
#include <alsa/asoundlib.h>


ALSAAudioSystem::ALSAAudioSystem()
{
}


ALSAAudioSystem::~ALSAAudioSystem()
{
}


std::vector<std::string> ALSAAudioSystem::list_interfaces()
{
	std::vector<std::string> result;

	void** hints;
	int err = snd_device_name_hint(-1, "pcm", &hints);
	if (err != 0)
		return result;

	for (void** hint = hints; *hint; ++hint) {
		char* name = snd_device_name_get_hint(*hint, "NAME");
		if (name) {
			if (strcmp(name, "null") != 0)
				result.push_back(name);
			free(name);
			}
		}
	snd_device_name_free_hint(hints);

	return result;
}


void ALSAAudioSystem::select_interface(std::string name)
{
	/***/
}


AudioInterface* ALSAAudioSystem::selected_interface()
{
	/***/
	return nullptr;
}


