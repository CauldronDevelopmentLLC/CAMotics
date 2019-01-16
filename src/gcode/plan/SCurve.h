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

#include <vector>
#include <complex>


namespace GCode {
  template<typename T> T square(T x) {return x * x;}
  template<typename T> T cube(T x) {return x * x * x;}
  bool near(double x, double y, double delta);

  std::vector<std::complex<double> >
  solveQuadratic(double a, double b, double c);

  namespace SCurve {
    double distance(double t, double v, double a, double j);
    double velocity(double t, double a, double j);
    double acceleration(double t, double j);

    double timeAtDistance(double d, double v, double a, double j, double maxT,
                          double tolerance = 1e-7);
    double timeAtVelocity(double iV, double tV, double a, double j,
                          double tolerance = 1e-20);
  }
}
