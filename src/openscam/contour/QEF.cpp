/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

// Original code from
// http://www.oocities.org/tzukkers/isosurf/isosurfaces.html
// By Ronen Tzur
// ThreeD Quadric Error Function
// 
// This file is in the public domain.

#include "QEF.h"

#include <cbang/Math.h>

#include <string.h>

using namespace cb;
using namespace OpenSCAM;

#define MAXROWS 12
#define EPSILON 1e-5


Vector3R QEF::evaluate(real mat[12][3], real vec[12], int rows) {
  // perform singular value decomposition on matrix mat into u, v and d.
  //   u is a matrix of rows x 3 (same as mat);
  //   v is a square matrix 3 x 3 (for 3 columns in mat);
  //   d is vector of 3 values representing the diagonal matrix 3 x 3
  //     (for 3 colums in mat).
  real u[MAXROWS][3], v[3][3], d[3];

  computeSVD(mat, u, v, d, rows);

  // solve linear system given by mat and vec using the
  // singular value decomposition of mat into u, v and d.
  if (d[2] < 0.1) d[2] = 0;
  if (d[1] < 0.1) d[1] = 0;
  if (d[0] < 0.1) d[0] = 0;

  real x[3];
  solveSVD(u, v, d, vec, x, rows);

  return Vector3R(x);
}


void QEF::computeSVD(real mat[12][3], real u[12][3], real v[3][3], real d[3],
                     int rows) {
  memcpy(u, mat, rows * 3 * sizeof(real));

  real *tau_u = d;
  real tau_v[2];

  factorize(u, tau_u, tau_v, rows);
  unpack(u, v, tau_u, tau_v, rows);
  diagonalize(u, v, tau_u, tau_v, rows);
  singularize(u, v, tau_u, rows);
}


void QEF::factorize(real mat[12][3], real tau_u[3], real tau_v[2], int rows) {
  int y;

  // bidiagonal factorization of (rows x 3) matrix into :- tau_u, a
  // vector of 1x3 (for 3 columns in the matrix) tau_v, a vector of
  // 1x2 (one less column than the matrix)
  for (int i = 0; i < 3; ++i) {
    // set up a vector to reference into the matrix from mat(i,i) to
    // mat(m,i), that is, from the i'th column of the i'th row and
    // down all the way through that column
    real *ptrs[MAXROWS * 2];
    int num_ptrs = rows - i;
    for (int q = 0; q < num_ptrs; ++q) ptrs[q] = &mat[q + i][i];

    // perform householder transformation on this vector
    real tau = factorize_hh(ptrs, num_ptrs);
    tau_u[i] = tau;

    // all computations below this point are performed only for the
    // first two columns: i=0 or i=1
    if (i + 1 < 3) {

      // perform householder transformation on the matrix mat(i,i+1)
      // to mat(m,n), that is, on the sub-matrix that begins in the
      // (i+1)'th column of the i'th row and extends to the end of the
      // matrix at (m,n)
      if (tau)
        for (int x = i + 1; x < 3; ++x) {
          real wx = mat[i][x];
          for (y = i + 1; y < rows; ++y)
            wx += mat[y][x] * (*ptrs[y - i]);
          real tau_wx = tau * wx;
          mat[i][x] -= tau_wx;
          for (y = i + 1; y < rows; ++y)
            mat[y][x] -= tau_wx * (*ptrs[y - i]);
        }

      // perform householder transformation on i'th row (remember at
      // this point, i is either 0 or 1)

      // set up a vector to reference into the matrix from mat(i,i+1)
      // to mat(i,n), that is, from the (i+1)'th column of the i'th
      // row and all the way through to the end of that row
      ptrs[0] = &mat[i][i + 1];
      if (!i) {
        ptrs[1] = &mat[i][i + 2];
        num_ptrs = 2;

      } else num_ptrs = 1;

      // perform householder transformation on this vector
      tau = factorize_hh(ptrs, num_ptrs);
      tau_v[i] = tau;

      // perform householder transformation on the sub-matrix
      // mat(i+1,i+1) to mat(m,n), that is, on the sub-matrix that
      // begins in the (i+1)'th column of the (i+1)'th row and extends
      // to the end of the matrix at (m,n)
      if (tau)
        for (y = i + 1; y < rows; ++y) {
          real wy = mat[y][i + 1];
          if (!i) wy += mat[y][i + 2] * (*ptrs[1]);

          real tau_wy = tau * wy;
          mat[y][i + 1] -= tau_wy;
          if (!i) mat[y][i + 2] -= tau_wy * (*ptrs[1]);
        }
    }
  }
}


