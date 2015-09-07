function Action() {
	this.name = '';
	this.prev_action = null;
	this.next_action = null;
	}

Action.prototype.do = function() {};
Action.prototype.undo = function() {};
Action.prototype.redo = function() {
	// Default: same as do().
	this.do();
	}
Action.prototype.fix = function() {};
Action.prototype.can_incorporate = function(new_action) {
	return false;
	};
Action.prototype.incorporate = function() {};

//===============//

function SentinalAction() {
	Action.call(this);
	this.name = 'sentinal';
	}

SentinalAction.prototype = Object.create(Action.prototype);


var last_action = new SentinalAction;

function do_action(action) {
	if (last_action) {
		if (last_action.can_incorporate(action)) {
			last_action.incorporate(action);
			return;
			}
		last_action.next_action = action;
		action.prev_action = last_action;
		}

	action.do();
	last_action = action;
	}

function undo_action() {
	if (!last_action || last_action.name == 'sentinal')
		return;

	last_action.undo();
	last_action = last_action.prev_action;
	}

function redo_action() {
	var action = last_action && last_action.next_action;
	if (!action)
		return;

	action.redo();
	last_action = action;
	}


//===============//

function ChangeTrackNameAction(track, new_name) {
	Action.call(this);
	this.name = 'change-track-name';
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

