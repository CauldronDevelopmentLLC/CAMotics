function extend(defaults, options) {
  var extended = {};
  var prop;

  for (prop in defaults)
    if (Object.prototype.hasOwnProperty.call(defaults, prop))
      extended[prop] = defaults[prop];

  for (prop in options)
    if (Object.prototype.hasOwnProperty.call(options, prop))
      extended[prop] = options[prop];

  return extended;
}


function lpad(s, width, delim) {
  if (typeof delim == 'undefined') delim = ' ';
  while (s.length < width) s = delim + s;
  return s;
}


function rpad(s, width, delim) {
  if (typeof delim == 'undefined') delim = ' ';
  while (s.length < width) s += delim;
  return s;
}


function dtoa(n, width) {
  return lpad(n.toFixed(2), typeof width == 'undefined' ? 6 : width);
}


function str(x) {
  return (typeof x == 'string') ? x : JSON.stringify(x);
}

function sqr(x) {
  return x * x;
}


function first(a) {
  return a[0];
}


function last(a) {
  return a[a.length - 1];
}


function distance2D(p1, p2) {
  return Math.sqrt(sqr(p2.x - p1.x) + sqr(p2.y - p1.y))
}


// Vertices ********************************************************************
function line_vertices(l) {
  return [l.start, l.end];
}


function angle_vertex(center, radius, angle) {
  return {
    x: center.x + radius * Math.cos(angle * Math.PI / 180),
    y: center.y + radius * Math.sin(angle * Math.PI / 180),
  }
}


function arc_vertices(a) {
  var angle = (a.endAngle - a.startAngle) * (a.clockwise ? 1 : -1);
  if (angle <= 0) angle += 360;
  var steps = Math.ceil(Math.abs(angle) / 360 * 100);
  var delta = angle / steps;

  var v = [];
  for (var i = 0; i <= steps; i++)
    v.push(angle_vertex(a.center, a.radius, a.startAngle + delta * i));

  return v;
}


function polyline_vertices(pl) {
  return pl.vertices;
}


function element_vertices(e) {
  switch (e.type) {
  case dxf.POINT:    return [e];
  case dxf.LINE:     return line_vertices(e);
  case dxf.ARC:      return arc_vertices(e);
  case dxf.POLYLINE: return polyline_vertices(e);
  default: throw 'Unsupported DXF element type ' + e.type;
  }
}


// CAM *************************************************************************
function find_closest(p, layer) {
  // Dumb linear search
  var best = 0;
  var dist = Number.POSITIVE_INFINITY;
  var flip = false;

  for (var i = 0; i < layer.length; i++) {
    var v = element_vertices(layer[i]);
    var d1 = distance2D(p, first(v));
    var d2 = distance2D(p, last(v));
    var d = Math.min(d1, d2);

    if (d < dist) {
      dist = d;
      best = i;
      flip = d2 < d1;
    }
  }

  var v = element_vertices(layer[best]);
  if (flip) v = [].concat(v).reverse();

  return {i: best, flip: flip};
}



function drill(p, zSafe, depth) {
  rapid({z: zSafe});
  rapid(p.x, p.y);
  cut({z: depth});
  rapid({z: zSafe});
}


// Polygons ********************************************************************
function z_ramp_cut(x, y, z) {
  var slope = 0.125;
  var ramping = z < position().z;

  if (ramping) {
    var len = distance2D({x: x, y: y}, position());
    z = Math.max(position().z - slope * len, z);
  }

  cut(x, y, z);

  return ramping;
}


function reverse_polys(polys) {
  if (!polys.length) return;

  var result = [];

  for (var i = 0; i < polys.length; i++)
    result.push(polys[i].reverse());

  return result;
}


// Print ***********************************************************************
function point_print(p) {
  print('(', dtoa(p.x), ', ', dtoa(p.y), ', ', dtoa(p.z), ')');
}


function line_print(l) {
  point_print(l.start);
  print(' -> ');
  point_print(l.end);
}


function arc_print(a) {
  point_print(a.center);
  print('     ', dtoa(a.radius), ', ',
        dtoa(a.startAngle), ', ',
        dtoa(a.endAngle));
}


function vertices_print(v) {
  for (var i = 0; i < v.length; i++) {
    if (i) print(' -> ');
    point_print(v[i]);
  }
}


function polyline_print(p) {
  vertices_print(p.vertices);
}


function spline_print(s) {
  print('(');
  vertices_print(s.ctrlPts);
  print('),(');
  vertices_print(s.knots);
  print(')');
}


