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


module.exports = {
  // Vertices ******************************************************************
  line_vertices: function(l) {
    return [l.start, l.end];
  },


  angle_vertex: function(center, radius, angle) {
    return {
      x: center.x + radius * Math.cos(angle * Math.PI / 180),
      y: center.y + radius * Math.sin(angle * Math.PI / 180),
    }
  },


  arc_vertices: function(a) {
    var angle = (a.endAngle - a.startAngle) * (a.clockwise ? 1 : -1);
    if (angle <= 0) angle += 360;
    var steps = Math.ceil(Math.abs(angle) / 360 * 100);
    var delta = angle / steps;

    var v = [];
    for (var i = 0; i <= steps; i++)
      v.push(this.angle_vertex(a.center, a.radius, a.startAngle + delta * i));

    return v;
  },


  polyline_vertices: function(pl) {
    return pl.vertices;
  },


  element_vertices: function(e) {
    switch (e.type) {
    case POINT:    return [e];
    case LINE:     return this.line_vertices(e);
    case ARC:      return this.arc_vertices(e);
    case POLYLINE: return this.polyline_vertices(e);
    default: throw 'Unsupported DXF element type ' + e.type;
    }
  },


  // Flip **********************************************************************
  line_flip: function(l) {
    return {
      type: LINE,
      start: l.end,
      end: l.start
    }
  },


  arc_flip: function(a) {
    return {
      type: ARC,
      center: a.center,
      radius: a.radius,
      startAngle: a.endAngle,
      endAngle: a.startAngle,
      clockwise: !a.clockwise
    }
  },


  polyline_flip: function(pl) {
    return {
      type: POLYLINE,
      vertices: [].concat(pl.vertices).reverse()
    }
  },


  element_flip: function(e) {
    switch (e.type) {
    case POINT:    return e;
    case LINE:     return this.line_flip(e);
    case ARC:      return this.arc_flip(e);
    case POLYLINE: return this.polyline_flip(e);
    default: throw 'Unsupported DXF element type ' + e.type;
    }
  },


  // CAM ***********************************************************************
  tabs: {},


  tabbed_cut: function(x, y) {
    /*
      var p = position();

      if (p.z < tabs.depth + tabs.height) {
      if (tabs.xAxis)
      for (var i = 0; i < tabs.xAxis; i++)
      if (x <= tabs.xAxis[i]) ;
      }
    */

    cut(x, y);
  },


  find_closest: function(p, layer) {
    // Dumb linear search
    var best = 0;
    var dist = Number.POSITIVE_INFINITY;
    var flip = false;

    for (var i = 0; i < layer.length; i++) {
      var v = this.element_vertices(layer[i]);
      var d1 = distance2D(p, first(v));
      var d2 = distance2D(p, last(v));
      var d = Math.min(d1, d2);

      if (d < dist) {
        dist = d;
        best = i;
        flip = d2 < d1;
      }
    }

    var v = this.element_vertices(layer[best]);
    if (flip) v = [].concat(v).reverse();

    return {i: best, flip: flip};
  },


  line_cut: function(l) {
    this.tabbed_cut(l.end.x, l.end.y);
  },


  arc_angle: function(a) {
    var angle = (a.endAngle - a.startAngle) * (a.clockwise ? 1 : -1)

    if (angle <= 0) angle += 360;

    return angle * Math.PI / 180;
  },


  arc_cut: function(a) {
    var p = position();

    var offset = {
      x: a.center.x - p.x,
      y: a.center.y - p.y,
    }

    arc({x: offset.x, y: offset.y, angle: arc_angle(a)});
  },


  polyline_cut: function(pl) {
    for (var i = 1; i < pl.vertices.length; i++)
      this.tabbed_cut(pl.vertices[i].x, pl.vertices[i].y);
  },


  element_cut: function(e) {
    switch (e.type) {
    case POINT:    return;
    case LINE:     return line_cut(e);
    case ARC:      return arc_cut(e);
    case POLYLINE: return polyline_cut(e);
    default: throw 'Unsupported DXF element type ' + e.type;
    }
  },


  layer_cut: function(layer, zSafe, zCut) {
    if (!layer.length) return;
    layer = [].concat(layer); // Copy layer

    while (layer.length) {
      var p = position();
      var match = this.find_closest(p, layer);
      var e = layer[match.i];

      if (match.flip) e = element_flip(e);

      var v = first(this.element_vertices(e));
      var d = distance2D(v, p);

      if (0.01 < d) {
        rapid({z: zSafe});
        rapid(v.x, v.y);
      }

      cut({z: zCut});
      this.tabbed_cut(v.x, v.y);

      this.element_cut(e);

      layer.splice(match.i, 1);
    }
  },


  // Polygons ******************************************************************
  layer_to_polys: function(layer) {
    if (!layer.length) return [];
    layer = [].concat(layer); // Copy layer

    var polys = [];
    var poly = [];
    var p = position();

    while (layer.length) {
      var match = this.find_closest(p, layer);
      var e = layer[match.i];
      var v = [].concat(this.element_vertices(e));
      if (match.flip) v.reverse();

      var d = distance2D(first(v), p);

      if (0.01 < d) {
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


  cut_polys: function(polys, zSafe, zCut) {
    if (!polys.length) return;

    for (var i = 0; i < polys.length; i++) {
      var poly = polys[i];
      var p = position();
      var v = first(poly);
      var d = distance2D({x: v[0], y: v[1]}, p);

      if (0.01 < d) {
        rapid({z: zSafe});
        rapid(v[0], v[1]);
      }

      cut({z: zCut});

      for (var j = 0; j < poly.length; j++)
        this.tabbed_cut(poly[j][0], poly[j][1]);

      this.tabbed_cut(v[0], v[1]);
    }
  },


  // Print *********************************************************************
  point_print: function(p) {
    print('(', dtoa(p.x), ', ', dtoa(p.y), ', ', dtoa(p.z), ')');
  },


  line_print: function(l) {
    this.point_print(l.start);
    print(' -> ');
    this.point_print(l.end);
  },


  arc_print: function(a) {
    this.point_print(a.center);
    print('     ', dtoa(a.radius), ', ',
          dtoa(a.startAngle), ', ',
          dtoa(a.endAngle));
  },


  vertices_print: function(v) {
    for (var i = 0; i < v.length; i++) {
      if (i) print(' -> ');
      this.point_print(v[i]);
    }
  },


  polyline_print: function(p) {
    this.vertices_print(p.vertices);
  },


  spline_print: function(s) {
    print('(');
    this.vertices_print(s.ctrlPts);
    print('),(');
    this.vertices_print(s.knots);
    print(')');
  },


  element_print: function(e) {
    switch (e.type) {
    case POINT: this.point_print(e); break;
    case LINE: this.line_print(e); break;
    case ARC: this.arc_print(e); break;
    case POLYLINE: this.polyline_print(e); break;
    case SPLINE: this.spline_print(e); break;
    default: throw 'Unknown DXF layer element type ' + e.type;
    }
  },


  element_label: function(e) {
    switch (e.type) {
    case POINT:    return 'POINT';
    case LINE:     return 'LINE';
    case ARC:      return 'ARC';
    case POLYLINE: return 'POLYLINE';
    case SPLINE:   return 'SPLINE';
    default: throw 'Unknown DXF layer element type ' + e.type;
    }
  },


  layer_print: function(p, prefix) {
    for (var i = 0; i < p.length; i++) {
      if (prefix) print(prefix);
      print(lpad(this.element_label(p[i]), 8), ' ');
      this.element_print(p[i]);
      print('\n');
    }
  },


  offset_polys: function(polys, delta) {
    var result = [];

    for (var i = 0; i < polys.length; i++)
      result = result.concat(offset([polys[i]], delta));

    return result;
  },


  open: open,


  set_tabs: function (data) {tabs = data},


  cut_layer_offset: function (layer, delta, zSafe, zCut, steps) {
    if (typeof steps == 'undefined') steps = 1;

    var zDelta = zCut / steps;

    for (var i = 0; i < steps; i++) {
      var polys = this.layer_to_polys(layer);
      if (delta) polys = this.offset_polys(polys, delta);
      this.cut_polys(polys, zSafe, zDelta * (i + 1));
    }
  }
}
