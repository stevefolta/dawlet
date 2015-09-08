function PopupMenu(x, y, parent, items) {
	this.items = items;

	var menu = document.createElement('div');
	menu.setAttribute('class', 'popup-menu');
	menu.style.left = x;
	menu.style.top = y;

	items.forEach(function(item) {
		var item_element = document.createElement('a');
		item_element.textContent = item;
		menu.appendChild(item_element);
		});

	menu.onmouseout = function() {
		// parent.removeChild(menu);
		};
	parent.insertBefore(menu, parent.firstChild);
	};


function pop_up_menu(x, y, parent, items) {
	new PopupMenu(x, y, parent, items);
	}


