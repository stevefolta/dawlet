function Track(id, parent) {
	this.id = id;
	this.parent = parent;
	this.children_div = null;
	this.children = [];
	var track = this;

	var indent = 0;
	var level = this.level();
	if (level > 1)
		indent = child_track_indent * (level - 1);

	// Create the elements (<div> & <svg>).
	this.div = document.createElement('div');
	this.div.setAttribute('class', 'track');
	this.track_svg = templates['track'].clone(controls_width - indent, track_height, indent, 0);
	var name_element = find_element_by_id(this.track_svg, "track-name");
	name_element.textContent = "";
		// Until we get the real name.
	name_element.addEventListener('mousedown', function(event) {
		// Use 'mousedown' so we can preventDefault(); "detail >= 2" to detect
		// double-clicks.
		event.preventDefault();
		if (event.detail >= 2)
			change_track_name(track);
		});
	this.svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");;
	this.div.appendChild(this.svg);
	this.svg.appendChild(this.track_svg);
	this.svg.addEventListener('mousedown', function(event) {
		track.track_clicked(event);
		});

	// Add this track's elements into the document in the appropriate place.
	// We have to do this before we start dealing with widths and heights.
	if (this.is_master())
		document.getElementById('master').appendChild(this.div);
	else if (parent.is_master()) {
		document.getElementById('tracks').appendChild(this.div);
		parent.children.push(this);
		}
	else
		parent.append_child(this);
	var total_width = this.div.getBoundingClientRect().width;

	// For some reason, we need to set the SVG's width & height.
	this.svg.style.width = total_width;
	this.svg.style.height = track_height;

	// Set up the gain knob.
	this.gain_knob = new Knob(find_element_by_id(this.track_svg, 'gain-knob'));
	this.gain_knob.is_db_knob = true;
	this.gain_knob.set_db_value(0);
		// Until we get the real value.
	this.gain_knob.changed = function(new_db) {
		var gain = dB_to_gain(new_db);
		var request = new XMLHttpRequest();
		request.open("PUT", "/api/track/" + track.id + "/gain", true);
		request.send("" + gain);
		};

	// Add the lane.
	this.lane = templates['lane'].clone(
		total_width - controls_width, track_height, controls_width);
	this.svg.appendChild(this.lane);

	// Call the API to get our data.
	api_get("/api/track/" + this.id, function(json) { track.got_json(json); });
	api_get(
		"/api/track/" + this.id + "/clips",
		function(json) { track.got_clips_json(json); });
	};


Track.prototype.got_json = function(json) {
	var track = this;

	var name_element = find_element_by_id(this.div, "track-name");
	name_element.textContent = json.name;

	this.gain_knob.set_db_value(gain_to_dB(json.gain));

	// Load up the children.
	json.children.forEach(function(child_id) {
		new Track(child_id, track);
		});
	}


Track.prototype.got_clips_json = function(json) {
	var track = this;
	json.forEach(function(clip) {
		var lanes_rect = track.lane.getBoundingClientRect();
		var svg_rect = track.svg.getBoundingClientRect();
		var clip_svg = templates['clip'].clone(
			clip.length * pixels_per_second, lanes_rect.height,
			lanes_rect.left - svg_rect.left + clip.start * pixels_per_second, 0);
		track.svg.appendChild(clip_svg);
		});
	}

Track.prototype.is_master = function() {
	return this.id == 1;
	};


Track.prototype.append_child = function(child) {
	if (!this.children_div) {
		this.children_div = document.createElement('div');
		this.children_div.setAttribute('class', 'track-children');
		this.div.appendChild(this.children_div);
		}

	this.children_div.appendChild(child.div);
	this.children.push(child);
	};


Track.prototype.level = function() {
	var level = 0;
	for (var parent = this.parent; parent; parent = parent.parent)
		level += 1;
	return level;
	}


function select_track(track) {
	if (selected_track)
		selected_track.track_svg.removeAttribute('selected');
	selected_track = track;
	if (track)
		track.track_svg.setAttribute('selected', 'selected');
	}

Track.prototype.track_clicked = function(event) {
	select_track(this);
	}

function select_next_track() {
	if (!selected_track) {
		if (master_track.children.length > 0)
			select_track(master_track.children[0]);
		else
			select_track(master_track);
		return;
		}

	// If there are children, select the first one.
	if (selected_track.children.length > 1) {
		select_track(selected_track.children[0]);
		return;
		}

	// Select the next sibling.
	var parent = selected_track.parent;
	if (!parent) {
		// The Master track is selected.
		if (selected_track.children.length > 0)
			select_track(selected_track.children[0]);
		return;
		}
	var track = selected_track;
	while (parent) {
		index = parent.children.indexOf(track);
		if (index < parent.children.length - 1) {
			select_track(parent.children[index + 1]);
			return;
			}
		else {
			// "track" is the last track in its parent, so select the parent's
			// next sibling.
			track = parent;
			parent = track.parent;
			}
		}
	}

function select_prev_track() {
	if (!selected_track) {
		if (master_track.children.length > 0)
			select_track(master_track.children[master_track.children.length - 1]);
		else
			select_track(master_track);
		return;
		}

	// Select the previous sibling.
	var track = selected_track;
	var parent = selected_track.parent;
	if (!parent) {
		// This is the master track.
		return;
		}
	var index = parent.children.indexOf(track);
	if (index == 0)
		select_track(parent);
	else
		select_track(parent.children[index - 1]);
	}


function change_track_name(track) {
	if (!track)
		return;

	// Ask for the new name.
	var name_element = find_element_by_id(track.track_svg, "track-name");
	var old_name = name_element.textContent;
	var new_name = window.prompt("New track name:", old_name);
	if (!new_name || new_name == old_name)
		return;

	// Send the API request to set the name.
	var request = new XMLHttpRequest();
	request.onreadystatechange = function() {
		var DONE = this.DONE || 4;
		if (this.readyState === DONE) {
			if (this.status == 200)
				name_element.textContent = new_name;
			}
		}
	request.open("PUT", "/api/track/" + track.id + "/name", true);
	request.send(new_name);
	}


