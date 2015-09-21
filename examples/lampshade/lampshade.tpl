var dxf = require('dxf');

var top = dxf.open('top.dxf');
var side = dxf.open('side.dxf');

var zSafe = 5;              // Safe height
var cutDepth = -1/4 * 25.4; // Total metric cut depth
var maxZStep = 3;           // Max depth of cut per pass
var toolDia = 1/16 * 25.4;  // Metric diameter of the tool
var offset = [145, 175];

// Compute steps and step down
var steps = Math.abs(Math.ceil(cutDepth / maxZStep));
var zStepDown = cutDepth / steps;

units(METRIC); // This must match the units of the DXF file
feed(40);      // Depends on what the machine and the material can handle


function cut_side() {
  for (var i = 0; i < steps; i++)
    dxf.layer_cut(side.inner, zSafe, zStepDown * (i + 1));

  for (var i = 0; i < steps; i++)
    dxf.cut_layer_offset(side.outer, toolDia / 2, zSafe, zStepDown * (i + 1));
}


function cut_top() {
  for (var i = 0; i < steps; i++)
    dxf.layer_cut(top.inner, zSafe, zStepDown * (i + 1));

  for (var i = 0; i < steps; i++)
    dxf.cut_layer_offset(top.outer, toolDia / 2, zSafe, zStepDown * (i + 1));
}


// Cut sides
for (var row = 0; row < 2; row++)
  for (var col = 0; col < 2; col++) {
    pushMatrix();
    translate(row * offset[0], col * offset[1]);
    cut_side();
    popMatrix();
  }


// Cut top
translate(0, 2 * offset[1]);
cut_top();
