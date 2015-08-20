function Track(id, parent) {
	this.id = id;
	this.children_div = null;
	var track = this;

	// Create the elements (<div> & <svg>).
	this.div = document.createElement('div');
	this.div.setAttribute('class', 'track');
	this.track_svg = templates['track'].clone();
	var name_element = find_element_by_id(this.track_svg, "track-name");
	name_element.textContent = "";
		// Until we get the real name.
	this.svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");;
	this.div.appendChild(this.svg);
	this.svg.appendChild(this.track_svg);

	// Add this track's elements into the document in the appropriate place.
	// We have to do this before we start dealing with widths and heights.
	if (this.is_master())
		document.getElementById('master').appendChild(this.div);
	else if (parent.is_master())
		document.getElementById('tracks').appendChild(this.div);
	else
		parent.append_child(this);

	// For some reason, we need to set the SVG's width & height.
	var size = this.track_svg.getBoundingClientRect();
	this.svg.style.width = size.width;
	this.svg.style.height = size.height;

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
	//***
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
	};


