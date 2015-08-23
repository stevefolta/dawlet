var key_bindings = {
	"Space": "stop-play",
	"Enter": "pause-play",
	"ArrowLeft": "playhead-left",
	"ArrowRight": "playhead-right",
	};

var commands = {
	"stop-play": function() { websocket.send("stop-play"); },
	"pause-play": function() { websocket.send("pause-play"); },
	"playhead-left": function() { websocket.send("seek " + (play_head - prefs.playhead_nudge)); },
	"playhead-right": function() { websocket.send("seek " + (play_head + prefs.playhead_nudge)); },
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
	if (event.controlKey)
		key = "C-" + key;
	var command = key_bindings[key];
	if (command) {
		handled = true;
		commands[command]();
		}

	if (handled)
		event.preventDefault();
	}

