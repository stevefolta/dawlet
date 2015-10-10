function setup_menus() {
	var file_menu = document.getElementById('file-menu');
	if (file_menu)
		file_menu.onclick = file_menu_clicked;
	}

function file_menu_clicked() {
	var menu = new PopupMenu(false);
	menu.add_item("New project...", function() {
		var name = window.prompt("New project name:", "");
		if (name) {
			websocket.send("new-project \"" + name + "\"");
			update_project_title(name);
			}
		});
	menu.add_divider();
	menu.add_item("Shut down", function() {
		websocket.send("shutdown");
		project_closed();
		});
	var rect = document.getElementById('file-menu').getBoundingClientRect();
	menu.open(rect.left, rect.bottom);
	}

