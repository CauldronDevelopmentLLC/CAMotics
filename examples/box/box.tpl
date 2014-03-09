size = 1;

wiggle = 0.075;
wall = 2;
toolDia = 25.4 / 8;
overlap = 0.5;
safe = 1;
depth = 13;

toolRadius = toolDia / 2;
trim = 0; //(1.25 * wall) * size;
radius = 25 - trim;
lip = (depth - wall - trim) * 0.36;


function spiral(radius, depth, passes, start) {
  rapid(-radius);
  rapid({z: -start + 1});
  cut({z: -start});
  arc({x: radius, z: -(depth - start), angle: Math.PI * 2 * passes});
  arc({x: radius, angle: Math.PI * 2});
  rapid({z: safe});
}


function pocket(radius, depth, passes) {
  var step = toolDia - overlap;
  var nSteps = (radius - toolDia) / step;

  for (var i = 0; i < nSteps; i++)
    spiral(toolRadius - overlap + i * step, depth, passes, 0);

  spiral(radius - toolRadius, depth, passes, 0);
}


function half(top) {
  // Inside
  pocket(radius - wall, depth - wall, 6);

  // Trim
  if (trim) {
    var tRad = radius - (wall / 2);
    spiral(tRad, trim, Math.ceil(trim / 3), 0);
  }

  // Lip
  var lRad = radius - (wall / 2);
  if (top) lRad -= toolRadius - wiggle;
  else lRad += toolRadius - wiggle;
  var lipDepth = lip + trim;
  if (top) lipDepth += 2 * wiggle;
  spiral(lRad, lipDepth, 3, trim);

  // Outside
  spiral(radius + toolRadius, depth, 6, 0);
}


feed(600);
rapid({z: safe});

rapid(radius + toolRadius, radius + toolRadius);
half(true);
rapid({y: (radius + toolRadius) * 3});
half(false);

rapid({z: 5});