function element_print(e) {
  switch (e.type) {
  case dxf.POINT: point_print(e); break;
  case dxf.LINE: line_print(e); break;
  case dxf.ARC: arc_print(e); break;
  case dxf.POLYLINE: polyline_print(e); break;
  case dxf.SPLINE: spline_print(e); break;
  default: throw 'Unknown DXF layer element type ' + e.type;
  }
}


function element_label(e) {
  switch (e.type) {
  case dxf.POINT:    return 'POINT';
  case dxf.LINE:     return 'LINE';
  case dxf.ARC:      return 'ARC';
  case dxf.POLYLINE: return 'POLYLINE';
  case dxf.SPLINE:   return 'SPLINE';
  default: throw 'Unknown DXF layer element type ' + e.type;
  }
}


module.exports = extend(dxf, {
  layer_print: function(p, prefix) {
    for (var i = 0; i < p.length; i++) {
      if (prefix) print(prefix);
      print(lpad(element_label(p[i]), 8), ' ');
      element_print(p[i]);
      print('\n');
    }
  },


  drill_layer: function (layer, zSafe, depth) {
    for (var i = 0; i < layer.length; i++) {
      var e = layer[i];

      switch (e.type) {
      case dxf.POINT:    drill(e, zSafe, depth);
      case dxf.ARC:      drill(e.center, zSafe, depth);
      case dxf.LINE:
      case dxf.POLYLINE: break;
      default: throw 'Unsupported DXF element type ' + e.type;
      }
    }
  },


  layer_to_polys: function (layer) {
    if (!layer.length) return [];
    layer = [].concat(layer); // Copy layer

    var polys = [];
    var poly = [];
    var p = position();

    while (layer.length) {
      var match = find_closest(p, layer);
      var e = layer[match.i];
      var v = [].concat(element_vertices(e));
      if (match.flip) v.reverse();

      var d = distance2D(first(v), p);

      if (0.01 < d || !polys.length) {
        poly = [];
        polys.push(poly);
      }

      for (var i = 0; i < v.length; i++)
        poly.push([v[i].x, v[i].y]);

      p = last(v);

      layer.splice(match.i, 1);
    }

    return polys;
  },


  offset_polys: function (polys, delta) {
    var result = [];

    for (var i = 0; i < polys.length; i++)
      result = result.concat(offset([polys[i]], delta));

    return result;
  },


  cut_polys: function(polys, zSafe, zCut, steps, ramp) {
    if (typeof steps == 'undefined') steps = 1;
    if (typeof ramp == 'undefined') ramp = 1;
    if (!polys.length) return;

    var zDelta = zCut / steps;

    for (var i = 0; i < polys.length; i++) {
      for (var j = 0; j < steps; j++) {
        var z = zDelta * (j + 1);

        var poly = polys[i].slice(0);
        var v = first(poly);
        var d = distance2D({x: v[0], y: v[1]}, position());

        poly.push(first(poly));

        if (0.01 < d) {
          rapid({z: zSafe});
          rapid(v[0], v[1]);
        }

        // Assuming surface is at zero
        if (zDelta * j < position().z) cut({z: zDelta * j});

        while (poly.length) {
          var p = poly.shift();
          if (z_ramp_cut(p[0], p[1], z)) poly.push(p);
        }
      }
    }
  },


  cut_layer_offset: function (layer, delta, zSafe, zCut, steps) {
    var polys = this.layer_to_polys(layer);
    polys = this.offset_polys(polys, delta);
    this.cut_polys(polys, zSafe, zCut, steps);
  },


  pocket_layer_offset: function (layer, islands, delta, stepOver, zSafe, zCut,
                                 steps, maxPasses) {
    if (typeof steps == 'undefined') steps = 1;

    var polys = this.layer_to_polys(layer);

    if (false && typeof islands != 'undefined') {
      islands = this.layer_to_polys(islands);
      islands = reverse_polys(islands);
      polys = polys.concat(islands);
    }

    polys = this.offset_polys(polys, delta);

    var zDelta = zCut / steps;

    cut({z: 0}); // Assuming surface is at 0

    for (var i = 0; i < steps; i++) {
      var passes = maxPasses;
      var oPolys = polys;

      while (oPolys.length) {
        this.cut_polys(oPolys, zSafe, zDelta * (i + 1), 1);
        if (typeof passes != 'undefined' && --passes <= 0) break;

        var u = first(first(oPolys));
        var z = position().z;

        oPolys = this.offset_polys(oPolys, -stepOver);
        if (oPolys.length) {
          var v = first(first(oPolys));

          rapid({z: zSafe});
          rapid(u[0], u[1]);
          cut({z: z});
          cut(v[0], v[1]);
        }
      }
    }
  }
})