real QEF::factorize_hh(real *ptrs[12], int n) {
  real tau = 0;

  if (n > 1) {
    real xnorm;
    if (n == 2) xnorm = fabs(*ptrs[1]);
    else {
      real scl = 0;
      real ssq = 1;

      for (int i = 1; i < n; ++i) {
        real x = fabs(*ptrs[i]);
        if (x) {
          if (scl < x) {
            ssq = 1 + ssq * (scl / x) * (scl / x);
            scl = x;

          } else ssq += (x / scl) * (x / scl);
        }
      }

      xnorm = scl * sqrt(ssq);
    }

    if (xnorm) {
      real alpha = *ptrs[0];
      real beta = sqrt(alpha * alpha + xnorm * xnorm);
      if (alpha >= 0) beta = -beta;
      tau = (beta - alpha) / beta;

      real scl = 1.0 / (alpha - beta);
      *ptrs[0] = beta;
      for (int i = 1; i < n; ++i) *ptrs[i] *= scl;
    }
  }

  return tau;
}


void QEF::unpack(real u[12][3], real v[3][3], real tau_u[3], real tau_v[2],
                 int rows) {
  // reset v to the identity matrix
  v[0][0] = v[1][1] = v[2][2] = 1;
  v[0][1] = v[0][2] = v[1][0] = v[1][2] = v[2][0] = v[2][1] = 0;

  for (int i = 1; i >= 0; --i) {
    real tau = tau_v[i];

    // perform householder transformation on the sub-matrix v(i+1,i+1)
    // to v(m,n), that is, on the sub-matrix of v that begins in the
    // (i+1)'th column of the (i+1)'th row and extends to the end of
    // the matrix at (m,n).  the householder vector used to perform
    // this is the vector from u(i,i+1) to u(i,n)
    if (tau)
      for (int x = i + 1; x < 3; ++x) {
        real wx = v[i + 1][x];
        for (int y = i + 1 + 1; y < 3; ++y)
          wx += v[y][x] * u[i][y];

        real tau_wx = tau * wx;
        v[i + 1][x] -= tau_wx;
        for (int y = i + 1 + 1; y < 3; ++y)
          v[y][x] -= tau_wx * u[i][y];
      }
  }

  // copy superdiagonal of u into tau_v
  for (int i = 0; i < 2; ++i) tau_v[i] = u[i][i + 1];

  // below, same idea for u: householder transformations
  // and the superdiagonal copy
  for (int i = 2; i >= 0; --i) {
    // copy superdiagonal of u into tau_u
    real tau = tau_u[i];
    tau_u[i] = u[i][i];

    // perform householder transformation on the sub-matrix u(i,i) to
    // u(m,n), that is, on the sub-matrix of u that begins in the i'th
    // column of the i'th row and extends to the end of the matrix at
    // (m,n).  the householder vector used to perform this is the i'th
    // column of u, that is, u(0,i) to u(m,i)
    if (!tau) {
      u[i][i] = 1;

      if (i < 2) {
        u[i][2] = 0;
        if (i < 1) u[i][1] = 0;
      }

      for (int y = i + 1; y < rows; ++y) u[y][i] = 0;

    } else {
      for (int x = i + 1; x < 3; ++x) {
        real wx = 0;
        for (int y = i + 1; y < rows; ++y) wx += u[y][x] * u[y][i];

        real tau_wx = tau * wx;
        u[i][x] = -tau_wx;
        for (int y = i + 1; y < rows; ++y) u[y][x] -= tau_wx * u[y][i];
      }

      for (int y = i + 1; y < rows; ++y)
        u[y][i] = u[y][i] * -tau;

      u[i][i] = 1.0 - tau;
    }
  }
}


