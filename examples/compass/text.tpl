units(METRIC)
feed(400);
speed(1000);

tool(1);
scale(0.32, 0.5);
rotate(Math.PI / 2);
translate(22.5, 5);
gcode('sworn_brothers_llc.ngc');
translate(15, 5);
gcode('mt_kazbegi_2013.ngc');

rapid({z: 5});
