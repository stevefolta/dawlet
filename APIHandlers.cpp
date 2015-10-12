#include "APIHandlers.h"
#include "web/Connection.h"
#include "Project.h"
#include "Track.h"
#include "DAW.h"
#include "Recorder.h"
#include "AudioEngine.h"
#include "AudioSystem.h"
#include "AudioInterface.h"
#include "SetTrackStateProcesses.h"
#include "NewTrackProcess.h"
#include "DeleteTrackProcess.h"
#include "RestoreTrackProcess.h"
#include "GetStatsProcess.h"
#include "Logger.h"
#include <map>
#include <string.h>


std::string APIHandler_project::json_value()
{
	Project* project = daw->cur_project();
	if (project)
		return project->api_json();
	else
		return "null";
}


void APIHandler_track::handle(std::string url_remainder, Web::Connection* connection)
{
	// Get the track.
	std::string id_str = pop_url_front(&url_remainder);
	if (id_str.empty()) {
		connection->error_out("404 Not Found");
		return;
		}
	int id = strtol(id_str.c_str(), nullptr, 0);
	Track* track = nullptr;
	Project* project = daw->cur_project();
	if (id > 0 && project)
		track = project->track_by_id(id);
	if (track == nullptr) {
		connection->error_out("404 Not Found");
		return;
		}

	if (url_remainder.empty())
		send_json_reply(connection, track->api_json());
	else if (url_remainder == "clips")
		send_json_reply(connection, track->clips_json());
	else {
		connection->error_out("404 Not Found");
		}
}


bool APIHandler_track::can_put()
{
	return true;
}


void APIHandler_track::handle_put(std::string url_remainder, std::string value, Web::Connection* connection)
{
	// Get the track.
	std::string id_str = pop_url_front(&url_remainder);
	if (id_str.empty()) {
		connection->error_out("404 Not Found");
		return;
		}
	int id = strtol(id_str.c_str(), nullptr, 0);
	Track* track = nullptr;
	Project* project = daw->cur_project();
	if (id > 0 && project)
		track = project->track_by_id(id);
	if (track == nullptr) {
		connection->error_out("404 Not Found");
		return;
		}

	if (url_remainder.empty())
		connection->error_out("405 Method Not Allowed");
	else {
		std::string component = pop_url_front(&url_remainder);
		if (!url_remainder.empty()) {
			connection->error_out("404 Not Found");
			return;
			}
		if (component == "gain") {
			char* end_ptr;
			float gain = strtof(value.c_str(), &end_ptr);
			if (end_ptr == value.c_str())
				connection->error_out("400 Bad Request");
			else
				engine->start_process(new SetTrackGainProcess(track, gain, connection));
			}
		else if (component == "name")
			engine->start_process(new SetTrackNameProcess(track, value, connection));
		else if (component == "record-arm") {
			bool armed = false, ok = true;
			if (value == "true")
				armed = true;
			else if (value != "false") {
				connection->error_out("400 Bad Request");
				ok = false;
				}
			if (ok) {
				if (armed)
					daw->get_recorder()->arm_track(track, connection);
				else
					daw->get_recorder()->unarm_track(track, connection);
				}
			}
		else if (component == "input")
			daw->get_recorder()->set_track_input(track, value, connection);
		else if (component == "monitor-input") {
			bool monitor = false, ok = true;
			if (value == "true")
				monitor = true;
			else if (value != "false") {
				connection->error_out("400 Bad Request");
				ok = false;
				}
			if (ok) {
				engine->start_process(
					new SetTrackMonitorInputProcess(track, monitor, connection));
				}
			}
		else
			connection->error_out("404 Not Found");
		}
}


bool APIHandler_track::can_post()
{
	return true;
}


void APIHandler_track::handle_post(
	std::string url_remainder, Web::Connection* connection)
{
	Project* project = daw->cur_project();
	if (project == nullptr) {
		connection->error_out("404 Not Found");
		return;
		}

	// Handle "?" parameters.
	Track* after_track = nullptr;
	Track* before_track = nullptr;
	Track* restore_track = nullptr;
	Track* parent_track = nullptr;
	if (!url_remainder.empty()) {
		if (url_remainder.front() != '?') {
			connection->error_out("400 Bad Request");
			return;
			}
		url_remainder.erase(0, 1);
		while (!url_remainder.empty()) {
			size_t separator_index = url_remainder.find('&');
			if (separator_index == std::string::npos)
				separator_index = url_remainder.find(';');
			std::string query;
			if (separator_index == std::string::npos) {
				query = url_remainder;
				url_remainder = "";
				}
			else {
				query = url_remainder.substr(0, separator_index);
				url_remainder.erase(0, separator_index + 1);
				}
			auto get_track = [&query, project]() -> Track* {
				int id = strtol(query.c_str(), nullptr, 0);
				if (id > 0)
					return project->track_by_id(id);
				return nullptr;
				};
			if (query.find("after=") == 0) {
				query.erase(0, strlen("after="));
				after_track = get_track();
				if (after_track == nullptr) {
					connection->error_out("404 Not Found");
					return;
					}
				}
			else if (query.find("before=") == 0) {
				query.erase(0, strlen("before="));
				before_track = get_track();
				if (before_track == nullptr) {
					connection->error_out("404 Not Found");
					return;
					}
				}
			else if (query.find("restore=") == 0) {
				query.erase(0, strlen("restore="));
				int id = strtol(query.c_str(), nullptr, 0);
				if (id > 0)
					restore_track = project->deleted_track_by_id(id);
				if (restore_track == nullptr) {
					connection->error_out("404 Not Found");
					return;
					}
				}
			else if (query.find("parent=") == 0) {
				query.erase(0, strlen("parent="));
				parent_track = get_track();
				if (parent_track == nullptr) {
					connection->error_out("404 Not Found");
					return;
					}
				}
			else {
				connection->error_out("400 Bad Request");
				return;
				}
			}
		}

	if (before_track && after_track) {
		connection->error_out("400 Bad Request");
		return;
		}
	if (restore_track) {
		if (parent_track == nullptr) {
			connection->error_out("400 Bad Request");
			return;
			}
		if (after_track)
			before_track = parent_track->child_after(after_track);
		engine->start_process(
			new RestoreTrackProcess(
				restore_track, parent_track, before_track, connection));
		}
	else {
		if (before_track || parent_track) {
			connection->error_out("501 Not Implemented");
			return;
			}
		engine->start_process(new NewTrackProcess(after_track, connection));
		}
}


