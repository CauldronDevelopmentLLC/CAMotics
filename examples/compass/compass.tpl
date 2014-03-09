var width = 25.4 * 1.9;
var length = 70.25;
var side = 59.25;
var depth = 25.4 * 0.25;


function print_point(point) {
    print('(', point[0], ', ', point[1], ')');
}


function print_poly(poly) {
    print('(');

    for (var j = 0; j < poly.length; j++) {
        if (j) print(', ');
        print_point(poly[j]);
    }

    print(')\n');
}


function arc2D(start, end) {
    var points = [];

    start = 2 * Math.PI - start;
    end = 2 * Math.PI - end;

    var steps = Math.round(Math.abs(end - start) / (Math.PI * 2 / 100)) + 1;
    var delta = (end - start) / steps;

    for (var i = 0; i < steps; i++) {
        var angle = start + delta * i;
        points.push([Math.cos(angle), Math.sin(angle)]);
    }

    return points;
}


function scale2D(points, factor) {
    var result = [];

    for (var i = 0; i < points.length; i++) {
        var point = [];

        for (var j = 0; j < points[i].length; j++)
            point.push(points[i][j] * factor[j]);

        result.push(point);
    }

    return result;
}


function translate2D(points, offset) {
    var result = [];

    for (var i = 0; i < points.length; i++) {
        var point = [];

        for (var j = 0; j < points[i].length; j++)
            point.push(points[i][j] + offset[j]);

        result.push(point);
    }

    return result;
}


function shape(width, length, side, radius) {
    var poly = [];

    poly.push([0, radius]);
    poly.push([0, side]);

    poly = poly.concat(
        translate2D(
            scale2D(arc2D(Math.PI, 2 * Math.PI), [width / 2, length - side]),
            [width / 2, side]));

    poly.push([width, radius]);

    var points = scale2D(arc2D(2 * Math.PI, 2.5 * Math.PI), [radius, radius]);
    poly = poly.concat(translate2D(points, [width - radius, radius]));

    poly.push([radius, 0]);

    var points = scale2D(arc2D(0.5 * Math.PI, Math.PI), [radius, radius]);
    poly = poly.concat(translate2D(points, [radius, radius]));

    return poly;
}


function cut_poly(poly, depth, safe) {
    if (typeof safe == 'undefined') safe = true;

    if (safe) rapid(poly[0][0], poly[0][1]);
    else cut(poly[0][0], poly[0][1]);

    cut({z: -depth});

    for (var i = 1; i < poly.length; i++)
        cut(poly[i][0], poly[i][1]);

    if (poly[0][0] != poly[poly.length -1][0] ||
        poly[0][1] != poly[poly.length -1][1])
        cut(poly[0][0], poly[0][1]);

    if (safe) rapid({z: 5});
}


function cut_polys(polys, depth, safe) {
    for (var i = 0; i < polys.length; i++)
        cut_poly(polys[i], depth, safe);
}


function cut_poly_steps(poly, depth, safe, tabs) {
    if (typeof safe == 'undefined') safe = true;
    if (typeof tabs == 'undefined') tabs = false;

    var steps = Math.ceil(depth / maxPass);
    var pass = depth / steps;

    if (safe) rapid(poly[0][0], poly[0][1]);
    else cut(poly[0][0], poly[0][1]);

    for (var step = 1; step <= steps; step++) {
        cut({z: -pass * step});

        for (var i = 1; i < poly.length; i++)
            cut(poly[i][0], poly[i][1]);

        if (poly[0][0] != poly[poly.length -1][0] ||
            poly[0][1] != poly[poly.length -1][1])
            cut(poly[0][0], poly[0][1]);
    }

    if (safe) rapid({z: 5});
}


function cut_polys_steps(polys, depth, safe) {
    for (var i = 0; i < polys.length; i++)
        cut_poly_steps(polys[i], depth, safe);
}


function pocket_poly(poly, depth, overlap) {
    if (typeof overlap == 'undefined') overlap = 0.4;

    var steps = Math.ceil(depth / maxPass);
    var pass = depth / steps;

    for (var step = 1; step <= steps; step++) {
        var pocket = offset([poly], -toolRadius);

        rapid(pocket[0][0][0], pocket[0][0][1]);

        while (pocket.length) {
            cut_polys(pocket, pass * step, false);
            pocket = offset(pocket, -toolDia + overlap);
        }
    }

    rapid({z: 5});
}


function bottom(poly, depth, wall) {
    // Inside
    pocket_poly(poly, depth - 3);

    // Rim
    cut_polys_steps(offset([poly], -toolRadius + wall / 2 + 0.0625),
                    25.4 / 4 - 1);

    // Cutout
    cut_polys_steps(offset([poly], toolRadius + wall), depth + 0.25);
}


function top(poly, depth, wall) {
    // Inside
    pocket_poly(poly, depth - 2.5);

    // Rim
    cut_polys_steps(offset([poly], toolRadius + wall / 2 - 0.0625),
                    depth - 2.5);

    // Divit
    var divit = arc2D(Math.PI, 2 * Math.PI);
    var divitWidth = 10;
    var x = width / 2;
    var y = -4;
    var z = -depth + 2.5;
    rapid(x - divitWidth / 2, y);

    for (var i = 0; i < divit.length; i++)
        cut({x: x + divitWidth / 2 * divit[i][0], z: z - divit[i][1]});

    rapid({z: 5});

    // Cutout
    cut_polys_steps(offset([poly], toolRadius + wall), depth + 0.25);
}


units(METRIC);
feed(400);
speed(4000);

var wall = 3;
var toolDia = 25.4 / 8;
var toolRadius = toolDia / 2;
var maxPass = 5;
tool_set({number: 1, units: IMPERIAL, length: 1, diameter: 1/8,
          shape: CYLINDRICAL});
tool(1);
rapid({z: 5});
translate(5, 5);

var poly = shape(width, length, side, toolDia);
poly = offset([poly], 1)[0]; // Allow for room

top(poly, 25.4 / 4, wall);
//bottom(poly, 25.4 / 2, wall);
