#include "GetStatsProcess.h"
#include "AudioEngine.h"
#include "web/Connection.h"
#include "Logger.h"
#include <sstream>


void GetStatsProcess::in_engine()
{
	stats = engine->get_stats();
	state = Replying;
}


void GetStatsProcess::replying()
{
	std::stringstream json;
	json << "{ ";
	json << "\"playback_xruns\": " << stats.playback_xruns << ", ";
	json << "\"capture_xruns\": " << stats.capture_xruns << ", ";
	json << "\"missing_file_reads\": " << stats.missing_file_reads << ", ";
	json << "\"read_slot_overflows\": " << stats.read_slot_overflows << ", ";
	json << "\"exhausted_buffers\": " << stats.exhausted_buffers << ", ";
	json << "\"exhausted_reads\": " << stats.exhausted_reads << ", ";
	json << "\"exhausted_record_buffers\": " << stats.exhausted_record_buffers << ", ";
	json << "\"ignore_this\": false ";
		// Just so we can easily add new fields without worrying about the
		// trailing comma.
	json << "}";

	connection->send_json_reply(json.str());
	state = Done;
}



