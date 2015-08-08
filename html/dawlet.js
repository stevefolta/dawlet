var websocket = null;
var logging_enabled = false;
var templates = {};

var template_names = [ 'track' ];

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

function api_get(url, when_done) {
	var request = new XMLHttpRequest();
	request.onreadystatechange = function() {
		var DONE = this.DONE || 4;
		if (this.readyState === DONE)
			when_done(JSON.parse(this.responseText));
		}
	request.open("GET", url, true);
	request.send(null);
	}

function project_loaded() {
	api_get("/api/project", got_project_json);
	}

function got_project_json(json) {
	// Clear out any existing elements.
	var master = document.getElementById('master');
	while (master.hasChildNodes())
		master.removeChild(master.lastChild);
	var tracks = document.getElementById('tracks');
	while (tracks.hasChildNodes())
		tracks.removeChild(tracks.lastChild);

	// Get the master track; the rest of the tracks will follow from there.
	new Track(json.master);
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

	// Get the templates.
	template_names.forEach(function(name) {
		var request = new XMLHttpRequest;
		request.onreadystatechange = function() {
			var DONE = this.DONE || 4;
			if (this.readyState === DONE) {
				var template_document = request.responseXML;
				templates[name] = template_document.getElementById("layer1");
				}
			};
		request.open('GET', name + '.svg', true);
		request.send(null);
		});

	// Start the websocket.
	// Unfortunately, we can't give a relative URL.
	websocket = new WebSocket("ws://" + window.location.host + "/socket");
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
		else if (event.data == "project-loaded")
			project_loaded();
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

