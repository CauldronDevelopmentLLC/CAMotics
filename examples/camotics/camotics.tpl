var dxf = require('dxf');

var layer = dxf.open('camotics.dxf')[0];
var zSafe = 3;
var cutDepth = -1.5;

units(METRIC); // This must match the units of the DXF file
feed(40);

dxf.layer_cut(layer, zSafe, cutDepth);

while (true) {
  print("hello\n");
}