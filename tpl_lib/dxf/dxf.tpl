/******************************************************************************\

  CAMotics is an Open-Source simulation and CAM software.
  Copyright (C) 2011-2019 Joseph Coffland <joseph@cauldrondevelopment.com>

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


// Object.assign polyfill
if (typeof Object.assign != 'function') {
  Object.assign = function(target, varArgs) { // .length of function is 2
    'use strict';
    if (target == null) { // TypeError if undefined or null
      throw new TypeError('Cannot convert undefined or null to object');
    }

    var to = Object(target);

    for (var index = 1; index < arguments.length; index++) {
      var nextSource = arguments[index];

      if (nextSource != null) { // Skip over if undefined or null
        for (var nextKey in nextSource) {
          // Avoid bugs when hasOwnProperty is shadowed
          if (Object.prototype.hasOwnProperty.call(nextSource, nextKey)) {
            to[nextKey] = nextSource[nextKey];
          }
        }
      }
    }
    return to;
  };
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


function sqr(x) {return x * x;}
function cube(x) {return x * x * x;}
function first(a) {return a[0];}
function last(a) {return a[a.length - 1];}


function distance2D(p1, p2) {
  return Math.sqrt(sqr(p2.x - p1.x) + sqr(p2.y - p1.y))
}


function split_seg_2d(start, end, ratio) {
  return {
    x: start.x + ratio * (end.x - start.x),
    y: start.y + ratio * (end.y - start.y)
  };
}


function bounds_new() {
  return {
    min: {x: Infinity, y: Infinity, z: Infinity},
    max: {x: -Infinity, y: -Infinity, z: -Infinity}
  };
}


function bounds_add_point(bounds, p) {
  return {
    min: {
      x: Math.min(bounds.min.x, p.x),
      y: Math.min(bounds.min.y, p.y),
      z: Math.min(bounds.min.z, p.z)
    },
    max: {
      x: Math.max(bounds.max.x, p.x),
      y: Math.max(bounds.max.y, p.y),
      z: Math.max(bounds.max.z, p.z)
    }
  };
}


function bounds_add(a, b) {
  return {
    min: {
      x: Math.min(a.min.x, b.min.x),
      y: Math.min(a.min.y, b.min.y),
      z: Math.min(a.min.z, b.min.z)
    },
    max: {
      x: Math.max(a.max.x, b.max.x),
      y: Math.max(a.max.y, b.max.y),
      z: Math.max(a.max.z, b.max.z)
    }
  };
}


function bounds_dims(bounds) {
  return {
    x: bounds.max.x - bounds.min.x,
    y: bounds.max.y - bounds.min.y,
    z: bounds.max.z - bounds.min.z
  };
}


function bounds_center(bounds) {
  return {
    x: (bounds.max.x - bounds.min.x) / 2 + bounds.min.x,
    y: (bounds.max.y - bounds.min.y) / 2 + bounds.min.y,
    z: (bounds.max.z - bounds.min.z) / 2 + bounds.min.z
  }
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
  arc_error: 0.0001, // In length units


  // Vertices ******************************************************************
  line_vertices: function(l) {return [l.start, l.end];},


  angle_vertex: function(center, radius, angle) {
    return {
      x: center.x + radius * Math.cos(angle * Math.PI / 180),
      y: center.y + radius * Math.sin(angle * Math.PI / 180),
    }
  },


  arc_vertices: function(a) {
    var angle = (a.endAngle - a.startAngle) * (a.clockwise ? 1 : -1);
    if (angle <= 0) angle += 360;

    // Allowed error cannot be greater than arc radius
    var error = Math.min(this.arc_error, a.radius);
    var error_angle = 2 * Math.acos(1 - error / a.radius);

    // Error angle cannot be greater than 2Pi/3 because we need at least 3
    // segments in a full circle
    error_angle = Math.min(2 * Math.PI / 3, error_angle);

    var steps = Math.ceil(angle / (error_angle / Math.PI * 180));
    var delta = angle / steps;

    // TODO The estimated arc should straddle the actual arc.  This one
    // always lies completely inside the arc and therefore always
    // underestimates it.  However, the arc must still start and stop at the
    // correct points.  Therefore, it must start and stop where the segments
    // intersect with the actual arc.

    var v = [];
    for (var i = 0; i <= steps; i++)
      v.push(this.angle_vertex(a.center, a.radius, a.startAngle + delta * i));

    return v;
  },


  polyline_vertices: function(pl) {return pl.vertices;},
  spline_vertices: function(s) {return s.ctrlPts;},


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


  line_cut: function(l) {cut(l.end.x, l.end.y);},


  arc_angle: function(a) {
    var angle = (a.endAngle - a.startAngle) * (a.clockwise ? -1 : 1)

    //if (angle <= 0) angle += 360;

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
      cut(pl.vertices[i].x, pl.vertices[i].y);

    if (pl.vertices.length) cut(pl.vertices[0].x, pl.vertices[0].y);
  },


  spline_cut: function(s, res) {
    if (typeof res == 'undefined') res = units() == METRIC ? 1 : 1 / 25.4;

    if (s.degree == 2) {
      var steps = Math.ceil(quad_bezier_length(s.ctrlPts) / res);
      var delta = 1 / steps;

      for (var i = 0; i < steps; i++) {
        v = quad_bezier(s.ctrlPts, delta * (i + 1));
        cut(v.x, v.y);
      }


    } else if (s.degree == 3) {
      var steps = Math.ceil(cubic_bezier_length(s.ctrlPts) / res);
      var delta = 1 / steps;

      for (var i = 0; i < steps; i++) {
        var v = cubic_bezier(s.ctrlPts, delta * (i + 1));
        cut(v.x, v.y);
      }

    } else
      for (var i = 1; i < s.ctrlPts.length; i++)
        cut(s.ctrlPts[i].x, s.ctrlPts[i].y);
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
      cut(v.x, v.y);

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
  poly_com: function(poly) { // Center Of Mass
    // TODO broken?
    var c = {x: 0, y: 0};

    for (var i = 0; i < poly.length; i++) {
      c.x += poly[i].x;
      c.y += poly[i].y;
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
      c.x += pc.x;
      c.y += pc.y;
    }

    c.x /= polys.length;
    c.y /= polys.length;

    return c;
  },


  poly_closest: function(poly, p) {
    var best = Infinity;
    var index = undefined;

    for (var i = 0; i < poly.length; i++) {
      var d = distance2D(poly[i], p);

      if (d < best) {
        best = d;
        index = i;
      }
    }

    return index;
  },


  polys_closest: function(polys, p) {
    var best = Infinity;
    var index = undefined;

    for (var i = 0; i < polys.length; i++) {
      var closest = this.poly_closest(polys[i], p);
      var d = distance2D(polys[i][closest], p);

      if (d < best) {
        best = d;
        index = i;
      }
    }

    return index;
  },


  layer_com: function(layer) {
    var polys = this.layer_to_polys(layer);
    return this.polys_com(polys);
  },


  layer_to_polys: function(layer) {
    if (!layer.length) return [];
    layer = [].concat(layer); // Copy layer

    var polys = [];
    var poly = undefined;
    var p = position();

    while (layer.length) {
      // TODO This is inefficient.  Should put points into octree.
      var match = this.find_closest(p, layer);
      var e = layer[match.i];
      var v = [].concat(this.element_vertices(e));
      if (match.flip) v.reverse();

      var d = distance2D(first(v), p);

      if (0.01 < d || typeof poly == 'undefined') {
        poly = [];
        polys.push(poly);
      }

      for (var i = 0; i < v.length; i++)
        poly.push(v[i]);

      p = last(v);

      layer.splice(match.i, 1);
    }

    return polys;
  },


  find_leadin_offset: function (poly, p, delta) {
    var closest = this.poly_closest(poly, p);
    var off = offset([poly], -delta);
    if (!off.length) return poly[closest];
    closest = this.poly_closest(off[0], p);
    return off[0][closest];
  },


  cut_polys: function(polys, zSafe, zCut, zFeed, close, leadin, cut_cb) {
    if (!polys.length) return;
    close = typeof close == 'undefined' ? true : close;
    leadin = typeof leadin == 'undefined' ? 0 : leadin;

    polys = polys.slice(); // Copy

    while (polys.length) {
      var p = position();
      var next = this.polys_closest(polys, p);
      var poly = polys[next];
      polys.splice(next, 1);

      if (!poly.length) continue;

      var closest = this.poly_closest(poly, p);
      var v = poly[closest];

      if (leadin) {
        var leadinV = v;
        v = this.find_leadin_offset(poly, p, leadin);
      }

      var d = distance2D(v, p);

      if (0.01 < d) {
        if (typeof zSafe != 'undefined') rapid({z: zSafe});
        v.z = position().z;
        if (typeof zSafe == 'undefined') cut(v);
        else rapid(v);
      }

      if (typeof cut_cb != 'undefined') cut_cb(true);

      var f = feed()[0];
      if (typeof zFeed != 'undefined') feed(zFeed);
      if (typeof zCut != 'undefined') cut({z: zCut});
      feed(f);

      if (leadin) v = leadinV;

      for (var j = 0; j < poly.length; j++) {
        var index = (j + closest) % poly.length;
        cut(poly[index].x, poly[index].y);
      }

      if (close) cut(v.x, v.y); // Close poly

      if (typeof cut_cb != 'undefined') cut_cb(false);
   }
  },


  poly_length: function(poly) {
    if (!poly.length) return 0;

    var length = 0;
    var last;

    for (var i = 0; i < poly.length; i++) {
      if (i) length += distance2D(poly[i], last);
      last = poly[i];
    }

    // Close poly
    length += distance2D(last, poly[0]);

    return length;
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


  cut_layer_offset: function (layer, delta, zSafe, zCut, steps, zFeed, close,
                              leadin, cut_cb) {
    if (typeof zCut == 'undefined') zCut = 0;
    if (typeof steps == 'undefined') steps = 1;

    var zDelta = zCut / steps;
    var polys = this.layer_to_polys(layer);
    if (delta) polys = this.offset_polys(polys, delta);

    for (var i = 0; i < steps; i++)
      this.cut_polys(polys, zSafe, zDelta * (i + 1), zFeed, close, leadin,
                     cut_cb);
  },


  compute_tab_points: function (length, config) {
    if (typeof config.tabs == 'undefined') return [];

    var points = [];

    for (var i = 0; i < config.tabs.length; i++) {
      var tab = config.tabs[i];
      var slope = tab.slope || config.tabSlope || config.zSlope;
      var center = tab.position || tab.ratio * length;
      var height = tab.height || config.tabHeight || 1;

      tab.position = center;
      tab.start = center - height / slope;
      tab.end = center + height / slope;
      tab.slope = slope;
      tab.height = height;

      while (tab.start < 0) tab.start += length;
      while (length <= tab.end) tab.end -= length;

      points.push(tab.start);
      points.push(tab.position);
      points.push(tab.end);
    }

    // Sort tab points
    points.sort(function (a, b) {return a - b});

    return points;
  },


  split_poly_at_tab_points: function (poly, points, config) {
    var output = [];
    var j = 0;
    var d = 0;

    for (var i = 0; i < poly.length; i++) {
      output.push(poly[i]);

      var next = (i == poly.length - 1) ? 0 : i + 1;
      var dist = distance2D(poly[i], poly[next]);
      var dNext = d + dist;

      while (points[j] == d) j++;

      while (points[j] < dNext) {
        output.push(split_seg_2d(poly[i], poly[next], (points[j] - d) / dist));
        j++;
      }

      d = dNext;
    }

    return output;
  },


  set_tab_heights: function (path, length, config) {
    if (typeof config.tabs == 'undefined') return;

    var dist = 0;

    for (var i = 0; i < path.length; i++) {
      if (i) {
        dist += distance2D(path[i - 1], path[i]);
        if (length <= dist) dist -= length;
      }

      for (var j = 0; j < config.tabs.length; j++) {
        var tab = config.tabs[j];

        // Check height
        if (config.zEnd + tab.height <= path[i].z) continue;

        var tStart = tab.start;
        var tEnd = tab.end;
        var tMid = tab.position;

        // Adjust values for wrap around tabs
        if (tEnd < tStart) {
          if (tStart < dist) {
            tEnd += length;
            if (tMid < tStart) tMid += length;

          } else {
            tStart -= length;
            if (tEnd < tMid) tMid -= length;
          }
        }

        // Check range
        if (dist <= tStart) continue;

        if (dist <= tMid) {
          var tabZ = config.zEnd + tab.slope * (dist - tStart);
          if (path[i].z < tabZ) path[i].z = tabZ;

        } else if (dist <= tEnd) {
          var tabZ = config.zEnd + tab.slope * (tEnd - dist);
          if (path[i].z < tabZ) path[i].z = tabZ;
        }
      }
    }
  },


  profile_segment: function (start, end, config) {
    var zTarget = end.z - config.zMax;
    if (zTarget < config.zEnd || typeof config.zMax == 'undefined')
      zTarget = config.zEnd;

    if (zTarget - 0.01 < start.z && start.z < zTarget + 0.01)
      return [{x: end.x, y: end.y, z: zTarget}];

    var len = distance2D(start, end);
    var rise = Math.abs(zTarget - start.z);
    var run = rise / config.zSlope;

    if (len < run)
      // Not enough length to achieve target depth
      return [{x: end.x, y: end.y, z: start.z - config.zSlope * len}];

    // Split segment
    var split = split_seg_2d(start, end, run / len);
    split.z = zTarget;

    return [split, {x: end.x, y: end.y, z: zTarget}];
  },


  // TODO Does not work for unclosed poly lines
  profile_poly: function (poly, config) {
    var self = this;
    if (!poly.length) return [];

    if (config.zStart < config.zEnd) throw 'zEnd must be <= zStart';

    var length = this.poly_length(poly);
    var tab_points = this.compute_tab_points(length, config);
    poly = this.split_poly_at_tab_points(poly, tab_points, config);

    var prev = [];
    for (var i = 0; i < poly.length; i++)
      prev.push({x: poly[i].x, y: poly[i].y, z: config.zStart});

    var path = [Object.assign({}, prev[0])]; // Must copy object, updated later
    var done = false;
    var last = prev[0];

    function add_seg(target, start, end) {
      var seg = self.profile_segment(start, end, config);
      Array.prototype.push.apply(target, seg);
      last = seg[seg.length - 1];
    }

    var first = true;
    while (!done) {
      for (var i = 1; i < prev.length && (!done || first); i++) {
        done = prev[i].z == config.zEnd;
        add_seg(path, last, prev[i]);
        prev[i].z = last.z;
      }

      first = false;

      // Close poly
      if (!done) {
        add_seg(path, last, prev[0]);
        prev[0].z = last.z;
      }
    }

    this.set_tab_heights(path, length, config);

    return path;
  },


  cut_path: function (path, config) {
    if (!path.length) return;

    cut({z: config.zSafe});
    rapid({x: path[0].x, y: path[0].y});

    for (var j = 0; j < path.length; j++)
      cut(path[j]);
  },


  cut_layer: function (layer, _config) {
    var config = Object.assign({
      delta: 0,
      zSafe: 5,
      zStart: 0,
      zEnd: 0,
      zMax: 3,
      zSlope: 1.0 / 3,
      tabs: []
    }, _config);

    var polys = this.layer_to_polys(layer);
    if (config.delta) polys = this.offset_polys(polys, config.delta);

    if (typeof polys.length == 'undefined') return;

    for (var i = 0; i < polys.length; i++)
      this.cut_path(this.profile_poly(polys[i], config), config);
  },


  poly_bounds: function (poly) {
    var bounds = bounds_new();

    for (var i = 0; i < poly.length; i++)
      bounds = bounds_add_point(bounds, poly[i]);

    return bounds;
  },


  poly_center: function (poly) {
    return bounds_center(this.poly_bounds(poly));
  },


  layer_bounds: function (layer) {
    var bounds = bounds_new();

    var polys = this.layer_to_polys(layer);
    for (var i = 0; i < polys.length; i++)
      bounds = bounds_add(bounds, this.poly_bounds(polys[i]));

    return bounds;
  },


  layer_center: function (poly) {
    return bounds_center(this.layer_bounds(poly));
  },


  layer_dims: function (layer) {return bounds_dims(this.layer_bounds(layer));}

}, _dxf);
