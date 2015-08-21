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
		function change_attribute(attr_name, delta) {
			var attribute = element.getAttribute(attr_name);
			var old_value = attribute ? parseFloat(attribute) : 0;
			element.setAttribute(attr_name, "" + (old_value + delta));
			}
		if (x_move)
			change_attribute('x', x_move);
		if (y_move)
			change_attribute('y', y_move);
		if (new_width)
			element.setAttribute('width', new_width);
		if (new_height)
			element.setAttribute('height', new_height);
		}

	for (var child = element.firstElementChild; child; child = child.nextElementSibling)
		this.scale_svg_elements(child, width_delta, height_delta, x_move, y_move);
	}


