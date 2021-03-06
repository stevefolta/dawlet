function Track(id, parent, json, before_track) {
	this.id = id;
	this.parent = parent;
	this.children_div = null;
	this.children = [];
	this.meter_rect = null;
	var track = this;
	tracks_by_id[id] = this;

	this.gain = 1;
	this.record_armed = false;
	this.monitor_input = true;
	this.input = null;

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
	else if (before_track)
		parent.insert_child_before(this, before_track);
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
	this.set_gain(0);
		// Until we get the real value.
	this.gain_knob.changed = function(new_db) {
		var gain = dB_to_gain(new_db);
		do_action(new ChangeTrackGainAction(track, gain));
		};
	this.gain_knob.mouse_up = function(new_db) {
		if (last_action.type == 'change-track-gain' && last_action.track ==  track)
			last_action.fix();
		};

	this.meter_track = find_element_by_id(this.track_svg, 'meter-track');

	// Set up the record-arm button.
	var rec_arm = find_element_by_id(this.track_svg, 'record-arm');
	function pop_up_rec_menu() {
		var popup = new PopupMenu(true);
		popup.add_item(
			"Monitor Input",
			function() {
				do_action(
					new ChangeTrackMonitorInputAction(
						track, !track.monitor_input));
				},
			track.monitor_input);
		popup.add_divider();
		interface_inputs.forEach(function(category) {
			var category_menu = new PopupMenu(true);
			category.inputs.forEach(function(name) {
				category_menu.add_item(name, function(name) {
					do_action(new ChangeTrackInputAction(track, name));
					},
					name == track.input);
				});
			popup.add_submenu(category.category, category_menu);
			});
		var rec_arm_rect = rec_arm.getBoundingClientRect();
		popup.open(rec_arm_rect.right, rec_arm_rect.top);
		}
	rec_arm.addEventListener('mousedown', function(event) {
		if (event.button == 2) {
			// The context menu is coming up, ignore this.
			}
		else if (event.ctrlKey)
			pop_up_rec_menu();
		else
			track.toggle_record_arm();
		});
	rec_arm.addEventListener('contextmenu', function(event) {
		pop_up_rec_menu();
		event.preventDefault();
		event.stopPropagation();
		});

	// Add the lane.
	this.lane = templates['lane'].clone(
		total_width - controls_width, track_height, controls_width);
	this.svg.appendChild(this.lane);

	// Call the API to get our data.
	if (json)
		this.got_json(json);
	else
		api_get("/api/track/" + this.id, function(json) { track.got_json(json); });
	api_get(
		"/api/track/" + this.id + "/clips",
		function(json) { track.got_clips_json(json); });
	};


Track.prototype.got_json = function(json) {
	var track = this;

	var name_element = find_element_by_id(this.div, "track-name");
	name_element.textContent = json.name;

	this.record_armed = json.record_armed || false;
	var rec_arm = find_element_by_id(this.track_svg, 'record-arm');
	if (this.record_armed)
		rec_arm.setAttribute("armed", "armed");
	else
		rec_arm.removeAttribute("armed");

	this.set_gain(json.gain);
	if (json['monitor_input'] == undefined)
		this.monitor_input = true;
	else
		this.monitor_input = json.monitor_input;
	this.input = json.input || null;

	// Load up the children.
	json.children.forEach(function(child_id) {
		new Track(child_id, track);
		});
	}


Track.prototype.got_clips_json = function(json) {
	json.forEach(clip => new Clip(this, clip));
	}

Track.prototype.add_clip_at = function(id, start) {
	return new Clip(this, id, start);
	}

Track.prototype.is_master = function() {
	return this.id == 1;
	};

Track.prototype.set_gain = function(new_gain) {
	this.gain = new_gain;
	this.gain_knob.set_db_value(gain_to_dB(this.gain));
	}


Track.prototype.append_child = function(child) {
	if (!this.children_div) {
		this.children_div = document.createElement('div');
		this.children_div.setAttribute('class', 'track-children');
		this.div.appendChild(this.children_div);
		}

	this.children_div.appendChild(child.div);
	this.children.push(child);
	};


Track.prototype.insert_child_after = function(child, after_child) {
	var children_div = null;
	if (this.is_master())
		children_div = document.getElementById('tracks');
	else {
		if (!this.children_div) {
			this.children_div = document.createElement('div');
			this.children_div.setAttribute('class', 'track-children');
			this.div.appendChild(this.children_div);
			}
		children_div = this.children_div;
		}

	children_div.insertBefore(child.div, after_child.div.nextSibling);
	var index = this.children.indexOf(after_child);
	if (index < 0)
		this.children.push(child);
	else
		this.children.splice(index + 1, 0, child);
	}

Track.prototype.insert_child_before = function(child, before_child) {
	var children_div = null;
	if (this.is_master())
		children_div = document.getElementById('tracks');
	else {
		if (!this.children_div) {
			this.children_div = document.createElement('div');
			this.children_div.setAttribute('class', 'track-children');
			this.div.appendChild(this.children_div);
			}
		children_div = this.children_div;
		}

	children_div.insertBefore(child.div, before_child.div);
	var index = this.children.indexOf(before_child);
	if (index < 0)
		this.children.push(child);
	else
		this.children.splice(index, 0, child);
	}

