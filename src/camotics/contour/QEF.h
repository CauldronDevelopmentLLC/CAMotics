/******************************************************************************\

    CAMotics is an Open-Source CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef CAMOTICS_QEF_H
#define CAMOTICS_QEF_H

// Original code from
// http://www.oocities.org/tzukkers/isosurf/isosurfaces.html
// By Ronen Tzur
// ThreeD Quadric Error Function
// 
// This file is in the public domain.

#include <camotics/Geom.h>


namespace CAMotics {
  /**
   * QEF, a class implementing the quadric error function
   *      E[x] = P - Ni . Pi
   *
   * Given at least three points Pi, each with its respective normal
   * vector Ni, that describe at least two planes, the QEF evalulates
   * to the point x.
   */
  class QEF {
  public:
    static Vector3R evaluate(real mat[12][3], real vec[12], int rows);
    static void computeSVD(real mat[12][3], real u[12][3], real v[3][3],
                           real d[3], int rows);
    static void factorize(real mat[12][3], real tau_u[3], real tau_v[2],
                          int rows);
    static real factorize_hh(real *ptrs[12], int n);
    static void unpack(real u[12][3], real v[3][3], real tau_u[3],
                       real tau_v[2], int rows);
    static void diagonalize(real u[12][3], real v[3][3], real tau_u[3],
                            real tau_v[2], int rows);
    static void chop(real *a, real *b, int n);
    static void qrstep(real u[12][3], real v[3][3], real tau_u[3],
                       real tau_v[3], int rows, int cols);
    static void qrstep_middle(real u[12][3], real tau_u[3], real tau_v[3],
                              int rows, int cols, int col);
    static void qrstep_end(real v[3][3], real tau_u[3], real tau_v[3],
                           int cols);
    static real qrstep_eigenvalue(real tau_u[3], real tau_v[3], int cols);
    static void qrstep_cols2(real u[12][3], real v[3][3], real tau_u[3],
                             real tau_v[3], int rows);
    static void computeGivens(real a, real b, real *c, real *s);
    static void computeSchur(real a1, real a2, real a3, real *c, real *s);
    static void singularize(real u[12][3], real v[3][3], real d[3], int rows);
    static void solveSVD(real u[12][3], real v[3][3], real d[3], real b[12],
                         real x[3], int rows);
  };
}

#endif // CAMOTICS_QEF_H
