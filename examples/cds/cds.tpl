units(METRIC);
feed(300);
speed(10000);
tool(1);

var zSafe = 3;
var zCut = 1.5;
var toolDia = 25.4 / 8;
var width = 50; // Circle and square dim
var dwidth = 35; // Diamond width


function diamond(dim) {
  icut({x: dim, y: -dim});
  icut({x: dim, y: dim});
  icut({x: -dim, y: dim});
  icut({x: -dim, y: -dim});
}


function square(dim) {
  icut({y: -dim});
  icut({x: 2 * dim});
  icut({y: 2 * dim});
  icut({x: -2 * dim});
  icut({y: -dim});
}


function triangle(dim) {
  icut({y: -dim});
  icut({x: dim});
  icut({x: -dim, y: dim});
}


function clear_triangle(dim) {
  for (var i = 0; i < 3; i++) {
    icut({x: 0.5 * toolDia, y: -1.5 * toolDia});
    triangle(dim - (i + 1) * 2 * toolDia);
  }

  icut({x: 1 * toolDia, y: -2 * toolDia});
}


function clear_wedge(dim) {
  icut({y: -0.5 * dim});
  icut({x: 0.5 * toolDia, y: -0.5 * toolDia});
  triangle(0.4 * dim);
  icut({x: 0.5 * toolDia, y: -0.5 * toolDia});
  triangle(0.4 * dim - toolDia);
  icut({x: 1 * toolDia, y: -2 * toolDia});
  cut({y: -dim});
  cut({x: 0});
}


rapid({z: zSafe});



// Diamond
var dim = Math.sqrt(2) * (dwidth + toolDia) / 2;
rapid({x: -dim, y: 0});
cut({z: -zCut});
diamond(dim);

// Square
var dim = (width + toolDia) / 2;
cut({x: -dim});
square(dim);

// Clear triangles
for (var i = 0; i < 4; i++) {
  clear_triangle(dim);
  cut({x: 0, y: -dim});
  rotate(Math.PI / 2);
}

// Circle
cut({z: -2 * zCut});
arc(dim, 0, 0, 2 * Math.PI);

// Square
square(dim);

// Clear wedges
for (var i = 0; i < 4; i++) {
  clear_wedge(dim);
  rotate(Math.PI / 2);
}

// Square
cut({z: -3 * zCut});
square(dim);

// Done
rapid({z: zSafe});
