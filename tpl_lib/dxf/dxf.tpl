/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

\******************************************************************************/

var _dxf = require('_dxf');


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


function cube(x) {
  return x * x * x;
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


function quad_bezier(p, t) {
  var c = [sqr(1 - t), 2 * (1 - t) * t, sqr(t)];

  return {
    x: c[0] * p[0].x + c[1] * p[1].x + c[2] * p[2].x,
    y: c[0] * p[0].y + c[1] * p[1].y + c[2] * p[2].y
  }
}


function quad_bezier_length(p) {
  var l = 0;
  var v = p[0];

  for (var i = 1; i <= 100; i++) {
    var u = quad_bezier(p, 0.01 * i);
    l += distance2D(v, u);
    v = u;
  }

  return l;
}


function cubic_bezier(p, t) {
  var c = [cube(1 - t), 3 * sqr(1 - t) * t, 3 * (1 - t) * sqr(t), cube(t)];

  return {
    x: c[0] * p[0].x + c[1] * p[1].x + c[2] * p[2].x + c[3] * p[3].x,
    y: c[0] * p[0].y + c[1] * p[1].y + c[2] * p[2].y + c[3] * p[3].y
  }
}


function cubic_bezier_length(p) {
  var l = 0;
  var v = p[0];

  for (var i = 1; i <= 100; i++) {
    var u = cubic_bezier(p, 0.01 * i);
    l += distance2D(v, u);
    v = u;
  }

  return l;
}


function extend(target, source) {
  for (var i in source)
    if (source.hasOwnProperty(i))
      target[i] = source[i];

  return target;
}


module.exports = extend({
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


  spline_vertices: function(s) {
    return s.ctrlPts;
  },


  element_vertices: function(e) {
    switch (e.type) {
    case _dxf.POINT:    return [e];
    case _dxf.LINE:     return this.line_vertices(e);
    case _dxf.ARC:      return this.arc_vertices(e);
    case _dxf.POLYLINE: return this.polyline_vertices(e);
    case _dxf.SPLINE:   return this.spline_vertices(e);
    default: throw 'Unsupported DXF element type ' + e.type;
    }
  },


  element_com: function(e) {
    var v = this.element_vertices(e);
    var c = {x: 0, y: 0};

    for (var i = 0; i < v.length; i++) {
      c.x += v[i].x;
      c.y += v[i].y;
    }

    c.x /= v.length;
    c.y /= v.length;

    return c;
  },


  // Flip **********************************************************************
  line_flip: function(l) {
    return {
      type: _dxf.LINE,
      start: l.end,
      end: l.start
    }
  },


  arc_flip: function(a) {
    return {
      type: _dxf.ARC,
      center: a.center,
      radius: a.radius,
      startAngle: a.endAngle,
      endAngle: a.startAngle,
      clockwise: !a.clockwise
    }
  },


  polyline_flip: function(pl) {
    return {
      type: _dxf.POLYLINE,
      vertices: [].concat(pl.vertices).reverse()
    }
  },


  spline_flip: function(s) {
    return {
      type: _dxf.SPLINE,
      degree: s.degree,
      ctrlPts: [].concat(s.ctrlPts).reverse(),
      knots: s.knots
    }
  },


  element_flip: function(e) {
    switch (e.type) {
    case _dxf.POINT:    return e;
    case _dxf.LINE:     return this.line_flip(e);
    case _dxf.ARC:      return this.arc_flip(e);
    case _dxf.POLYLINE: return this.polyline_flip(e);
    case _dxf.SPLINE:   return this.spline_flip(e);
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

    arc({x: offset.x, y: offset.y, angle: this.arc_angle(a)});
  },


  polyline_cut: function(pl) {
    for (var i = 1; i < pl.vertices.length; i++)
      this.tabbed_cut(pl.vertices[i].x, pl.vertices[i].y);

    if (pl.vertices.length)
      this.tabbed_cut(pl.vertices[0].x, pl.vertices[0].y);
  },


  spline_cut: function(s, res) {
    if (typeof res == 'undefined') res = units() == METRIC ? 1 : 1 / 25.4;

    if (s.degree == 2) {
      var steps = Math.ceil(quad_bezier_length(s.ctrlPts) / res);
      var delta = 1 / steps;

      for (var i = 0; i < steps; i++) {
        v = quad_bezier(s.ctrlPts, delta * (i + 1));
        this.tabbed_cut(v.x, v.y);
      }


    } else if (s.degree == 3) {
      var steps = Math.ceil(cubic_bezier_length(s.ctrlPts) / res);
      var delta = 1 / steps;

      for (var i = 0; i < steps; i++) {
        var v = cubic_bezier(s.ctrlPts, delta * (i + 1));
        this.tabbed_cut(v.x, v.y);
      }

    } else
      for (var i = 1; i < s.ctrlPts.length; i++)
        this.tabbed_cut(s.ctrlPts[i].x, s.ctrlPts[i].y);
  },


  element_cut: function(e, res) {
    switch (e.type) {
    case _dxf.POINT:    return;
    case _dxf.LINE:     return this.line_cut(e);
    case _dxf.ARC:      return this.arc_cut(e);
    case _dxf.POLYLINE: return this.polyline_cut(e);
    case _dxf.SPLINE:   return this.spline_cut(e, res);
    default: throw 'Unsupported DXF element type ' + e.type;
    }
  },


  layer_cut: function(layer, zSafe, zCut, res) {
    if (!layer.length) return;
    layer = [].concat(layer); // Copy layer

    while (layer.length) {
      var p = position();
      var match = this.find_closest(p, layer);
      var e = layer[match.i];

      if (match.flip) e = this.element_flip(e);

      var v = first(this.element_vertices(e));
      var d = distance2D(v, p);

      if (0.01 < d) {
        rapid({z: zSafe});
        rapid(v.x, v.y);
      }

      cut({z: zCut});
      this.tabbed_cut(v.x, v.y);

      this.element_cut(e, res);

      layer.splice(match.i, 1);
    }
  },


  layer_cut_step: function(layer, zSafe, zCut, maxZStep, res) {
    // Compute steps and step down
    var steps = Math.ceil(Math.abs(zCut / maxZStep));
    var zStepDown = zCut / steps;

    // Do steps
    for (var i = 0; i < steps; i++)
      this.layer_cut(layer, zSafe, zStepDown * (i + 1), res);
  },


  // Polygons ******************************************************************
  poly_com: function(poly) {
    // TODO broken?
    var c = {x: 0, y: 0};

    for (var i = 0; i < poly.length; i++) {
      c.x += poly[i][0];
      c.y += poly[i][1];
    }

    c.x /= poly.length;
    c.y /= poly.length;

    return c;
  },


  polys_com: function(polys) {
    // TODO broken?
    var c = {x: 0, y: 0};

    for (var i = 0; i < polys.length; i++) {
      if (!polys[i].length) continue;

      var pc = this.poly_com(polys[i]);
      c.x += pc[0];
      c.y += pc[1];
    }

    c.x /= polys.length;
    c.y /= polys.length;

    return c;
  },


  layer_com: function(layer) {
    var polys = this.layer_to_polys(layer);
    return this.polys_com(polys);
  },


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


  cut_polys: function(polys, zSafe, zCut, zFeed) {
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

      var f = feed()[0];
      if (typeof zFeed != 'undefined') feed(zFeed);
      cut({z: zCut});
      feed(f);

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
    print('     ', dtoa(a.radius), ', ', dtoa(a.startAngle), ', ',
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
    print(dtoa(s.degree) + ': (');
    this.vertices_print(s.ctrlPts);
    print('),(');
    for (var i = 0; i < s.knots.length; i++) {
      if (i) print(', ');
      print(dtoa(s.knots[i]));
    }
    print(')');
  },


  element_print: function(e) {
    switch (e.type) {
    case _dxf.POINT: this.point_print(e); break;
    case _dxf.LINE: this.line_print(e); break;
    case _dxf.ARC: this.arc_print(e); break;
    case _dxf.POLYLINE: this.polyline_print(e); break;
    case _dxf.SPLINE: this.spline_print(e); break;
    default: throw 'Unknown DXF layer element type ' + e.type;
    }
  },


  element_label: function(e) {
    switch (e.type) {
    case _dxf.POINT:    return 'POINT';
    case _dxf.LINE:     return 'LINE';
    case _dxf.ARC:      return 'ARC';
    case _dxf.POLYLINE: return 'POLYLINE';
    case _dxf.SPLINE:   return 'SPLINE';
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


  set_tabs: function (data) {tabs = data},


  cut_layer_offset: function (layer, delta, zSafe, zCut, steps, zFeed) {
    if (typeof steps == 'undefined') steps = 1;

    var zDelta = zCut / steps;

    for (var i = 0; i < steps; i++) {
      var polys = this.layer_to_polys(layer);
      if (delta) polys = this.offset_polys(polys, delta);
      this.cut_polys(polys, zSafe, zDelta * (i + 1), zFeed);
    }
  }

}, _dxf);
