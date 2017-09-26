var mode = 'inside';


function circle(depth, width, height, startAngle, angle, cX, cY, offset,
                divisions) {
    if (typeof width == 'undefined') width = 1;
    if (typeof height == 'undefined') height = 1;
    if (typeof startAngle == 'undefined') startAngle = 0;
    if (typeof angle == 'undefined') angle = Math.PI * 2;
    if (typeof cX == 'undefined') cX = 0;
    if (typeof cY == 'undefined') cY = 0;
    if (typeof offset == 'undefined') offset = 0;
    if (typeof divisions == 'undefined') divisions = 180;

    var step = Math.PI * 2 / divisions;
    var count = angle / step;
    var delta = angle / count;

    rapid(cX + width / 2 * (1 - Math.cos(startAngle)) - width / 2,
          cY + height / 2 * Math.sin(startAngle));
    cut({z: depth});

    for (var i = 1; i < count; i++) {
        var t = startAngle + i * delta;
        var x = width / 2 * (1 - Math.cos(t)) - width / 2;
        var y = height / 2 * Math.sin(t);
        if (i % 2) {x *= 1 + offset; y *= 1 + offset;}
        else  {x *= 1 - offset; y *= 1 - offset;}
        cut(cX + x, cY + y);
    }

    var endAngle = startAngle + angle;
    cut(cX + width / 2 * (1 - Math.cos(endAngle)) - width / 2,
        cY + height / 2 * Math.sin(endAngle));
}


function round(width, height, startDepth, endDepth) {
    var depth = startDepth;
    var step = toolDia - 0.5;
    var rings = height / step / 2 - 1;
    var depthDelta = (endDepth - startDepth) / rings;

    for (var i = 0; i < rings; i++) {
        circle(depth, width, height);

        height -= 2 * step;
        width -= 2 * step / height * width;

        depth += depthDelta;
        rapid({z: 1});
    }
}


function tool_change(t) {
    if (t == tool()) return;

    rapid({z: 25});
    tool(t);
}


var safe = 5;
var depth = 1 / 4 * 25.4;
var toolDia = 1 / 8 * 25.4;
var toolRad = toolDia / 2;
var width = 40;
var height = 60;
var wall = 1;
var wallGap = 0.075;
var eyeY = height / 2 + 0.5;


// Guides
function guides() {
    rapid(0, height / 2 + 10);
    cut({z: -depth - 3});
    rapid({z: safe});

    rapid(0, -(height / 2 + 10));
    cut({z: -depth - 3});
    rapid({z: safe});
}


function eye(depth, dia) {
    circle(depth, dia, dia, 0, Math.PI * 2 * 32 / 64, 0, eyeY, 0, 32);
}


function eye_clean(depth) {
    var dia = toolDia;
    circle(depth, dia, dia, 0, Math.PI * 2, 0, eyeY, 0, 32);
    dia = toolDia * 2 - 0.5
    circle(depth, dia, dia, 0, Math.PI * 2, 0, eyeY, 0, 32);
}


// Outside
function outside(depth) {
    var w = width + toolDia;
    var h = height + toolDia;
    circle(depth, w, h, Math.PI * 2 * 18.75 / 64, Math.PI * 2 * 58.5 / 64);

    // Eye
    eye(depth, 12);
    rapid({z: safe});
}

function inside(depth, offset) {
    var w = width - offset * 2 - toolDia;
    var h = height - offset * 2 - toolDia;
    var stepIn = toolDia - 0.5;

    while (toolDia < w) {
        cut(-w / 2, 0);
        circle(depth, w, h);

        w -= stepIn * 2;
        h -= stepIn * 2;
    }

    cut(-w / 2, 0);
    circle(depth, w, h);

    rapid({z: safe});
}

// Rim
function rim(depth) {
    var w = width - wall * 2 + toolDia - wallGap;
    var h = height - wall * 2 + toolDia - wallGap;

    circle(depth, w, h);
    rapid({z: safe});
}


// Bottom
function bottom(depth, odepth) {
    pushMatrix();

    // Side 0
    translate(-(width / 2 + 5));
    rapid(0, 0);

    outside(odepth);
    inside(depth, wall);

    // Side 1
    translate(width + 10);
    rapid(0, 0);

    outside(odepth);

    // Clean eye
    eye(depth, toolDia);
    eye(depth, toolDia * 2 - 0.5);
    rapid({z: safe});

    rim(depth);
    inside(depth, 2 * wall + wallGap);

    popMatrix();
}


// Top
function cutout() {
    tool_change(1);
    pushMatrix();

    // Side 0
    translate(-(width / 2 + 5));
    rapid(0, 0);

    outside(-4);

    // Side 1
    translate(width + 10);
    rapid(0, 0);

    outside(-4);

    popMatrix();
}


function holes() {
    tool_change(1);
    rapid(-(width / 2 + 5), height / 2 + 1);
    cut({z: -depth - 1});
    rapid({z: safe});

    rapid(width / 2 + 5, height / 2 + 1);
    cut({z: -depth - 1});
    rapid({z: safe});
}


function patterns() {
    pushMatrix();

    // Side 0
    translate(-(width / 2 + 5));
    rapid(0, 0);

    tool_change(1);
    round(width - toolDia, height - toolDia, -3, 0);
    eye_clean(-3);
    rapid({z: safe});

    // Side 1
    translate(width + 10);
    rapid(0, 0);

    round(width - toolDia, height - toolDia, -3, 0);
    eye_clean(-3);
    rapid({z: safe});

if (false) {
    tool_change(2);
    var w = width - 5;
    var h = height - 5;
    var depth = -3;
    var step = 4;
    while (2 < w && 2 < h) {
        circle(depth, w, h, 0, Math.PI * 2, 0, 0, 0.04, 64);
        rapid({z: safe});

        w -= step;
        h -= step * h / w;
        depth *= 0.9;
    }
    rapid({z: safe});
}

    popMatrix();
}


function top() {
    pushMatrix();
    patterns();
    holes();
    cutout();
    popMatrix();
}


// Init
rapid({z: safe});
tool_change(1);
feed(400);

if (mode == 'inside') {
    rapid(0.1, 0.1);
    guides();
    bottom(-2, -3);

} else if (mode == 'outside') top();

else if (mode == 'engrave') {
    speed(100);
    feed(40 * 25.4);

    var dxf = require('dxf');
    var sh = dxf.open('seahorse.dxf');

    translate(-44, -29);
    dxf.cut_layer_offset(sh[0]);

    var jf = dxf.open('jellyfish.dxf');

    translate(49, -1);
    dxf.cut_layer_offset(jf[0]);
}

tool_change(1);
