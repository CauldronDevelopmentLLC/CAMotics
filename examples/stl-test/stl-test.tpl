var stl = require('stl');

// http://www.thingiverse.com/download:15654
//var data = stl.open('bunny-flatfoot.stl');
var data = stl.open('V1.stl');

var bounds = stl.bounds({stl: data});
var start = bounds[1][2];
var end = bounds[0][2];
var steps = 100;

var contours = stl.contour({stl: data, start: start, end: end, steps: steps});

feed(400);

for (var i = 0; i < contours.length; i++) {
  var level = contours[i];
  rapid({z: level.Z});

  for (var j = 0; j < level.contours.length; j++) {
    var contour = level.contours[j];

    for (var k = 0; k < contour.length; k++) {
      if (k) cut({x: contour[k].X, y: contour[k].Y});
      else rapid({x: contour[k].X, y: contour[k].Y});
    }
  }
}