void QEF::diagonalize(real u[12][3], real v[3][3], real tau_u[3], real tau_v[2],
                      int rows) {
  chop(tau_u, tau_v, 3);

  // progressively reduce the matrices into diagonal form

  int b = 3 - 1;
  while (b > 0) {
    if (!tau_v[b - 1]) --b;
    else {
      int a = b - 1;
      while (a > 0 && tau_v[a - 1]) --a;

      int n = b - a + 1;
      real u1[MAXROWS][3];
      real v1[3][3];
      for (int j = a; j <= b; ++j) {
        for (int i = 0; i < rows; ++i) u1[i][j - a] = u[i][j];
        for (int i = 0; i < 3; ++i) v1[i][j - a] = v[i][j];
      }

      qrstep(u1, v1, &tau_u[a], &tau_v[a], rows, n);

      for (int j = a; j <= b; ++j) {
        for (int i = 0; i < rows; ++i) u[i][j] = u1[i][j - a];
        for (int i = 0; i < 3; ++i) v[i][j] = v1[i][j - a];
      }

      chop(&tau_u[a], &tau_v[a], n);
    }
  }
}


void QEF::chop(real *a, real *b, int n) {
  real ai = a[0];

  for (int i = 0; i < n - 1; ++i) {
    real bi = b[i];
    real ai1 = a[i + 1];
    if (fabs(bi) < EPSILON * (fabs(ai) + fabs(ai1))) b[i] = 0;
    ai = ai1;
  }
}


void QEF::qrstep(real u[12][3], real v[3][3], real tau_u[3], real tau_v[3],
                 int rows, int cols) {
  if (cols == 2) {
    qrstep_cols2(u, v, tau_u, tau_v, rows);
    return;
  }

  if (cols == 1) {
    char *bomb = 0;
    *bomb = 0;
  }

  // handle zeros on the diagonal or at its end
  for (int i = 0; i < cols - 1; ++i)
    if (!tau_u[i]) {
      qrstep_middle(u, tau_u, tau_v, rows, cols, i);
      return;
    }

  if (!tau_u[cols - 1]) {
    qrstep_end(v, tau_u, tau_v, cols);
    return;
  }

  // perform qr reduction on the diagonal and off-diagonal
  real mu = qrstep_eigenvalue(tau_u, tau_v, cols);
  real y = tau_u[0] * tau_u[0] - mu;
  real z = tau_u[0] * tau_v[0];

  real ak = 0;
  real bk = 0;
  real zk;
  real ap = tau_u[0];
  real bp = tau_v[0];
  real aq = tau_u[1];

  for (int k = 0; k < cols - 1; ++k) {
    real c, s;

    // perform Givens rotation on V
    computeGivens(y, z, &c, &s);

    for (int i = 0; i < 3; ++i) {
      real vip = v[i][k];
      real viq = v[i][k + 1];
      v[i][k] = vip * c - viq * s;
      v[i][k + 1] = vip * s + viq * c;
    }

    // perform Givens rotation on B
    real bk1 = bk * c - z * s;
    real ap1 = ap * c - bp * s;
    real bp1 = ap * s + bp * c;
    real zp1 = aq * -s;
    real aq1 = aq * c;

    if (k > 0) tau_v[k - 1] = bk1;
    ak = ap1;
    bk = bp1;
    zk = zp1;
    ap = aq1;

    if (k < cols - 2) bp = tau_v[k + 1];
    else bp = 0;
    y = ak;
    z = zk;

    // perform Givens rotation on U
    computeGivens(y, z, &c, &s);

    for (int i = 0; i < rows; ++i) {
      real uip = u[i][k];
      real uiq = u[i][k + 1];
      u[i][k] = uip * c - uiq * s;
      u[i][k + 1] = uip * s + uiq * c;
    }

    // perform Givens rotation on B
    real ak1 = ak * c - zk * s;
    bk1 = bk * c - ap * s;
    real zk1 = bp * -s;

    ap1 = bk * s + ap * c;
    bp1 = bp * c;

    tau_u[k] = ak1;

    ak = ak1;
    bk = bk1;
    zk = zk1;
    ap = ap1;
    bp = bp1;

    if (k < cols - 2) aq = tau_u[k + 2];
    else aq = 0;

    y = bk;
    z = zk;
  }

  tau_v[cols - 2] = bk;
  tau_u[cols - 1] = ap;
}


