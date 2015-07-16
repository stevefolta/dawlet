var websocket = null;
var logging_enabled = false;

function log(message) {
	if (!logging_enabled)
		return;

	console.log(message);
	}


function load() {
	log("Loaded page.");
	document.getElementById("start_message").textContent = "...in progress...";
	websocket = new WebSocket("ws://localhost:8080/socket");
	websocket.onmessage = function (event) {
		log("Got websocket message: \"" + event.data + "\"");
		}
	websocket.onopen = function (event) {
		try {
			websocket.send("get-play-head");
			}
		catch (e) {
			log("Websocket send failed!");
			}
		}
	}
