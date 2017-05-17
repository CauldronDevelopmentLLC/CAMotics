var dxf = require('dxf');

var layer = dxf.open('stretching_cat.dxf')[0];
var zSafe = 5;             // Safe height
var cutDepth = -11.5;      // Total cut depth
var maxZStep = 3;          // Max depth of cut per pass
var offset = 100;          // Array X & Y offset

// Compute steps
var steps = Math.abs(Math.ceil(cutDepth / maxZStep));

units(METRIC); // This must match the units of the DXF file
feed(200);     // Depends on what the machine and the material can handle
speed(10000);
tool(1)

translate(3, 3); // Offset a little
scale(0.4, 0.4);

// Cut an array
for (var x = 0; x < 2; x++) {
  for (var y = 0; y < 4; y++) {
    dxf.cut_layer(layer, {
      delta: tool(1).diameter / 2 / 0.4,
      zSafe: zSafe,
      zStart: 0,
      zEnd: cutDepth,
      zMax: maxZStep,
      zSlope: 0.125,
      tabSlope: 0.8,
      tabHeight: 4.5,
      tabs: [{ratio: 0}, {ratio: 0.35}]
    });

    translate(0, x == 0 ? offset : -offset);
  }

  translate(offset, -offset);
}

rapid({z: zSafe});
