var dxf = require('dxf');
var layers = dxf.open('buildbotics_logo.dxf');

var zSafe = 3;
var cutDepth = -1.5;

units(METRIC); // This must match the units of the DXF file
feed(1600);
speed(10000);
tool(2);

rapid({z: zSafe});

dxf.cut_layer_offset(layers.Inside, -0.5, zSafe, cutDepth);
dxf.cut_layer_offset(layers.Tools,   0.5, zSafe, cutDepth);
dxf.cut_layer_offset(layers.Head,    0.5, zSafe, cutDepth);
dxf.cut_layer_offset(layers.Ears,   -0.5, zSafe, cutDepth);
dxf.cut_layer_offset(layers.Eyes,   -0.5, zSafe, cutDepth * 0.66);
dxf.cut_layer_offset(layers.Pupils, -1.2, zSafe, cutDepth * 0.66);
dxf.cut_layer_offset(layers.Outline,   0, zSafe, cutDepth);

rapid({z: zSafe});
speed(0);
rapid(40, 75);