Track.prototype.remove_child = function(track) {
	// Remove from the DOM.
	var children_div = null;
	if (this.is_master())
		children_div = document.getElementById('tracks');
	else
		children_div = this.children_div;
	children_div.removeChild(track.div);

	// Remove from this.children.
	var index = this.children.indexOf(track);
	if (index >= 0)
		this.children.splice(index, 1);
	}

Track.prototype.child_after = function(track) {
	var index = this.children.indexOf(track);
	if (index < 0 || index >= this.children.length - 1)
		return null;
	return this.children[index + 1];
	}


Track.prototype.level = function() {
	var level = 0;
	for (var parent = this.parent; parent; parent = parent.parent)
		level += 1;
	return level;
	}


Track.prototype.update_meter = function(level) {
	// Delete old meter_rect.
	if (this.meter_rect) {
		this.track_svg.removeChild(this.meter_rect);
		this.meter_rect = null;
		}

	if (level == 0 || !this.meter_track)
		return;

	// Figure out the width.
	if (level > 1.0)
		level = 1.0;
	var dB = gain_to_dB(level);
	if (dB < min_dB)
		return;
	var max_width = parseFloat(this.meter_track.getAttribute('width'));
	var width = max_width * (min_dB - dB) / min_dB;

	// Create the new rect.
	this.meter_rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");;
	this.meter_rect.setAttribute('x', this.meter_track.getAttribute('x'));
	this.meter_rect.setAttribute('y', this.meter_track.getAttribute('y'));
	this.meter_rect.setAttribute('height', this.meter_track.getAttribute('height'));
	this.meter_rect.setAttribute('width', "" + width);
	this.meter_rect.setAttribute('class', 'meter');
	this.track_svg.insertBefore(this.meter_rect, this.meter_track);
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
		select_track(null);
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

	do_action(new ChangeTrackNameAction(track, new_name));
	}


Track.prototype.name = function() {
	var name_element = find_element_by_id(this.track_svg, "track-name");
	return name_element.textContent;
	}

Track.prototype.name_changed_to = function(new_name) {
	var name_element = find_element_by_id(this.track_svg, "track-name");
	name_element.textContent = new_name;
	}


Track.prototype.toggle_record_arm = function() {
	do_action(new ChangeRecordArmedAction(this, !this.record_armed));
	}

Track.prototype.set_record_armed = function(new_armed) {
	this.record_armed = new_armed;
	var rec_arm = find_element_by_id(this.track_svg, 'record-arm');
	if (this.record_armed)
		rec_arm.setAttribute("armed", "armed");
	else
		rec_arm.removeAttribute("armed");
	}


//===============//

function ChangeTrackNameAction(track, new_name) {
	Action.call(this);
	this.type = 'change-track-name';
	this.track = track;
	this.old_name = track.name();
	this.new_name = new_name;
	}

ChangeTrackNameAction.prototype = Object.create(Action.prototype);

ChangeTrackNameAction.prototype.do = function() {
	this.change_name(this.new_name);
	}
ChangeTrackNameAction.prototype.undo = function() {
	this.change_name(this.old_name);
	}

ChangeTrackNameAction.prototype.change_name = function(new_name) {
	// Send the API request to set the name.
	var request = new XMLHttpRequest();
	var action = this;
	request.onreadystatechange = function() {
		var DONE = this.DONE || 4;
		if (this.readyState === DONE) {
			if (this.status == 200)
				action.track.name_changed_to(new_name);
			}
		}
	request.open("PUT", "/api/track/" + this.track.id + "/name", true);
	request.send(new_name);
	}


//===============//

function ChangeTrackGainAction(track, new_gain) {
	Action.call(this);
	this.type = 'change-track-gain';
	this.track = track;
	this.old_gain = track.gain;
	this.new_gain = new_gain;
	this.is_fixed = false;
	}

ChangeTrackGainAction.prototype = Object.create(Action.prototype);

ChangeTrackGainAction.prototype.do = function() {
	this.change_gain(this.new_gain);
	}
ChangeTrackGainAction.prototype.undo = function() {
	this.change_gain(this.old_gain);
	this.track.set_gain(this.old_gain); 	// Update the knob too.
	}
ChangeTrackGainAction.prototype.redo = function() {
	this.change_gain(this.new_gain);
	this.track.set_gain(this.new_gain); 	// Update the knob too.
	}

ChangeTrackGainAction.prototype.can_incorporate = function(action) {
	var result =
		!this.is_fixed && action.type == 'change-track-gain' &&
		action.track == this.track;
	return result;
	}
ChangeTrackGainAction.prototype.incorporate = function(action) {
	this.new_gain = action.new_gain;
	}
ChangeTrackGainAction.prototype.fix = function() {
	this.is_fixed = true;
	}

