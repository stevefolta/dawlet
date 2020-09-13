function Clip(track, id_or_json, start_time) {
	this.track = track;
	if (typeof(id_or_json) === 'number') {
		this.id = id_or_json;
		this.start_time = start_time;
		this.length = 0;
		}
	else {
		let json = id_or_json;
		this.id = json.id || 0;
		this.start_time = json.start;
		this.length = json.length || 0;
		}

	// Create the SVG.
	var lanes_rect = this.track.lane.getBoundingClientRect();
	var track_svg_rect = this.track.svg.getBoundingClientRect();
	this.svg = templates['clip'].clone(
		this.length * pixels_per_second, lanes_rect.height,
		lanes_rect.left - track_svg_rect.left + this.start_time * pixels_per_second,
		0);
	this.track.svg.appendChild(this.svg);
	}

