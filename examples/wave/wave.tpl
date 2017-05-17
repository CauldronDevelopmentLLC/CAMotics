speed(10000);
tool(3);

var maxFeed = 500;
var minFeed = 250;
var maxDepth = -2.5;
var minDepth = 0; 
var width = 96;
var height = 96; 
var zSafe = 3;
var scale = 2;
var period = 0.25; 

rapid({z: zSafe});
rapid(0, 0);


function depth(x, y) {
  x *= period * scale;
  y *= period * scale;
  var z = Math.sin(x + 0.3 * y) - Math.cos(y + 0.3 * x); // Wave
  z = z / 4 + 0.5; // Normalize
  return (maxDepth - minDepth) * z + minDepth; // Scale
}


function do_cut(x, y) {
  var z = depth(x, y);
  feed((1 - ((z - minDepth) / maxDepth)) * (maxFeed - minFeed) + minFeed);
  cut(x * scale, y * scale, z);
}


width /= scale;
height /= scale;

for (var col = 0; col < height; col++)
  for (var row = 0; row < width; row++)
    do_cut((col & 1) ? width - row - 1 : row, col);

for (var row = 0; row < width; row++)
  for (var col = 0; col < height; col++)
    do_cut(row, (row & 1) ? col : height - col - 1);

rapid({z: zSafe});