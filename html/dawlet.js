var websocket = null;
var logging_enabled = false;
var templates = {};
var controls_width = 500;
var track_height = 60;
var child_track_indent = 20;
var pixels_per_second = 5;
var play_head = 0;
var master_track = null;
var tracks_by_id = {};
var selected_track = null;
var theme_css_link = null;

var prefs = {
	playhead_nudge: 0.1,
	};

var template_names = [ 'track', 'lane', 'clip' ];

function log(message) {
	if (!logging_enabled)
		return;

	console.log(message);
	}

function set_visible(id, visible) {
	var element = document.getElementById(id);
	if (!element)
		return;
	if (visible)
		element.removeAttribute('hidden');
	else
		element.setAttribute('hidden', 'hidden');
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

function update_favorite_interfaces(new_interface) {
	var fav_interfaces = [];
	var json = localStorage.getItem('fav-interfaces');
	if (json)
		fav_interfaces = JSON.parse(json);
	var index = fav_interfaces.indexOf(new_interface);
	if (index == 0) {
		// It's already the first item.  LRU is unchanged.
		}
	else {
		if (index > 0) {
			// Remove it from its current position.
			fav_interfaces.splice(index, 1);
			}
		fav_interfaces.unshift(new_interface);
		localStorage.setItem('fav-interfaces', JSON.stringify(fav_interfaces));
		}
	}

function select_initial_interface(interfaces) {
	// Get the favorite interfaces.
	var fav_interfaces = [];
	var json = localStorage.getItem('fav-interfaces');
	if (json)
		fav_interfaces = JSON.parse(json);

	// Find the first one that's currently available.
	var use_interface_index = 0;
	var done = false;
	for (var i = 0; !done && i < fav_interfaces.length; ++i) {
		var want_interface = fav_interfaces[i];
		for (var which_interface = 0; which_interface < interfaces.length; ++which_interface) {
			if (interfaces[which_interface] == want_interface) {
				use_interface_index = which_interface;
				done = true;
				}
			}
		}

	websocket.send("select-interface \"" + interfaces[use_interface_index] + "\"");
	var popup = document.getElementById("interface-popup");
	if (popup)
		popup.selectedIndex = use_interface_index;
	}


function show_play_head(play_head) {
	// Clock.
	var hours = "" + Math.floor(play_head / (60 * 60));
	var minutes = "" + Math.floor(play_head / 60);
	if (minutes.length < 2)
		minutes = "0" + minutes;
	var seconds = "" + (play_head % 60).toFixed(3);
	if (seconds.indexOf(".") < 2)
		seconds = "0" + seconds;
	document.getElementById("clock").textContent =
		hours + ":" + minutes + ":" + seconds;

	// Play head.
	var play_head_x = controls_width + play_head * pixels_per_second;
	document.getElementById("play-head").setAttribute(
		'transform', "translate(" + play_head_x + ", 0)");
	}


function update_meters(message) {
	var data = message.split(' ');
	data.shift();
	while (data.length > 0) {
		var track_id = parseInt(data.shift());
		var peak = parseFloat(data.shift());
		var track = tracks_by_id[track_id];
		if (track)
			track.update_meter(peak);
		}
	}


var xrun_timeout = null;
var xrun_display_ms = 300;
function got_xrun() {
	if (xrun_timeout)
		window.clearTimeout(xrun_timeout);

	var clock = document.getElementById("clock");
	if (clock) {
		clock.setAttribute("xrun", "xrun");

		xrun_timeout = window.setTimeout(function() {
			clock.removeAttribute("xrun");
			xrun_timeout = null;
			},
			xrun_display_ms);
		}
	}

function got_error(error) {
	var error_div = document.getElementById("error");
	if (error_div) {
		error_div.textContent = error;
		error_div.style.display = 'block';
		}
	}

function hide_error() {
	var error_div = document.getElementById("error");
	if (error_div)
		error_div.style.display = 'none';
	}


var stats_timer = null;

function show_stats() {
	if (stats_timer)
		return;

	var stats_div = document.getElementById("stats");
	if (stats_div) {
		stats_div.style.display = 'block';
		update_stats();
		stats_timer = window.setInterval(update_stats, 1000);
		}
	}

function hide_stats() {
	var stats_div = document.getElementById("stats");
	stats_div.style.display = 'none';
	if (stats_timer) {
		window.clearInterval(stats_timer);
		stats_timer = null;
		}
	}

function toggle_stats_visibility() {
	if (stats_timer)
		hide_stats();
	else
		show_stats();
	}

function update_stats() {
	api_get("/api/stats", function(stats) {
		var stats_div = document.getElementById("stats");
		if (stats_div) {
			var stats_str = "";
			var started = false;
			var stat_names = [ "playback_xruns", "capture_xruns", "missing_file_reads", "read_slot_overflows", "exhausted_reads" ];
			stat_names.forEach(function(stat_name) {
				if (started)
					stats_str += "  ";
				else
					started = true;
				stats_str += stat_name + ": " + stats[stat_name];
				});
			stats_div.textContent = stats_str;
			}
		});
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
	master_track = new Track(json.master);
	}

function save_project() {
	if (last_action)
		last_action.fix();
	websocket.send("save-project");
	}


function show_entered_value(value) {
	var entered_value_element = document.getElementById("entered-value");
	if (entered_value_element)
		entered_value_element.textContent = value;
	}


function load_theme() {
	// Get the templates.
	templates = {};
	template_names.forEach(function(name) {
		var request = new XMLHttpRequest;
		request.onreadystatechange = function() {
			var DONE = this.DONE || 4;
			if (this.readyState === DONE) {
				var template_document = request.responseXML;
				templates[name] = new SVGTemplate(template_document);
				}
			};
		request.open('GET', 'theme/' + name + '.svg', true);
		request.send(null);
		});

	// Insert the CSS.
	var head = document.getElementsByTagName('head')[0];
	if (theme_css_link)
		head.removeChild(theme_css_link);
	theme_css_link = document.createElement('link');
	theme_css_link.setAttribute('href', 'theme/theme.css');
	theme_css_link.setAttribute('rel', 'stylesheet');
	theme_css_link.setAttribute('type', 'text/css');
	head.appendChild(theme_css_link);
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

	// Keys.
	window.addEventListener("keydown", handle_key_down);

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
		var new_interface = event.target.value;
		websocket.send("select-interface \"" + new_interface + "\"");
		update_favorite_interfaces(new_interface);
		hide_error();
		};
	window.setInterval(function() {
		websocket.send("get-play-head");
		},
		200);

	var error_div = document.getElementById("error");
	if (error_div)
		error_div.onclick = hide_error;

	load_theme();

	// Start the websocket.
	// Unfortunately, we can't give a relative URL.
	websocket = new WebSocket("ws://" + window.location.host + "/socket");
	websocket.onmessage = function (event) {
		if (!event.data.startsWith("play-head ") && !event.data.startsWith("meters"))
			log("Got websocket message: \"" + event.data + "\"");
		if (event.data.startsWith("interfaces ")) {
			var json = event.data.substr(10);
			var interfaces = JSON.parse(json);
			if (interfaces && interfaces[0]) {
				populate_interfaces(interfaces);
				select_initial_interface(interfaces);
				}
			}
		else if (event.data.startsWith("play-head ")) {
			play_head = parseFloat(event.data.substr(10));
			show_play_head(play_head);
			}
		else if (event.data.startsWith("meters "))
			update_meters(event.data);
		else if (event.data.startsWith("dirty ")) {
			var json = event.data.substr("dirty ".length);
			var value = JSON.parse(json);
			set_visible('dirty', value);
			}
		else if (event.data == "project-loaded")
			project_loaded();
		else if (event.data == "xrun")
			got_xrun();
		else if (event.data.startsWith("error "))
			got_error(event.data.substr("error ".length));
		};
	websocket.onopen = function (event) {
		try {
			websocket.send("ping");
			websocket.send("list-interfaces");
			websocket.send("open-project \"test/project.json\"");
			document.getElementById('project-title').textContent = "Test Project";
			}
		catch (e) {
			log("Websocket send failed!");
			}
		}
	}

