var w = 100;
var h = 135;
var b = 2;

var dxf = require('dxf');
var border = dxf.open('border.dxf')[0];
var config = {zSafe: 0, zStart: 0, zEnd: 0, zMax: 0};

gcode('la_peinture.gc');

feed(100);
speed(255);

for (var i = 0; i < 2; i++)
  dxf.cut_layer(border, config);