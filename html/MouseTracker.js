function MouseTracker(event) {
	this.start_x = event.clientX;
	this.start_y = event.clientY;
	this.last_x = this.start_x;
	this.last_y = this.start_y;

	// Set up to track events.
	// Listen on the document rather than the target element, so we still get
	// events when outside the element.
	var tracker = this;
	this.move_listener = function (event) {
		tracker.mouse_moved_outer(event);
		};
	document.addEventListener('mousemove', this.move_listener);
	this.up_listener = function (event) {
		tracker.mouse_up(event);
		}
	document.addEventListener('mouseup', this.up_listener);

	// Prevent it from trying to select or drag-n-drop.
	event.preventDefault();
	event.stopPropagation();
	}


MouseTracker.prototype.mouse_moved_outer = function(event) {
	var x = event.clientX;
	var y = event.clientY;
	if (x == this.last_x && y == this.last_y)
		return;

	this.mouse_moved(x, y, event);

	this.last_x = x;
	this.last_y = y;
	};


MouseTracker.prototype.mouse_up = function(event) {
	document.removeEventListener('mousemove', this.move_listener);
	document.removeEventListener('mouseup', this.up_listener);
	};


