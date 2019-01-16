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


namespace CAMotics {
  class Path;

  class AnnealState {
  public:
    typedef std::vector<Path> paths_t;
    const paths_t &paths;

    std::vector<unsigned> index;
    std::vector<bool> flip;
    double cost;

    AnnealState(const paths_t &paths);

    AnnealState &operator=(const AnnealState &o);

    void flipIndex(unsigned i);

    double computeCost(unsigned first, unsigned second) const;
    double computeCost() const;

    double swapDelta(unsigned first, unsigned second);
    double reverseDelta(unsigned first, unsigned second);
    double flipDelta(unsigned i);
    void acceptSwap(unsigned first, unsigned second);
    void acceptReverse(unsigned first, unsigned second);
    void acceptFlip(unsigned i);
  };
}
