function SVGTemplate(svg_doc) {
	this.template = svg_doc.getElementById('layer1');

	// Find the declared size.
	this.declared_width = null;
	this.declared_height = null;
	var svg_element = null;
	for (var node = svg_doc.firstChild; node; node = node.nextSibling) {
		if (node.nodeName == "svg") {
			svg_element = node;
			break;
			}
		}
	if (svg_element) {
		this.declared_width = svg_element.getAttribute('width');
		this.declared_height = svg_element.getAttribute('height');
		}
	}


SVGTemplate.prototype.clone = function(new_width, new_height, x_move, y_move) {
	var result = this.template.cloneNode(true);

	if (new_width && new_height && this.declared_width && this.declared_height) {
		this.scale_svg_elements(
			result,
			new_width - this.declared_width,
			new_height - this.declared_height,
			x_move, y_move);
		}

	return result;
	}


SVGTemplate.prototype.scale_svg_elements = function(element, width_delta, height_delta, x_move, y_move) {
	function change_attribute(attr_name, delta) {
		var attribute = element.getAttribute(attr_name);
		var old_value = attribute ? parseFloat(attribute) : 0;
		element.setAttribute(attr_name, "" + (old_value + delta));
		}

	var attach_spec = element.getAttribute('attach');
	if (attach_spec) {
		var attach_bits = parseInt(attach_spec, 2);
		x_move = x_move || 0;
		y_move = y_move || 0;
		var new_width = 0, new_height = 0;
		if ((attach_bits & 0b1010) == 0b1010)
			x_move += width_delta;
		else if ((attach_bits & 0b1010) == 0b0010) {
			var attribute = element.getAttribute('width');
			var width = attribute ? parseFloat(attribute) : 0;
			new_width = width + width_delta;
			}
		// 0b1000 is not supported.
		if ((attach_bits & 0b0101) == 0b0101)
			y_move += height_delta;
		else if ((attach_bits & 0b0101) == 0b0010) {
			var attribute = element.getAttribute('height');
			var height = attribute ? parseFloat(attribute) : 0;
			new_height = height + height_delta;
			}
		if (element.nodeName == 'path')
			this.scale_svg_path(element, width_delta, height_delta, x_move, y_move);
		else if (element.nodeName == 'circle')
			this.scale_svg_circle(element, width_delta, height_delta, x_move, y_move);
		else {
			if (x_move)
				change_attribute('x', x_move);
			if (y_move)
				change_attribute('y', y_move);
			if (new_width)
				element.setAttribute('width', new_width);
			if (new_height)
				element.setAttribute('height', new_height);
			}
		}
	else if (x_move || y_move) {
		if (element.nodeName == 'path')
			this.scale_svg_path(element, 0, 0, x_move, y_move);
		else if (element.nodeName == 'circle')
			this.scale_svg_circle(element, 0, 0, x_move, y_move);
		else {
			if (x_move)
				change_attribute('x', x_move);
			if (y_move)
				change_attribute('y', y_move);
			}
		}

	for (var child = element.firstElementChild; child; child = child.nextElementSibling)
		this.scale_svg_elements(child, width_delta, height_delta, x_move, y_move);
	}


