function setup_menus() {
	var file_menu = document.getElementById('file-menu');
	if (file_menu)
		file_menu.onclick = file_menu_clicked;
	}

function file_menu_clicked() {
	var menu = new PopupMenu(false);
	menu.add_item("Shut down", function() {
		websocket.send("shutdown");
		project_closed();
		});
	var rect = document.getElementById('file-menu').getBoundingClientRect();
	menu.open(rect.left, rect.bottom);
	}

