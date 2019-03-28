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

#include "SCurve.h"

#include <cbang/Exception.h>
#include <cbang/Math.h>

#include <limits>

using namespace std;


namespace GCode {
  bool near(double x, double y, double delta) {
    return x - delta < y && y < x + delta;
  }


  vector<complex<double> > solveQuadratic(double a, double b, double c) {
    // Find roots of quadratic polynomial of the form:
    //
    //   a * x^2 + b * x + c = 0

    vector<complex<double> > solutions;

    if (a) {
      // Use the quadratic formula:
      //
      //   x = (-b +/- sqrt(b^2 - 4 * a * c)) / (2 * a)

      double q = square(b) - 4 * a * c;
      complex<double> r = sqrt(complex<double>(q));

      solutions.push_back((-b + r) / (2 * a));
      if (q) solutions.push_back((-b - r) / (2 * a));

    } else if (b) solutions.push_back(-c / b);

    return solutions;
  }


  namespace SCurve {
    double distance(double t, double v, double a, double j) {
      // v * t + 1/2 * a * t^2 + 1/6 * j * t^3
      return t * (v + t * (0.5 * a + 1.0 / 6.0 * j * t));
    }


    double velocity(double t, double a, double j) {
      // a * t + 1/2 * j * t^2
      return t * (a + 0.5 * j * t);
    }


    double acceleration(double t, double j) {return j * t;}


    double timeAtDistance(double d, double v, double a, double j, double maxT,
                          double tolerance) {
      if (d < tolerance) return 0;

      double maxD = distance(maxT, v, a, j);

      if (isnan(d) || isnan(maxD))
        THROW("Invalid input to timeAtDistance: d=" << d << " v=" << v
               << " a=" << a << " j=" << j << " maxT=" << maxT);

      if (near(maxD, d, tolerance)) return maxT;

      if (maxD < d)
        THROW("Distance " << d << " beyond max time " << maxT <<
               " with max distance " << maxD);

      // Newton–Raphson method to find solution within tolerance
      double t = 0.5 * maxT; // Initial guess

      while (true) {
        double x = distance(t, v, a, j) - d;
        if (near(x, 0, tolerance)) return t;
        t -= x / (v + velocity(t, a, j));
      }
    }


    double timeAtVelocity(double iV, double tV, double a, double j,
                          double tolerance) {
      if (near(iV, tV, tolerance) || iV < 0 || tV < 0) return 0;

      // Solve j / 2 * t^2 + a * t + (iV - tV) = 0 for t
      vector<complex<double> > solutions = solveQuadratic(0.5 * j, a, iV - tV);

      double t = numeric_limits<double>::quiet_NaN();

      for (unsigned i = 0; i < solutions.size(); i++)
        if (abs(solutions[i].imag()) < tolerance && 0 < solutions[i].real())
          if (isnan(t) || solutions[i].real() < t) t = solutions[i].real();

      if (isnan(t))
        THROW("Invalid time at velocity: iV=" << iV << " tV=" << tV << " a="
               << a << " j=" << j);

      return t;
    }
  }
}
