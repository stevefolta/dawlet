var websocket = null;
var logging_enabled = false;

function log(message) {
	if (!logging_enabled)
		return;

	console.log(message);
	}


function load() {
	log("Loaded page.");

	if (!String.prototype.startsWith) {
		String.prototype.startsWith = function(searchString, position) {
			position = position || 0;
			return this.indexOf(searchString, position) === position;
			};
		}

	document.getElementById("open-project").onclick = function() {
		websocket.send("open-project \"test/test.json\"");
		};
	document.getElementById("list-interfaces").onclick = function() {
		websocket.send("list-interfaces");
		};
	document.getElementById("play").onclick = function() {
		websocket.send("play");
		}
	document.getElementById("stop").onclick = function() {
		websocket.send("stop");
		}
	document.getElementById("go").onclick = function() {
		websocket.send("open-project \"test/test.json\"");
		websocket.send("list-interfaces");
		websocket.send("play");
		}

	document.getElementById("start_message").textContent = "...in progress...";
	websocket = new WebSocket("ws://localhost:8080/socket");
	websocket.onmessage = function (event) {
		log("Got websocket message: \"" + event.data + "\"");
		if (event.data.startsWith("interfaces ")) {
			var json = event.data.substr(10);
			var interfaces = JSON.parse(json);
			if (interfaces && interfaces[0])
				websocket.send("select-interface \"" + interfaces[0] + "\"");
			}
		}
	websocket.onopen = function (event) {
		try {
			websocket.send("ping");
			websocket.send("get-play-head");
			}
		catch (e) {
			log("Websocket send failed!");
			}
		}
	}
