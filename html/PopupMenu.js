function PopupMenu(x, y, items) {
	var popup = this;

	this.menu_div = document.createElement('div');
	this.menu_div.setAttribute('class', 'popup-menu');
	};


PopupMenu.prototype.add_item = function(title, fn) {
	var item_element = document.createElement('a');
	var popup = this;
	item_element.textContent = title;
	item_element.onclick = function(event) {
		if (fn)
			fn();
		popup.close();
		};
	this.menu_div.appendChild(item_element);
	}

PopupMenu.prototype.add_divider = function() {
	var element = document.createElement('hr');
	this.menu_div.appendChild(element);
	}


PopupMenu.prototype.open = function(x, y) {
	var body = document.getElementsByTagName('body')[0];
	var document_view_rect = body.getBoundingClientRect();
	this.menu_div.style.left = x - document_view_rect.left;
	this.menu_div.style.top = y - document_view_rect.top;

	var popup = this;
	this.menu_div.onmouseout = function() {
		// body.removeChild(popup.menu_div);
		};
	body.appendChild(this.menu_div);
	}


PopupMenu.prototype.close = function() {
	var body = document.getElementsByTagName('body')[0];
	body.removeChild(this.menu_div);
	}



