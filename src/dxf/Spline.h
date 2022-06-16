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


#include "Entity.h"

#include <vector>

namespace DXF {
  class Spline : public Entity {
    unsigned degree;
    unsigned flags; ///< See flag_t
    std::vector<cb::Vector3D> ctrlPts;
    std::vector<double> weights;
    std::vector<double> knots;

  public:
    typedef enum {
      SPLINE_FLAG_CLOSED   = 1 << 0, // Closed spline
      SPLINE_FLAG_PERIODIC = 1 << 1, // Periodic spline
      SPLINE_FLAG_RATIONAL = 1 << 2, // Rational spline
      SPLINE_FLAG_PLANAR   = 1 << 3, // Planar
      SPLINE_FLAG_LINEAR   = 1 << 4, // Linear (planar bit is also set)
    } flag_t;

    Spline(unsigned degree, unsigned flags) : degree(degree), flags(flags) {}

    unsigned getDegree() const {return degree;}
    unsigned getFlags() const {return flags;}
    bool isClosed()   const {return flags & SPLINE_FLAG_CLOSED;}
    bool isPeriodic() const {return flags & SPLINE_FLAG_PERIODIC;}
    bool isRational() const {return flags & SPLINE_FLAG_RATIONAL;}
    bool isPlanar()   const {return flags & SPLINE_FLAG_PLANAR;}
    bool isLinear()   const {return flags & SPLINE_FLAG_LINEAR;}
    const std::vector<cb::Vector3D> &getControlPoints() const {return ctrlPts;}
    const std::vector<double> &getWeights() const {return weights;}
    const std::vector<double> &getKnots() const {return knots;}

    // From Entity
    void addKnot(double k) {knots.push_back(k);}
    void addVertex(const cb::Vector3D &v, double weight = 1)
      {ctrlPts.push_back(v); weights.push_back(weight);}
    type_t getType() const {return DXF_SPLINE;}
  };
}
