var key_bindings = {
	"Space": "stop-play",
	"Enter": "pause-play",
	"ArrowLeft": "playhead-left",
	"ArrowRight": "playhead-right",
	"C-S": "save",
	"J": "select-next-track",
	"K": "select-prev-track",
	"N": "change-track-name",
	};

var commands = {
	"stop-play": function() { websocket.send("stop-play"); },
	"pause-play": function() { websocket.send("pause-play"); },
	"playhead-left": function() { websocket.send("seek " + (play_head - prefs.playhead_nudge)); },
	"playhead-right": function() { websocket.send("seek " + (play_head + prefs.playhead_nudge)); },
	"save": function() { websocket.send("save-project"); },
	"select-next-track": function() { select_next_track(); },
	"select-prev-track": function() { select_prev_track(); },
	"change-track-name": function() { change_track_name(selected_track); }
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
	var command = key_bindings[key];
	if (command) {
		handled = true;
		commands[command]();
		}

	if (handled)
		event.preventDefault();
	}
