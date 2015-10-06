function RecordAction() {
	Action.call(this);
	this.type = 'Record';
	}

RecordAction.prototype = Object.create(Action.prototype);


RecordAction.prototype.do = function() {
	websocket.send("record");
	//***
	}

RecordAction.prototype.undo = function() {
	//***
	}

RecordAction.prototype.redo = function() {
	//***
	}

