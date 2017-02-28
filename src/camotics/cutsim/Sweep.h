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

#ifndef CAMOTICS_SWEEP_H
#define CAMOTICS_SWEEP_H

#include <camotics/Geom.h>

#include <vector>


namespace CAMotics {
  class Move;

  class Sweep {
  public:
    virtual ~Sweep() {} // Compiler needs this

    void getBBoxes(const Vector3R &start, const Vector3R &end,
                   std::vector<Rectangle3R> &bboxes, real radius,
                   real length, real tolerance = 0.01) const;

    virtual void getBBoxes(const Vector3R &start, const Vector3R &end,
                           std::vector<Rectangle3R> &bboxes,
                           real tolerance = 0.01) const = 0;
    virtual bool intersects(const Move &move, const Rectangle3R &box) const
    {return false;}
    virtual real depth(const Vector3R &start, const Vector3R &end,
                       const Vector3R &p) const = 0;
  };
}

#endif // CAMOTICS_SWEEP_H
