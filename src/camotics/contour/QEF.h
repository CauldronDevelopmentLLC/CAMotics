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

#pragma once

#include <cbang/geom/Vector.h>


// Original code from
// http://www.oocities.org/tzukkers/isosurf/isosurfaces.html
// By Ronen Tzur
// ThreeD Quadric Error Function
//
// This file is in the public domain.


namespace CAMotics {
  /**
   * QEF, a class implementing the quadric error function
   *      E[x] = P - Ni . Pi
   *
   * Given at least three points Pi, each with its respective normal
   * vector Ni, that describe at least two planes, the QEF evaluates
   * to the point x.
   */
  class QEF {
  public:
    static cb::Vector3D evaluate(double mat[12][3], double vec[12], int rows);
    static void computeSVD(double mat[12][3], double u[12][3], double v[3][3],
                           double d[3], int rows);
    static void factorize(double mat[12][3], double tau_u[3], double tau_v[2],
                          int rows);
    static double factorize_hh(double *ptrs[12], int n);
    static void unpack(double u[12][3], double v[3][3], double tau_u[3],
                       double tau_v[2], int rows);
    static void diagonalize(double u[12][3], double v[3][3], double tau_u[3],
                            double tau_v[2], int rows);
    static void chop(double *a, double *b, int n);
    static void qrstep(double u[12][3], double v[3][3], double tau_u[3],
                       double tau_v[3], int rows, int cols);
    static void qrstep_middle(double u[12][3], double tau_u[3], double tau_v[3],
                              int rows, int cols, int col);
    static void qrstep_end(double v[3][3], double tau_u[3], double tau_v[3],
                           int cols);
    static double qrstep_eigenvalue(double tau_u[3], double tau_v[3], int cols);
    static void qrstep_cols2(double u[12][3], double v[3][3], double tau_u[3],
                             double tau_v[3], int rows);
    static void computeGivens(double a, double b, double *c, double *s);
    static void computeSchur(double a1, double a2, double a3, double *c,
                             double *s);
    static void singularize(double u[12][3], double v[3][3], double d[3],
                            int rows);
    static void solveSVD(double u[12][3], double v[3][3], double d[3],
                         double b[12], double x[3], int rows);
  };
}
