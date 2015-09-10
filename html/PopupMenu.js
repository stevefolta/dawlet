var cur_popup = null;

function PopupMenu(x, y, items) {
	this.cur_submenu = null;
	this.parent_menu = null;

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
		close_popup_menu();
		};
	item_element.onmouseover = function(event) {
		popup.open_submenu(null);
		};
	this.menu_div.appendChild(item_element);
	}

PopupMenu.prototype.add_divider = function() {
	var element = document.createElement('hr');
	this.menu_div.appendChild(element);
	}

PopupMenu.prototype.add_submenu = function(title, submenu) {
	var item_element = document.createElement('table');
	item_element.setAttribute('class', 'submenu-item');
	var tr = document.createElement('tr');
	item_element.appendChild(tr);
	var title_element = document.createElement('td');
	title_element.setAttribute('class', 'title');
	title_element.textContent = title;
	tr.appendChild(title_element);
	var arrow_element = document.createElement('td');
	arrow_element.setAttribute('class', 'arrow');
	arrow_element.textContent = '\u25B6';
	tr.appendChild(arrow_element);

	item_element.submenu = submenu;
	var popup = this;
	item_element.onmouseover = function(event) {
		popup.open_submenu(item_element);
		};

	this.menu_div.appendChild(item_element);
	}

PopupMenu.prototype.open_submenu = function(item_element) {
	if (this.cur_submenu)
		this.cur_submenu.close();
	this.cur_submenu = item_element ? item_element.submenu : null;
	if (this.cur_submenu) {
		this.cur_submenu.parent_menu = this;
		var item_element_rect = item_element.getBoundingClientRect();
		this.cur_submenu.open(item_element_rect.right, item_element_rect.top);
		}
	}


PopupMenu.prototype.open = function(x, y) {
	var body = document.getElementsByTagName('body')[0];
	var document_view_rect = body.getBoundingClientRect();
	this.menu_div.style.left = x - document_view_rect.left;
	this.menu_div.style.top = y - document_view_rect.top;

	var popup = this;
	this.menu_div.onmouseout = function() {
		// popup.close();
		};
	body.appendChild(this.menu_div);

	if (!cur_popup)
		cur_popup = this;
	}


PopupMenu.prototype.close = function() {
	var body = document.getElementsByTagName('body')[0];
	body.removeChild(this.menu_div);
	if (this.cur_submenu)
		this.cur_submenu.close();
	if (cur_popup == this)
		cur_popup = null;
	}


function popup_menu_is_up()
{
	return !!cur_popup;
}

function close_popup_menu()
{
	if (cur_popup)
		cur_popup.close();
}


