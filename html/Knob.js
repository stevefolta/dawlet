function Knob(knob_element) {
	this.track = find_element_by_id(knob_element, 'knob-track');
	this.indicator = find_element_by_id(knob_element, 'knob-indicator');

	var indicator_rect = this.indicator.getBoundingClientRect();
	var track_rect = this.track.getBoundingClientRect();
	this.max_value = Math.round(track_rect.width - indicator_rect.width);
	this.is_db_knob = false;

	var knob = this;
	this.indicator.onmousedown = function(event) {
		new KnobTracker(event, knob);
		};
	}

Knob.prototype.db_above_zero = 6;
Knob.prototype.db_below_zero = 100;


Knob.prototype.cur_value = function() {
	var indicator_rect = this.indicator.getBoundingClientRect();
	var track_rect = this.track.getBoundingClientRect();
	return Math.round(indicator_rect.left - track_rect.left);
	}

Knob.prototype.set_value = function(value) {
	if (value < 0)
		value = 0;
	else if (value > this.max_value)
		value = this.max_value;
	this.indicator.setAttribute('transform', "translate(" + value + ", 0)");
	}

Knob.prototype.db_value = function() {
	var zero_point = Math.round((this.max_value * 3) / 4);
	var value = this.cur_value();
	var db = 0.0;
	// Handle end values specially, to avoid rounding problems.
	if (value == 0)
		db = -this.db_below_zero;
	else if (value == this.max_value)
		db = this.db_above_zero;
	else if (value >= zero_point) {
		db =
			this.db_above_zero * (value - zero_point) /
			(this.max_value - zero_point);
		}
	else {
		db = -this.db_below_zero * (zero_point - value) / zero_point;
		}
	return db;
	}

Knob.prototype.set_db_value = function(db) {
	if (db > this.db_above_zero)
		db = this.db_above_zero;
	else if (db < -this.db_below_zero)
		db = -this.db_below_zero;

	var zero_point = Math.round((this.max_value * 3) / 4);
	var value = 0;
	if (db >= 0) {
		value =
			zero_point +
			Math.round((this.max_value - zero_point) * db / this.db_above_zero);
		}
	else {
		value = zero_point + Math.round(zero_point * db / this.db_below_zero);
		}
	this.set_value(value);
	};



function KnobTracker(event, knob) {
	MouseTracker.call(this, event);
	this.knob = knob;
	this.start_position = knob.cur_value();
	this.max_position = knob.max_value;
	}

KnobTracker.prototype = Object.create(MouseTracker.prototype);

KnobTracker.prototype.mouse_moved = function(x, y, event) {
	var offset = x - this.start_x;
	var new_position = this.start_position + offset;
	this.knob.set_value(new_position);
	if (this.knob.is_db_knob)
		show_entered_value(this.knob.db_value().toFixed(2) + "dB");
	};

KnobTracker.prototype.mouse_up = function(event) {
	show_entered_value("");
	};



