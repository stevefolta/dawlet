function Action() {
	this.type = '';
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
	this.type = 'sentinal';
	}

SentinalAction.prototype = Object.create(Action.prototype);


var last_action = new SentinalAction;

function do_action(action) {
	if (last_action) {
		if (last_action.can_incorporate(action)) {
			action.do();
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
	if (!last_action || last_action.type == 'sentinal')
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