void QEF::qrstep_middle(real u[12][3], real tau_u[3], real tau_v[3],
                        int rows, int cols, int col) {
  real x = tau_v[col];
  real y = tau_u[col + 1];

  for (int j = col; j < cols - 1; ++j) {
    real c, s;

    // perform Givens rotation on U
    computeGivens(y, -x, &c, &s);

    for (int i = 0; i < rows; ++i) {
      real uip = u[i][col];
      real uiq = u[i][j + 1];
      u[i][col] = uip * c - uiq * s;
      u[i][j + 1] = uip * s + uiq * c;
    }

    // perform transposed Givens rotation on B
    tau_u[j + 1] = x * s + y * c;
    if (j == col) tau_v[j] = x * c - y * s;

    if (j < cols - 2) {
      real z = tau_v[j + 1];
      tau_v[j + 1] *= c;
      x = z * -s;
      y = tau_u[j + 2];
    }
  }
}


void QEF::qrstep_end(real v[3][3], real tau_u[3], real tau_v[3], int cols) {
  real x = tau_u[1];
  real y = tau_v[1];

  for (int k = 1; k >= 0; --k) {
    real c, s;

    // perform Givens rotation on V
    computeGivens(x, y, &c, &s);

    for (int i = 0; i < 3; ++i) {
      real vip = v[i][k];
      real viq = v[i][2];
      v[i][k] = vip * c - viq * s;
      v[i][2] = vip * s + viq * c;
    }

    // perform Givens rotation on B
    tau_u[k] = x * c - y * s;
    if (k == 1) tau_v[k] = x * s + y * c;

    if (k > 0) {
      real z = tau_v[k - 1];
      tau_v[k - 1] *= c;

      x = tau_u[k - 1];
      y = z * s;
    }
  }
}


real QEF::qrstep_eigenvalue(real tau_u[3], real tau_v[2], int cols) {
  real ta = tau_u[1] * tau_u[1] + tau_v[0] * tau_v[0];
  real tb = tau_u[2] * tau_u[2] + tau_v[1] * tau_v[1];
  real tab = tau_u[1] * tau_v[1];
  real dt = (ta - tb) / 2.0;
  real mu;

  if (dt >= 0) mu = tb - (tab * tab) / (dt + sqrt(dt * dt + tab * tab));
  else mu = tb + (tab * tab) / (sqrt(dt * dt + tab * tab) - dt);

  return mu;
}


void QEF::qrstep_cols2(real u[12][3], real v[3][3], real tau_u[3],
                       real tau_v[3], int rows) {
  // eliminate off-diagonal element in [0 tau_v0] [0 tau_u1]
  // to make [tau_u[0] 0] [0 0]
  if (!tau_u[0]) {
    // perform transposed Givens rotation on B multiplied by X = [0 1] [1 0]
    real c, s;
    computeGivens(tau_v[0], tau_u[1], &c, &s);

    tau_u[0] = tau_v[0] * c - tau_u[1] * s;
    tau_v[0] = tau_v[0] * s + tau_u[1] * c;
    tau_u[1] = 0;

    // perform Givens rotation on U
    for (int i = 0; i < rows; ++i) {
      real uip = u[i][0];
      real uiq = u[i][1];
      u[i][0] = uip * c - uiq * s;
      u[i][1] = uip * s + uiq * c;
    }

    // multiply V by X, effectively swapping first two columns
    for (int i = 0; i < 3; ++i) {
      real tmp = v[i][0];
      v[i][0] = v[i][1];
      v[i][1] = tmp;
    }

  } else if (!tau_u[1]) {
    // eliminate off-diagonal element in [tau_u0 tau_v0] [0 0]
    real c, s;

    // perform Givens rotation on B
    computeGivens(tau_u[0], tau_v[0], &c, &s);

    tau_u[0] = tau_u[0] * c - tau_v[0] * s;
    tau_v[0] = 0;

    // perform Givens rotation on V
    for (int i = 0; i < 3; ++i) {
      real vip = v[i][0];
      real viq = v[i][1];
      v[i][0] = vip * c - viq * s;
      v[i][1] = vip * s + viq * c;
    }

  } else { // make colums orthogonal
    real c, s;

    // perform Schur rotation on B
    computeSchur(tau_u[0], tau_v[0], tau_u[1], &c, &s);

    real a11 = tau_u[0] * c - tau_v[0] * s;
    real a21 = -tau_u[1] * s;
    real a12 = tau_u[0] * s + tau_v[0] * c;
    real a22 = tau_u[1] * c;

    // perform Schur rotation on V
    for (int i = 0; i < 3; ++i) {
      real vip = v[i][0];
      real viq = v[i][1];
      v[i][0] = vip * c - viq * s;
      v[i][1] = vip * s + viq * c;
    }

    // eliminate off diagonal elements
    if ((a11 * a11 + a21 * a21) < (a12 * a12 + a22 * a22)) {
      // multiply B by X
      real tmp = a11;
      a11 = a12;
      a12 = tmp;
      tmp = a21;
      a21 = a22;
      a22 = tmp;

      // multiply V by X, effectively swapping first two columns
      for (int i = 0; i < 3; ++i) {
        real tmp = v[i][0];
        v[i][0] = v[i][1];
        v[i][1] = tmp;
      }
    }

    // perform transposed Givens rotation on B
    computeGivens(a11, a21, &c, &s);

    tau_u[0] = a11 * c - a21 * s;
    tau_v[0] = a12 * c - a22 * s;
    tau_u[1] = a12 * s + a22 * c;

    // perform Givens rotation on U
    for (int i = 0; i < rows; ++i) {
      real uip = u[i][0];
      real uiq = u[i][1];
      u[i][0] = uip * c - uiq * s;
      u[i][1] = uip * s + uiq * c;
    }
  }
}


