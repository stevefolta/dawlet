var websocket = null;
var logging_enabled = false;
var track_template = null;

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


function find_element_by_id(element, id) {
	var element_id = element.getAttribute('id');
	if (element_id && element_id == id)
		return element;
	for (var child = element.firstElementChild; child; child = child.nextElementSibling) {
		var result = find_element_by_id(child, id);
		if (result)
			return result;
		}
	return null;
	}

function got_track_template(request) {
	var template_document = request.responseXML;
	track_template = template_document.getElementById("layer1");
	var track_svg = track_template.cloneNode(true);
	var name_element = find_element_by_id(track_svg, "track-name");
	name_element.textContent = "Master";
	var svg = document.getElementById('master');
	svg.appendChild(track_svg);
	svg.style.height = track_svg.getBoundingClientRect().height;
	var gain_knob = new Knob(find_element_by_id(track_svg, 'gain-knob'));
	gain_knob.is_db_knob = true;
	gain_knob.set_db_value(0);
	gain_knob.changed = function(new_db) {
		var gain = dB_to_gain(new_db);
		var request = new XMLHttpRequest();
		request.open("PUT", "/api/track/1/gain", true);
		request.send("" + gain);
		};
	}


function show_entered_value(value) {
	var entered_value_element = document.getElementById("entered-value");
	if (entered_value_element)
		entered_value_element.textContent = value;
	}


function load() {
	log("Loaded page.");

	// Polyfills.
	if (!String.prototype.startsWith) {
		String.prototype.startsWith = function(searchString, position) {
			position = position || 0;
			return this.indexOf(searchString, position) === position;
			};
		}
	Math.log10 = Math.log10 || function(x) {
		return Math.log(x) / Math.LN10;
		};

	// Buttons.
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

	// Start the websocket.
	websocket = new WebSocket("ws://localhost:8080/socket");
	websocket.onmessage = function (event) {
		if (!event.data.startsWith("play-head "))
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

	// Get the track template.
	var request = new XMLHttpRequest();
	request.onreadystatechange = function() {
		var DONE = this.DONE || 4;
		if (this.readyState === DONE)
			got_track_template(this);
		}
	request.open("GET", "track.svg", true);
	request.send(null);
	}