SVGTemplate.prototype.scale_svg_path = function(element, width_delta, height_delta, x_move, y_move) {
	var path = element.getAttribute('d');
	path = path.match(/[A-Za-z]|[0-9.]+/g);

	function is_number(str) {
		return !isNaN(str);
		}

	// Figure out the size.
	// We're only including the path points.  For curves, is it more correct to
	// include the control points too?
	var save_path = path.slice();
	var is_first_point = true;
	var x, y;
	var min_x = 0, min_y = 0;
	var max_x = 0, max_y = 0;
	var pen_x = 0, pen_y = 0;
	function update_min_max(x, y) {
		if (x < min_x)
			min_x = x;
		if (x > max_x)
			max_x = x;
		if (y < min_y)
			min_y = y;
		if (y > max_y)
			max_y = y;
		}
	function update_pen(x, y) {
		pen_x = x;
		pen_y = y;
		}
	while (path.length > 0) {
		var command = path.shift();
		var is_relative = false;
		switch (command) {
			case 'm':
			case 'l':
			case 't':
				is_relative = true;
			case 'M':
			case 'L':
			case 'T':
				while (is_number(path[0])) {
					x = parseFloat(path.shift());
					y = parseFloat(path.shift());
					if (is_first_point) {
						min_x = max_x = pen_x = x;
						min_y = max_y = pen_y = y;
						is_first_point = false;
						}
					else {
						if (is_relative) {
							x += pen_x;
							y += pen_y;
							}
						update_min_max(x, y);
						}
					}
				update_pen(x, y);
				break;

			case 'h':
				is_relative = true;
			case 'H':
				while (is_number(path[0])) {
					x = parseFloat(path.shift());
					if (is_relative)
						x += pen_x;
					update_min_max(x, pen_y);
					}
				update_pen(x, pen_y);
				break;

			case 'l':
				is_relative = true;
			case 'L':
				while (is_number(path[0])) {
					y = parseFloat(path.shift());
					if (is_relative)
						y += pen_y;
					update_min_max(pen_x, y);
					}
				update_pen(pen_x, y);
				break;

			case 'c':
				is_relative = true;
			case 'C':
				while (is_number(path[0])) {
					path.shift();	// Skip x1.
					path.shift();	// Skip y1.
					path.shift();	// Skip x2.
					path.shift();	// Skip y2.
					x = parseFloat(path.shift());
					y = parseFloat(path.shift());
					if (is_relative) {
						x += pen_x;
						y += pen_y;
						}
					update_min_max(x, y);
					}
				update_pen(x, y);
				break;

			case 's':
			case 'q':
				is_relative = true;
			case 'S':
			case 'Q':
				while (is_number(path[0])) {
					path.shift();	// Skip x2/x1.
					path.shift();	// Skip y2/y1.
					x = parseFloat(path.shift());
					y = parseFloat(path.shift());
					if (is_relative) {
						x += pen_x;
						y += pen_y;
						}
					update_min_max(x, y);
					}
				update_pen(x, y);
				break;

			case 'a':
				is_relative = true;
			case 'A':
				while (is_number(path[0])) {
					path.shift();	// Skip rx.
					path.shift();	// Skip ry.
					path.shift();	// Skip x-axis-rotation.
					path.shift();	// Skip large-arc-flag.
					path.shift();	// Skip sweep-flag.
					x = parseFloat(path.shift());
					y = parseFloat(path.shift());
					if (is_relative) {
						x += pen_x;
						y += pen_y;
						}
					update_min_max(x, y);
					}
				update_pen(x, y);
				break;
			}
		}

	// Rebuild the path with updated coordinates.
	path = save_path;
	var result = [];
	is_first_point = true;
	var old_width = max_x - min_x;
	var x_scale = 1;
	if (old_width)
		x_scale = (old_width + width_delta) / old_width;
	var old_height = max_y - min_y;
	var y_scale = 1;
	if (old_height)
		y_scale = (old_height + height_delta) / old_height;
	var is_relative = false;
	function add_updated_point(old_x, old_y) {
		result.push(min_x + (old_x - min_x) * x_scale + x_move);
		result.push(min_y + (old_y - min_y) * y_scale + y_move);
		}
	function transpose_point() {
		x = parseFloat(path.shift());
		y = parseFloat(path.shift());
		if (is_relative) {
			x += pen_x;
			y += pen_y;
			}
		add_updated_point(x, y);
		}
	while (path.length > 0) {
		var command = path.shift();
		is_relative = false;
		switch (command) {
			case 'm':
			case 'l':
			case 't':
				is_relative = true;
			case 'M':
			case 'L':
			case 'T':
				result.push(command.toUpperCase());
				while (is_number(path[0])) {
					x = parseFloat(path.shift());
					y = parseFloat(path.shift());
					if (is_first_point) {
						pen_x = x;
						pen_y = y;
						is_first_point = false;
						}
					else {
						if (is_relative) {
							x += pen_x;
							y += pen_y;
							}
						}
					add_updated_point(x, y);
					}
				update_pen(x, y);
				break;

			case 'h':
				is_relative = true;
			case 'H':
				result.push(command.toUpperCase());
				while (is_number(path[0])) {
					x = parseFloat(path.shift());
					if (is_relative)
						x += pen_x;
					add_updated_point(x, pen_y);
					}
				update_pen(x, pen_y);
				break;

			case 'l':
				is_relative = true;
			case 'L':
				result.push(command.toUpperCase());
				while (is_number(path[0])) {
					y = parseFloat(path.shift());
					if (is_relative)
						y += pen_y;
					add_updated_point(pen_x, y);
					}
				update_pen(pen_x, y);
				break;

			case 'c':
				is_relative = true;
			case 'C':
				result.push(command.toUpperCase());
				while (is_number(path[0])) {
					transpose_point(); 	// x1, y1.
					transpose_point();	// x2, y2.
					transpose_point();	// x, y.
					}
				update_pen(x, y);
				break;

			case 's':
			case 'q':
				is_relative = true;
			case 'S':
			case 'Q':
				result.push(command.toUpperCase());
				while (is_number(path[0])) {
					transpose_point();	// x2, y2 / x1, y1.
					transpose_point();	// x, y.
					}
				update_pen(x, y);
				break;

			case 'a':
				is_relative = true;
			case 'A':
				result.push(command.toUpperCase());
				while (is_number(path[0])) {
					result.push(parseFloat(path.shift() * x_scale)); 	// rx.
					result.push(parseFloat(path.shift() * y_scale)); 	// ry.
					result.push(path.shift()); 	// x-axis-rotation.
					result.push(path.shift()); 	// large-arc-flag.
					result.push(path.shift()); 	// sweep-flag.
					transpose_point();	// x, y.
					}
				update_pen(x, y);
				break;
			}
		}
	element.setAttribute('d', result.join(' '));
	}


SVGTemplate.prototype.scale_svg_circle = function(element, width_delta, height_delta, x_move, y_move) {
	if (width_delta || height_delta)
		log("Scaling circles is not (yet) supported.");
	element.setAttribute('cx', "" + (parseFloat(element.getAttribute('cx')) + x_move));
	element.setAttribute('cy', "" + (parseFloat(element.getAttribute('cy')) + y_move));
	}