void QEF::computeGivens(real a, real b, real *c, real *s) {
  if (!b) {
    *c = 1;
    *s = 0;

  } else if (fabs(b) > fabs(a)) {
    real t = -a / b;
    real s1 = 1.0 / sqrt(1 + t * t);
    *s = s1;
    *c = s1 * t;

  } else {
    real t = -b / a;
    real c1 = 1.0 / sqrt(1 + t * t);
    *c = c1;
    *s = c1 * t;
  }
}


void QEF::computeSchur(real a1, real a2, real a3, real *c, real *s) {
  real apq = a1 * a2 * 2.0;

  if (!apq) {
    *c = 1;
    *s = 0;

  } else {
    real t;
    real tau = (a2 * a2 + (a3 + a1) * (a3 - a1)) / apq;

    if (tau >= 0) t = 1.0 / (tau + sqrt(1 + tau * tau));
    else t = -1.0 / (sqrt(1 + tau * tau) - tau);

    *c = 1.0 / sqrt(1 + t * t);
    *s = t * (*c);
  }
}


void QEF::singularize(real u[12][3], real v[3][3], real d[3], int rows) {
  // make singularize values positive
  for (int j = 0; j < 3; ++j)
    if (d[j] < 0) {
      for (int i = 0; i < 3; ++i) v[i][j] = -v[i][j];
      d[j] = -d[j];
    }

  // sort singular values in decreasing order
  for (int i = 0; i < 3; ++i) {
    real d_max = d[i];
    int i_max = i;

    for (int j = i + 1; j < 3; ++j)
      if (d[j] > d_max) {
        d_max = d[j];
        i_max = j;
      }

    if (i_max != i) {
      // swap eigenvalues
      real tmp = d[i];
      d[i] = d[i_max];
      d[i_max] = tmp;

      // swap eigenvectors
      for (int y = 0; y < rows; ++y) {
        tmp = u[y][i];
        u[y][i] = u[y][i_max];
        u[y][i_max] = tmp;
      }

      for (int y = 0; y < 3; ++y) {
        tmp = v[y][i];
        v[y][i] = v[y][i_max];
        v[y][i_max] = tmp;
      }
    }
  }
}


void QEF::solveSVD(real u[12][3], real v[3][3], real d[3], real b[12],
                   real x[3], int rows) {
  static real zeroes[3] = {0, 0, 0};

  // compute vector w = U^T * b
  real w[3];

  memcpy(w, zeroes, sizeof(w));
  for (int i = 0; i < rows; ++i)
    if (b[i])
      for (int j = 0; j < 3; ++j)
        w[j] += b[i] * u[i][j];

  // introduce non-zero singular values in d into w
  for (int i = 0; i < 3; ++i)
    if (d[i]) w[i] /= d[i];

  // compute result vector x = V * w
  for (int i = 0; i < 3; ++i) {
    real tmp = 0;
    for (int j = 0; j < 3; ++j) tmp += w[j] * v[i][j];
    x[i] = tmp;
  }
}
