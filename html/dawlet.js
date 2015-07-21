var websocket = null;
var logging_enabled = false;

function log(message) {
	if (!logging_enabled)
		return;

	console.log(message);
	}


function populate_interfaces(interfaces) {
	// Clear out existing elements.
	var popup = document.getElementById("interface-popup");
	while (popup.length > 0)
		popup.remove(0);

	// Add the interfaces.
	interfaces.forEach(function(interface) {
		var option = document.createElement('option');
		option.value = interface;
		option.text = interface;
		popup.add(option);
		});
	}


function show_play_head(play_head) {
	var hours = "" + Math.floor(play_head / (60 * 60));
	var minutes = "" + Math.floor(play_head / 60);
	if (minutes.length < 2)
		minutes = "0" + minutes;
	var seconds = "" + (play_head % 60).toFixed(3);
	if (seconds.indexOf(".") < 2)
		seconds = "0" + seconds;
	document.getElementById("play-head").textContent =
		hours + ":" + minutes + ":" + seconds;
	}


function set_button_function(id, fn) {
	var button = document.getElementById(id);
	if (button)
		button.onclick = fn;
	}


function load() {
	log("Loaded page.");

	if (!String.prototype.startsWith) {
		String.prototype.startsWith = function(searchString, position) {
			position = position || 0;
			return this.indexOf(searchString, position) === position;
			};
		}

	set_button_function("play", function() {
		websocket.send("play");
		});
	set_button_function("stop", function() {
		websocket.send("stop");
		});
	set_button_function("pause", function() {
		websocket.send("pause");
		});
	set_button_function("rewind", function() {
		websocket.send("rewind");
		});
	set_button_function("open-project", function() {
		websocket.send("open-project \"test/project.json\"");
		});
	set_button_function("list-interfaces", function() {
		websocket.send("list-interfaces");
		});
	set_button_function("go", function() {
		websocket.send("open-project \"test/project.json\"");
		websocket.send("play");
		});
	document.getElementById("interface-popup").onchange = function(event) {
		websocket.send("select-interface \"" + event.target.value + "\"");
		};
	window.setInterval(function() {
		websocket.send("get-play-head");
		},
		200);

	websocket = new WebSocket("ws://localhost:8080/socket");
	websocket.onmessage = function (event) {
		log("Got websocket message: \"" + event.data + "\"");
		if (event.data.startsWith("interfaces ")) {
			var json = event.data.substr(10);
			var interfaces = JSON.parse(json);
			if (interfaces && interfaces[0]) {
				populate_interfaces(interfaces);
				websocket.send("select-interface \"" + interfaces[0] + "\"");
				}
			}
		else if (event.data.startsWith("play-head ")) {
			var play_head = parseFloat(event.data.substr(10));
			show_play_head(play_head);
			}
		};
	websocket.onopen = function (event) {
		try {
			websocket.send("ping");
			websocket.send("list-interfaces");
			websocket.send("open-project \"test/project.json\"");
			}
		catch (e) {
			log("Websocket send failed!");
			}
		}
	}

