var dxf = require('dxf');

var layer = dxf.open('stretching_cat.dxf')[0];
var zSafe = 5;             // Safe height
var cutDepth = -10.4;      // Total cut depth
var maxZStep = 3;          // Max depth of cut per pass
var toolDia = 1/16 * 25.4; // Metric diameter of the tool
var offset = 50;           // Array X & Y offset
var rows = 2;
var cols = 4;

// Compute steps and step down
var steps = Math.abs(Math.ceil(cutDepth / maxZStep));
var zStepDown = cutDepth / steps;

units(METRIC); // This must match the units of the DXF file
feed(40);      // Depends on what the machine and the material can handle

translate(3, 3); // Offset a little
scale(0.5, 0.5); // Quarter size

// Cut an array
for (var row = 0; row < rows; row++)
  for (var col = 0; col < cols; col++) {
    pushMatrix();
    translate(row * offset, col * offset);

    for (var i = 0; i < steps; i++)
      dxf.layer_cut(layer, zSafe, zStepDown * (i + 1));

    popMatrix();
  }
