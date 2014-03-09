// See: http://en.wikipedia.org/wiki/Rose_%28mathematics%29

var drawRhodonea = function (n, d, depth, scale) {
	var left_limit  = 0;
	var right_limit = 4 * Math.PI;

	for (var theta = left_limit; theta <= right_limit; theta += 0.02) {
		r = Math.cos(n / d * theta);
		x = r * Math.cos(theta) * scale;
		y = r * Math.sin(theta) * scale;

        if (getZ() != depth) {
            rapid(x, y);
            cut({z: depth});
        }

		cut(x, y);
	}
}

tool_set({number: 1, units: IMPERIAL, length: 1/4, diameter: 1/2,
          shape: CONICAL});
tool_set({number: 2, units: IMPERIAL, length: 1, diameter: 1/8,
          shape: CYLINDRICAL});

feed(200);
speed(4000);

tool(2);
tool(1);
rapid({z: 5});

scale(0.25, 0.25);

drawRhodonea(7, 2, -6, 100);
rapid({z: 5});

rotate(2 * Math.PI / 14 / 2);
drawRhodonea(7, 2, -7, 90);
rapid({z: 5});

// Cutout
tool(2);

loadIdentity();
var radius = 30;
var maxStep = 4;
var depth = 14;
var steps = Math.ceil(depth / maxStep);
var delta = depth / steps;

rapid(-radius, 0);
for (var i = 0; i < steps; i++) {
    cut({z: -delta * (i + 1)});
    arc({x: radius, y: 0, z: 0, angle: 2 * Math.PI, plane: XY});
}
rapid({z: 5});

speed(0);
