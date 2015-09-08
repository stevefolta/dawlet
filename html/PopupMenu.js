function PopupMenu(x, y, items) {
	this.items = items;

	var popup = this;
	var body = document.getElementsByTagName('body')[0];

	this.menu_div = document.createElement('div');
	this.menu_div.setAttribute('class', 'popup-menu');
	var document_view_rect = body.getBoundingClientRect();
	this.menu_div.style.left = x - document_view_rect.left;
	this.menu_div.style.top = y - document_view_rect.top;

	items.forEach(function(item) {
		var item_element = document.createElement('a');
		item_element.textContent = item;
		item_element.onclick = function(event) {
			popup.close();
			};
		popup.menu_div.appendChild(item_element);
		});

	this.menu_div.onmouseout = function() {
		// body.removeChild(popup.menu_div);
		};
	body.appendChild(this.menu_div);
	};

PopupMenu.prototype.close = function() {
	var body = document.getElementsByTagName('body')[0];
	body.removeChild(this.menu_div);
	}


function pop_up_menu(x, y, items) {
	new PopupMenu(x, y, items);
	}