bool APIHandler_track::can_delete()
{
	return true;
}


void APIHandler_track::handle_delete(std::string url_remainder, Web::Connection* connection)
{
	// Get the track.
	std::string id_str = pop_url_front(&url_remainder);
	if (id_str.empty()) {
		connection->error_out("404 Not Found");
		return;
		}
	int id = strtol(id_str.c_str(), nullptr, 0);
	Track* track = nullptr;
	Project* project = daw->cur_project();
	if (id > 0 && project)
		track = project->track_by_id(id);
	if (track == nullptr) {
		connection->error_out("404 Not Found");
		return;
		}

	engine->start_process(new DeleteTrackProcess(track, connection));
}




void APIHandler_stats::handle(std::string url_remainder, Web::Connection* connection)
{
	engine->start_process(new GetStatsProcess(connection));
}


void APIHandler_inputs::handle(std::string url_remainder, Web::Connection* connection)
{
	if (!url_remainder.empty()) {
		connection->error_out("404 Not Found");
		return;
		}

	AudioInterface* interface = audio_system->selected_interface();
	if (interface == nullptr)
		connection->error_out("404 Not Found");
	else
		send_json_reply(connection, interface->input_names_json());
}



#undef Handler
#define Handler(name) 	\
	APIHandler_##name	name##_handler;
#include "TopAPIHandlers.h"

struct HandlerAssoc {
	const char*	name;
	APIHandler*	handler;
	};
static HandlerAssoc top_level_handlers[] = {
	#undef Handler
	#define Stringify1(s)	#s
	#define Stringify(s)	Stringify1(s)
	#define Handler(name) 	\
		{ Stringify(name), &name##_handler },
	#include "TopAPIHandlers.h"
	{ nullptr, nullptr }
	};
static std::map<std::string, APIHandler*> handlers_map;
bool handlers_map_initialized = false;

static void initialize_handlers_map()
{
	if (handlers_map_initialized)
		return;

	HandlerAssoc* assoc = top_level_handlers;
	for (; assoc->name; ++assoc)
		handlers_map[assoc->name] = assoc->handler;
	handlers_map_initialized = true;
}

void dispatch_top_level_api(std::string url_remainder, Web::Connection* connection)
{
	initialize_handlers_map();

	std::string api = pop_url_front(&url_remainder);
	APIHandler* handler = handlers_map[api];
	if (handler)
		handler->handle(url_remainder, connection);
	else
		connection->error_out("404 Not Found");
}


void dispatch_top_level_api_put(
	std::string url_remainder, std::string content, Web::Connection* connection)
{
	initialize_handlers_map();

	std::string api = pop_url_front(&url_remainder);
	APIHandler* handler = handlers_map[api];
	if (handler) {
		if (handler->can_put())
			handler->handle_put(url_remainder, content, connection);
		else
			connection->error_out("405 Method Not Allowed");
		}
	else
		connection->error_out("404 Not Found");
}


void	dispatch_top_level_api_post(
	std::string url_remainder, Web::Connection* connection)
{
	initialize_handlers_map();

	std::string api = pop_url_front(&url_remainder);
	APIHandler* handler = handlers_map[api];
	if (handler) {
		if (handler->can_post())
			handler->handle_post(url_remainder, connection);
		else
			connection->error_out("405 Method Not Allowed");
		}
	else
		connection->error_out("404 Not Found");
}


void	dispatch_top_level_api_delete(
	std::string url_remainder, Web::Connection* connection)
{
	initialize_handlers_map();

	std::string api = pop_url_front(&url_remainder);
	APIHandler* handler = handlers_map[api];
	if (handler) {
		if (handler->can_delete())
			handler->handle_delete(url_remainder, connection);
		else
			connection->error_out("405 Method Not Allowed");
		}
	else
		connection->error_out("404 Not Found");
}



