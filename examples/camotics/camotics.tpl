var dxf = require('dxf');

var layer = dxf.open('camotics.dxf')[0];
var zSafe = 3;
var cutDepth = -1.5;

units(METRIC); // This must match the units of the DXF file
tool(1);
feed(200);
speed(10000);
dwell(10);     // Wait for spindle to spin up

dxf.layer_cut(layer, zSafe, cutDepth);

rapid({z: zSafe});
speed(0);
rapid({x: 0, y: 0});