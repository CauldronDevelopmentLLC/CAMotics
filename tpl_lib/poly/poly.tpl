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

module.exports = {
    print_point: function(point) {
        print('(', point[0], ', ', point[1], ')');
    },


    print_poly: function(poly) {
        print('(');

        for (var j = 0; j < poly.length; j++) {
            if (j) print(', ');
            print_point(poly[j]);
        }

        print(')\n');
    },


    arc2D: function(start, end) {
        var points = [];

        start = 2 * Math.PI - start;
        end = 2 * Math.PI - end;

        var steps = Math.round(Math.abs(end - start) / (Math.PI * 2 / 100)) + 1;
        var delta = (end - start) / steps;

        for (var i = 0; i < steps; i++) {
            var angle = start + delta * i;
            points.push([Math.cos(angle), Math.sin(angle)]);
        }

        return points;
    },


    scale2D: function(points, factor) {
        var result = [];

        for (var i = 0; i < points.length; i++) {
            var point = [];

            for (var j = 0; j < points[i].length; j++)
                point.push(points[i][j] * factor[j]);

            result.push(point);
        }

        return result;
    },


    translate2D: function(points, offset) {
        var result = [];

        for (var i = 0; i < points.length; i++) {
            var point = [];

            for (var j = 0; j < points[i].length; j++)
                point.push(points[i][j] + offset[j]);

            result.push(point);
        }

        return result;
    },


    cut_poly: function(poly, depth, safe) {
        if (typeof safe == 'undefined') safe = true;

        if (safe) rapid(poly[0][0], poly[0][1]);
        else cut(poly[0][0], poly[0][1]);

        cut({z: -depth});

        for (var i = 1; i < poly.length; i++)
            cut(poly[i][0], poly[i][1]);

        if (poly[0][0] != poly[poly.length -1][0] ||
            poly[0][1] != poly[poly.length -1][1])
            cut(poly[0][0], poly[0][1]);

        if (safe) rapid({z: zSafe});
    },


    cut_polys: function(polys, depth, safe) {
        for (var i = 0; i < polys.length; i++)
            cut_poly(polys[i], depth, safe);
    },


    cut_poly_steps: function(poly, depth, zSafe, tabs) {
        if (typeof tabs == 'undefined') tabs = false;
        var steps = Math.ceil(depth / maxPass);
        var pass = depth / steps;
        var safe = typeof zSafe != 'undefined';

        if (safe) rapid(poly[0][0], poly[0][1]);
        else cut(poly[0][0], poly[0][1]);

        for (var step = 1; step <= steps; step++) {
            cut({z: -pass * step});

            for (var i = 1; i < poly.length; i++)
                cut(poly[i][0], poly[i][1]);

            if (poly[0][0] != poly[poly.length -1][0] ||
                poly[0][1] != poly[poly.length -1][1])
                cut(poly[0][0], poly[0][1]);
        }

        if (safe) rapid({z: zSafe});
    },


    cut_polys_steps: function(polys, depth, zSafe) {
        for (var i = 0; i < polys.length; i++)
            cut_poly_steps(polys[i], depth, zSafe, zSafe);
    },


    pocket_poly: function(poly, depth, overlap, zSafe) {
        if (typeof overlap == 'undefined') overlap = 0.4;
        if (typeof zSafe == 'undefined') zSafe = 5;

        var steps = Math.ceil(depth / maxPass);
        var pass = depth / steps;

        for (var step = 1; step <= steps; step++) {
            var pocket = offset([poly], -toolRadius);

            rapid(pocket[0][0][0], pocket[0][0][1]);

            while (pocket.length) {
                cut_polys(pocket, pass * step);
                pocket = offset(pocket, -toolDia + overlap);
            }
        }

        rapid({z: zSafe});
    }
}
