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
var interface_inputs = [];
var project_is_open = false;
var in_open_project_screen = false;
var recording_clips = [];
var recording_start_time = 0.0;

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

function update_inputs() {
	interface_inputs = [];
	api_get("/api/inputs", function(inputs) {
		interface_inputs = inputs;
		});
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


function start_recording() {
	do_action(new RecordAction());
	//***
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
			var stat_names = [
				"playback_xruns", "capture_xruns", "missing_file_reads", "read_slot_overflows",
				"exhausted_buffers", "exhausted_reads", "exhausted_record_buffers"
				];
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

function playing_started() {
	var play_button = document.getElementById("play");
	if (play_button)
		play_button.setAttribute("playing", "playing");
	}
function playing_stopped() {
	var play_button = document.getElementById("play");
	if (play_button)
		play_button.removeAttribute("playing");
	}
function recording_started(start_time, values) {
	var record_button = document.getElementById("record");
	if (record_button)
		record_button.setAttribute("recording", "recording");

	// Add all the new clips.
	recording_clips = [];
	recording_start_time = start_time;
	values.forEach(function(spec) {
		if (spec.length == 0)
			return;
		let [track_id, clip_id] = spec.split(':').map(id => parseInt(id));
		if (track_id == 0 || clip_id == 0)
			return;
		let track = tracks_by_id[track_id];
		if (!track)
			return;
		recording_clips.push(track.add_clip_at(clip_id, start_time));
		});
	}
function recording_stopped() {
	var record_button = document.getElementById("record");
	if (record_button)
		record_button.removeAttribute("recording");
	recording_clips = [];
	}
function update_during_recording() {
	if (recording_clips.length > 0) {
		let new_length = (play_head - recording_start_time) * pixels_per_second;
		recording_clips.forEach(clip => clip.length_changed_to(new_length));
		}
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

function api_post(url, when_done) {
	var request = new XMLHttpRequest();
	request.onreadystatechange = function() {
		var DONE = this.DONE || 4;
		if (this.readyState === DONE && this.status == 200)
			when_done(JSON.parse(this.responseText));
		}
	request.open("POST", url, true);
	request.send(null);
	}

function api_delete(url, when_done) {
	var request = new XMLHttpRequest();
	request.onreadystatechange = function() {
		var DONE = this.DONE || 4;
		if (this.readyState === DONE && this.status == 200)
			when_done();
		}
	request.open("DELETE", url, true);
	request.send(null);
	}

function project_loaded() {
	api_get("/api/project", got_project_json);
	hide_error();
	}

function got_project_json(json) {
	project_is_open = true;

	// Clear out and reset the project UI.
	clear_project_ui();
	last_action = new SentinalAction();
	set_visible('dirty', false);

	// Need to make the project screen visible before adding the master track,
	// otherwise it gets a bad width and doesn't display.
	set_visible('whole-project', true);
	set_visible('projects-screen', false);
	set_visible('server-shut-down', false);
	in_open_project_screen = false;

	// Get the master track; the rest of the tracks will follow from there.
	master_track = new Track(json.master);
	}

function clear_project_ui() {
	var master = document.getElementById('master');
	while (master.hasChildNodes())
		master.removeChild(master.lastChild);
	var tracks = document.getElementById('tracks');
	while (tracks.hasChildNodes())
		tracks.removeChild(tracks.lastChild);
	}

function project_closed() {
	project_is_open = false;
	clear_project_ui();
	document.cookie = "last_project_path=; max-age=31536000";
	}

function save_project() {
	if (last_action)
		last_action.fix();
	websocket.send("save-project");
	}

function title_for_path(project_path) {
	var title = project_path;
	var slash_pos = project_path.indexOf("/");
	if (slash_pos >= 0) {
		title = project_path.substr(0, slash_pos);
		var filename = project_path.substr(slash_pos + 1);
		if (filename != "project.json")
			title = filename;
		}
	return title;
	}

function update_project_title(project_path) {
	// Update the project title.
	var project_title_element = document.getElementById('project-title');
	if (project_title_element)
		project_title_element.textContent = title_for_path(project_path);
	}


function open_project(path) {
	websocket.send("open-project \"" + path + "\"");
	update_project_title(path);
	document.cookie = "last_project_path=" + path + "; max-age=31536000";
	}

function attempt_new_project() {
	var name = window.prompt("New project name:", "");
	if (name) {
		websocket.send("new-project \"" + name + "\"");
		update_project_title(name);
		document.cookie = "last_project_path=" + name + "/project.json; max-age=31536000";
		}
	}


function show_open_project_screen() {
	set_visible('whole-project', false);
	set_visible('projects-screen', true);
	in_open_project_screen = true;

	// Clear out anything that's there.
	var projects_div = document.getElementById('projects');
	while (projects_div.hasChildNodes())
		projects_div.removeChild(projects_div.lastChild);

	api_get("/api/projects", function(projects) {
		projects.forEach(function(path) {
			var element = document.createElement('a');
			element.setAttribute('href', "#");
			element.setAttribute('class', 'project');
			element.textContent = title_for_path(path);
			element.path = path;
			element.onclick = function() {
				open_project(path);
				};
			projects_div.appendChild(element);
			});
		});

	let new_project_button = document.getElementById('new-project');
	if (new_project_button) {
		new_project_button.onclick = function() {
			attempt_new_project();
			};
		}

	set_visible('cancel-open-project-div', project_is_open);
	if (project_is_open) {
		var cancel_button = document.getElementById('cancel-open-project');
		if (cancel_button)
			cancel_button.onclick = function() {
				set_visible('projects-screen', false);
				set_visible('whole-project', true);
				in_open_project_screen = false;
				};
		}
	}

function close_project() {
	websocket.send("close-project");
	project_closed();
	show_open_project_screen();
	}

function server_shut_down() {
	set_visible('whole-project', false);
	set_visible('server-shut-down', true);
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


function initial_screen() {
	let opened = false;
	let last_project_path_cookie =
		document.cookie
		.split('; ')
		.find(row => row.startsWith('last_project_path'));
	if (last_project_path_cookie) {
		let path = last_project_path_cookie.split('=')[1];
		if (path) {
			open_project(path);
			opened = true;
			}
		}
	if (!opened)
		show_open_project_screen();
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
	if (!String.prototype.endsWith) {
		String.prototype.endsWith = function(searchString, position) {
			var subjectString = this.toString();
			if (typeof position !== 'number' || !isFinite(position) || Math.floor(position) !== position || position > subjectString.length)
				position = subjectString.length;
			position -= searchString.length;
			var lastIndex = subjectString.indexOf(searchString, position);
			return lastIndex !== -1 && lastIndex === position;
			};
		}
	Math.log10 = Math.log10 || function(x) {
		return Math.log(x) / Math.LN10;
		};

	// Keys.
	window.addEventListener("keydown", handle_key_down);

	// Menus.
	setup_menus();

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
	set_button_function("record", start_recording);
	set_button_function("open-project", function() {
		open_project("test/project.json");
		});
	set_button_function("list-interfaces", function() {
		websocket.send("list-interfaces");
		});
	set_button_function("go", function() {
		open_project("test/project.json");
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
			update_during_recording();
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
		else if (event.data == "playing")
			playing_started();
		else if (event.data == "stopped") {
			playing_stopped();
			recording_stopped();
			}
		else if (event.data.startsWith("recording-started ")) {
			var values = event.data.substr("recording-started ".length).trim().split(' ');
			var start_time = parseFloat(values.shift());
			recording_started(start_time, values);
			}
		else if (event.data == "recording-stopped")
			recording_stopped();
		else if (event.data == "xrun")
			got_xrun();
		else if (event.data.startsWith("interface-selected "))
			update_inputs();
		else if (event.data.startsWith("error "))
			got_error(event.data.substr("error ".length));
		};
	websocket.onopen = function (event) {
		try {
			websocket.send("ping");
			websocket.send("list-interfaces");
			initial_screen();
			}
		catch (e) {
			log("Websocket send failed!");
			}
		}
	}

