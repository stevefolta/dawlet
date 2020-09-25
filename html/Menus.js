function setup_menus() {
	var file_menu = document.getElementById('file-menu');
	if (file_menu)
		file_menu.onclick = file_menu_clicked;
	}

function file_menu_clicked() {
	var menu = new PopupMenu(false);
	menu.add_item("New project...", function() {
		attempt_new_project();
		});
	menu.add_item("Open project...", show_open_project_screen);
	menu.add_item("Close project", close_project);
	menu.add_divider();
	menu.add_item("Shut down", function() {
		websocket.send("shutdown");
		project_closed();
		server_shut_down();
		});
	var rect = document.getElementById('file-menu').getBoundingClientRect();
	menu.open(rect.left, rect.bottom);
	}

