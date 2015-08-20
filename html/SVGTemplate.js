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


SVGTemplate.prototype.clone = function(new_width, new_height) {
	var result = this.template.cloneNode(true);

	if (new_width && new_height && this.declared_width && this.declared_height) {
		this.scale_svg_elements(
			result,
			new_width - this.declared_width,
			new_height - this.declared_height);
		}

	return result;
	}


SVGTemplate.prototype.scale_svg_elements = function(element, width_delta, height_delta) {
	var attach_spec = element.getAttribute('attach');
	if (attach_spec) {
		var attach_bits = parseInt(attach_spec, 2);
		var rect = element.getBoundingClientRect();
		var x_move = 0, y_move = 0;
		var x_scale = 0, y_scale = 0;
		if ((attach_bits & 0b1010) == 0b1010)
			x_move = width_delta;
		else if ((attach_bits & 0b1010) == 0b0010)
			x_scale = (rect.width + width_delta) / rect.width;
		// 0b1000 is not supported.
		if ((attach_bits & 0b0101) == 0b0101)
			y_move = height_delta;
		else if ((attach_bits & 0b0101) == 0b0010)
			y_scale = (rect.height + height_delta) / rect.height;
		function change_attribute(attr_name, delta) {
			var attribute = element.getAttribute(attr_name);
			var old_value = attribute ? parseFloat(attribute) : 0;
			element.setAttribute(attr_name, "" + (old_value + delta));
			}
		if (x_move)
			change_attribute('x', x_move);
		if (y_move)
			change_attribute('y', y_move);
		if (x_scale)
			change_attribute('width', x_scale);
		if (y_scale)
			change_attribute('height', y_scale);
		}

	for (var child = element.firstElementChild; child; child = child.nextElementSibling)
		this.scale_svg_elements(child, width_delta, height_delta);
	}