ChangeTrackGainAction.prototype.change_gain = function(gain) {
	var request = new XMLHttpRequest();
	request.open("PUT", "/api/track/" + this.track.id + "/gain", true);
	request.send("" + gain);
	this.track.gain = gain;
	}

//===============//

function ChangeRecordArmedAction(track, new_armed) {
	Action.call(this);
	this.type = 'change-track-record-armed';
	this.track = track;
	this.new_armed = new_armed;
	}

ChangeRecordArmedAction.prototype = Object.create(Action.prototype);

ChangeRecordArmedAction.prototype.do = function() {
	this.change_armed(this.new_armed);
	}
ChangeRecordArmedAction.prototype.undo = function() {
	this.change_armed(!this.new_armed);
	}

ChangeRecordArmedAction.prototype.change_armed = function(armed) {
	var request = new XMLHttpRequest();
	var action = this;
	request.onreadystatechange = function() {
		var DONE = this.DONE || 4;
		if (this.readyState === DONE) {
			if (this.status == 200)
				action.track.set_record_armed(armed);
			}
		}
	request.open("PUT", "/api/track/" + this.track.id + "/record-arm", true);
	request.send("" + armed);
	}

//===============//

function ChangeTrackMonitorInputAction(track, new_monitor) {
	Action.call(this);
	this.type = 'change-track-monitor-input';
	this.track = track;
	this.new_monitor = new_monitor;
	}

ChangeTrackMonitorInputAction.prototype = Object.create(Action.prototype);

ChangeTrackMonitorInputAction.prototype.do = function() {
	this.change_monitor(this.new_monitor);
	}
ChangeTrackMonitorInputAction.prototype.undo = function() {
	this.change_monitor(!this.new_monitor);
	}

ChangeTrackMonitorInputAction.prototype.change_monitor = function(monitor) {
	var request = new XMLHttpRequest();
	var action = this;
	request.onreadystatechange = function() {
		var DONE = this.DONE || 4;
		if (this.readyState === DONE) {
			if (this.status == 200)
				action.track.monitor_input = monitor;
			}
		}
	request.open("PUT", "/api/track/" + this.track.id + "/monitor-input", true);
	request.send("" + monitor);
	}

//===============//

function ChangeTrackInputAction(track, new_input) {
	Action.call(this);
	this.type = 'change-track-input';
	this.track = track;
	this.old_input = track.input;
	this.new_input = new_input;
	}

ChangeTrackInputAction.prototype = Object.create(Action.prototype);

ChangeTrackInputAction.prototype.do = function() {
	this.change_input(this.new_input);
	}
ChangeTrackInputAction.prototype.undo = function() {
	this.change_input(this.old_input);
	}

ChangeTrackInputAction.prototype.change_input = function(input) {
	var request = new XMLHttpRequest();
	var action = this;
	request.onreadystatechange = function() {
		var DONE = this.DONE || 4;
		if (this.readyState === DONE) {
			if (this.status == 200)
				action.track.input = input;
			}
		}
	request.open("PUT", "/api/track/" + this.track.id + "/input", true);
	request.send(input || "- none -"); 	// Can't send an empty string for some reason.
	}

//===============//

function NewTrackAction() {
	Action.call(this);
	this.type = 'new-track';

	this.new_track = null;
	this.after_track = null;
	this.parent_track = null;
	if (selected_track && selected_track != master_track) {
		this.after_track = selected_track;
		this.parent_track = this.after_track.parent;
		}
	else
		this.parent_track = master_track;
	}

NewTrackAction.prototype = Object.create(Action.prototype);

NewTrackAction.prototype.do = function() {
	var action = this;
	var url = "/api/track";
	if (this.after_track)
		url += "?after=" + this.after_track.id;
	api_post(url, function(json) {
		var before_track = null;
		if (action.after_track)
			before_track = action.parent_track.child_after(action.after_track);
		action.new_track =
			new Track(json.id, action.parent_track, json, before_track);
		});
	}

NewTrackAction.prototype.undo = function() {
	// Delete the track.
	var action = this;
	var url = "/api/track/" + this.new_track.id;
	api_delete(url, function() {
		action.new_track.parent.remove_child(action.new_track);
		});
	}


//===============//

function DeleteTrackAction(track) {
	Action.call(this);
	this.type = 'delete-track';

	this.track = track;
	this.parent = track.parent;
	this.before_track = track.parent.child_after(track);
	}

DeleteTrackAction.prototype = Object.create(Action.prototype);

DeleteTrackAction.prototype.do = function() {
	var action = this;
	var url = "/api/track/" + this.track.id;
	api_delete(url, function() {
		action.parent.remove_child(action.track);
		});
	}

DeleteTrackAction.prototype.undo = function() {
	var action = this;
	var url = "/api/track?restore=" + this.track.id + ";parent=" + this.parent.id;
	if (this.before_track)
		url += ";before=" + this.before_track.id;
	api_post(url, function(json) {
		action.track =
			new Track(json.id, action.parent, json, action.before_track);
		});
	}


