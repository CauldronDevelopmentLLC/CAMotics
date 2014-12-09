function square(depth, safe) {
  rapid({z: safe});
  rapid(0, 0);

  cut({z: depth});

  cut(1, 0);
  cut(1, 1);
  cut(0, 1);
  cut(0, 0);

  rapid({z: safe});
}

var zsafe = 0.2;
var zcut = 0.1;

feed(400);

scale(2, 2);
translate(1, 1);
square(zcut, zsafe);

loadIdentity();
translate(1, 1);
scale(2, 2);
square(zcut, zsafe);
