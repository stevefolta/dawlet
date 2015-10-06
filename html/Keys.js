var key_bindings = {
	"Space": "stop-play",
	"Enter": "pause-play",
	"ArrowLeft": "playhead-left",
	"ArrowRight": "playhead-right",
	"C-S": "save",
	"J": "select-next-track",
	"K": "select-prev-track",
	"N": "change-track-name",
	"A": "toggle-record-arm",
	"R": "record",
	"C-Z": "undo",
	"C-S-Z": "redo",
	"S-&": "toggle-stats-visibility",
	};

var commands = {
	"stop-play": function() { websocket.send("stop-play"); },
	"pause-play": function() { websocket.send("pause-play"); },
	"playhead-left": function() { websocket.send("seek " + (play_head - prefs.playhead_nudge)); },
	"playhead-right": function() { websocket.send("seek " + (play_head + prefs.playhead_nudge)); },
	"save": function() { save_project(); },
	"select-next-track": function() { select_next_track(); },
	"select-prev-track": function() { select_prev_track(); },
	"change-track-name": function() { change_track_name(selected_track); },
	"undo": function() { undo_action(); },
	"redo": function() { redo_action(); },
	"toggle-record-arm": function() { if (selected_track) selected_track.toggle_record_arm(); },
	"record": function() { start_recording(); },
	"toggle-stats-visibility": function() { toggle_stats_visibility(); },
	};


function handle_key_down(event) {
	if (event.defaultPrevented)
		return;

	var handled = false;
	var key = event.key;
	if (key.length == 1) {
		key = key.toUpperCase();
		if (key == " ")
			key = "Space";
		}
	if (event.shiftKey)
		key = "S-" + key;
	if (event.metaKey)
		key = "M-" + key;
	if (event.altKey)
		key = "A-" + key;
	if (event.ctrlKey)
		key = "C-" + key;

	if (key == "Escape" && popup_menu_is_up()) {
		close_popup_menu();
		handled = true;
		}

	if (!handled) {
		var command = key_bindings[key];
		if (command) {
			handled = true;
			commands[command]();
			}
		}

	if (handled)
		event.preventDefault();
	}

