#include "ALSAAudioSystem.h"
#include "ALSAAudioInterface.h"
#include "AudioEngine.h"
#include "Exception.h"
#include "Logger.h"
#include <alsa/asoundlib.h>
#include <sstream>


ALSAAudioSystem::ALSAAudioSystem()
	: interface(nullptr)
{
	update_interfaces();
}


ALSAAudioSystem::~ALSAAudioSystem()
{
	delete interface;
}


std::vector<std::string> ALSAAudioSystem::list_interfaces()
{
	std::vector<std::string> result;

	for (auto interface: interfaces)
		result.push_back(interface.name);

	return result;
}


void ALSAAudioSystem::select_interface(std::string name)
{
	// Get the device name.
	std::string device_name;
	for (auto interface_spec: interfaces) {
		if (interface_spec.name == name) {
			device_name = interface_spec.device_name;
			break;
			}
		}
	if (device_name == "") {
		log("Unknown device: %s", name.c_str());
		return;
		}

	try {
		delete interface;
		interface = nullptr;
		interface = new ALSAAudioInterface(device_name);
		interface->setup(2, engine->sample_rate(), engine->buffer_size());
		}
	catch (Exception& e) {
		engine->report_error(e.type);
		delete interface;
		interface = nullptr;
		}
}


AudioInterface* ALSAAudioSystem::selected_interface()
{
	return interface;
}


void ALSAAudioSystem::update_interfaces()
{
	interfaces.clear();

	snd_ctl_card_info_t* info;
	snd_ctl_card_info_alloca(&info);
	snd_pcm_info_t* pcm_info;
	snd_pcm_info_alloca(&pcm_info);

	int card = -1;
	while (snd_card_next(&card) >= 0 && card >= 0) {
		std::ostringstream device_name_builder;
		device_name_builder << "hw:" << card;
		std::string device_name = device_name_builder.str();

		snd_ctl_t* handle;
		if (snd_ctl_open(&handle, device_name.c_str(), 0) < 0)
			continue;
		if (snd_ctl_card_info(handle, info) >= 0) {
			std::string card_name = snd_ctl_card_info_get_name(info);

			// Change the device_name to use the card ID, not its number.
			device_name_builder.str("");
			device_name_builder << "default:" << snd_ctl_card_info_get_id(info);
			// Use this if/when we switch to MMAP access:
			// device_name_builder << "hw:" << snd_ctl_card_info_get_id(info);
			device_name = device_name_builder.str();

			int device = -1;
			while (snd_ctl_pcm_next_device(handle, &device) >= 0 && device >= 0) {
				// Only detect duplex devices.
				snd_pcm_info_set_device(pcm_info, device);
				snd_pcm_info_set_subdevice(pcm_info, 0);
				snd_pcm_info_set_stream(pcm_info, SND_PCM_STREAM_CAPTURE);
				if (snd_ctl_pcm_info(handle, pcm_info) < 0)
					continue;
				snd_pcm_info_set_device(pcm_info, device);
				snd_pcm_info_set_subdevice(pcm_info, 0);
				snd_pcm_info_set_stream(pcm_info, SND_PCM_STREAM_PLAYBACK);
				if (snd_ctl_pcm_info(handle, pcm_info) < 0)
					continue;
				device_name_builder.str("");
				device_name_builder << device_name;
				// Use this if/when we switch to MMAP access:
				// device_name_builder << device_name << ',' << device;
				Interface interface = { card_name, device_name_builder.str() };
				interfaces.push_back(interface);
				}
			}
		snd_ctl_close(handle);
		}
}



