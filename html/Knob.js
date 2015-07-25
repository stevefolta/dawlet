function Knob(knob_element) {
	this.track = find_element_by_id(knob_element, 'knob-track');
	this.indicator = find_element_by_id(knob_element, 'knob-indicator');
	var knob = this;
	this.indicator.onmousedown = function(event) {
		new KnobTracker(event, knob);
		};
	}


function KnobTracker(event, knob) {
	MouseTracker.call(this, event);
	this.knob = knob;
	var indicator_rect = knob.indicator.getBoundingClientRect();
	var track_rect = knob.track.getBoundingClientRect();
	this.start_position = indicator_rect.left - track_rect.left;
	this.max_position = track_rect.width - indicator_rect.width;
	}

KnobTracker.prototype = Object.create(MouseTracker.prototype);

KnobTracker.prototype.mouse_moved = function(x, y, event) {
	var offset = x - this.start_x;
	var new_position = this.start_position + offset;
	if (new_position < 0)
		new_position = 0;
	else if (new_position > this.max_position)
		new_position = this.max_position;
	this.knob.indicator.setAttribute('transform', "translate(" + new_position + ", 0)");
	};



