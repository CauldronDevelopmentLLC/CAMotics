var res = 100.0;
var zSafe = 5;
var zCut = -3.25;

units(METRIC);
feed(400);
speed(1000);
rapid({z: zSafe});

function gear(teeth, depth, diameter, peak, trough, hole) {
  // Hole
  rapid(-hole / 2, 0);
  cut({z: zCut});
  arc({x: hole / 2, angle: Math.PI * 2});
  rapid({z: zSafe});

  // Teeth
  function tooth(angle) {
    var r = Math.cos(angle * teeth);

    // Clamp
    r = Math.max(Math.min(r, peak), trough);

    return r * depth / 2;
  }


  for (var i = 0; i < teeth * res; i++) {
    var angle = Math.PI * 2 / teeth / res * i;
    var r = diameter / 2.0 + tooth(angle);
    var x = Math.cos(angle) * r;
    var y = Math.sin(angle) * r;

    if (!i) {
      rapid(x, y);
      cut({z: zCut});

    } else cut(x, y);
  }

  rapid({z: zSafe});
}

gear(24, 10, 100, 0.8, -0.2, 6);
translate(85, 25);
gear(12, 10, 50, 0.8, -0.2, 4);
translate(-10, -50);
gear(8, 10, 30, 0.8, -0.2, 4);
