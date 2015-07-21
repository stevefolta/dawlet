var websocket = null;
var logging_enabled = false;

function log(message) {
	if (!logging_enabled)
		return;

	console.log(message);
	}


function populate_interfaces(interfaces) {
	// Clear out existing elements.
	var popup = document.getElementById("interface-popup");
	while (popup.length > 0)
		popup.remove(0);

	// Add the interfaces.
	interfaces.forEach(function(interface) {
		var option = document.createElement('option');
		option.value = interface;
		option.text = interface;
		popup.add(option);
		});
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
		};
	document.getElementById("stop").onclick = function() {
		websocket.send("stop");
		};
	document.getElementById("pause").onclick = function() {
		websocket.send("pause");
		};
	document.getElementById("rewind").onclick = function() {
		websocket.send("rewind");
		};
	var go_button = document.getElementById("go");
	if (go_button) {
		go_button.onclick = function() {
			websocket.send("open-project \"test/test.json\"");
			websocket.send("play");
			};
		}
	document.getElementById("interface-popup").onchange = function(event) {
		websocket.send("select-interface \"" + event.target.value + "\"");
		};
	window.setInterval(function() {
		websocket.send("get-play-head");
		},
		200);

	document.getElementById("start_message").textContent = "...in progress...";
	websocket = new WebSocket("ws://localhost:8080/socket");
	websocket.onmessage = function (event) {
		log("Got websocket message: \"" + event.data + "\"");
		if (event.data.startsWith("interfaces ")) {
			var json = event.data.substr(10);
			var interfaces = JSON.parse(json);
			if (interfaces && interfaces[0]) {
				populate_interfaces(interfaces);
				websocket.send("select-interface \"" + interfaces[0] + "\"");
				}
			}
		else if (event.data.startsWith("play-head ")) {
			var play_head = parseFloat(event.data.substr(10));
			document.getElementById("play-head").textContent = "" + play_head.toFixed(3);
			}
		};
	websocket.onopen = function (event) {
		try {
			websocket.send("ping");
			websocket.send("list-interfaces");
			websocket.send("open-project \"test/test.json\"");
			}
		catch (e) {
			log("Websocket send failed!");
			}
		}
	}

