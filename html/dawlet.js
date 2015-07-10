var logging_enabled = false;

function log(message) {
	if (!logging_enabled)
		return;

	console.log(message);
	}


function load() {
	log("Loaded page.");
	document.getElementById("start_message").textContent = "...in progress...";
	}
