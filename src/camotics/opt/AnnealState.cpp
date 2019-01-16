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

#include "AnnealState.h"
#include "Path.h"

#include <cbang/geom/Vector.h>


using namespace std;
using namespace cb;
using namespace CAMotics;


AnnealState::AnnealState(const paths_t &paths) : paths(paths) {
  index.clear();
  flip.clear();

  for (unsigned i = 0; i < paths.size(); i++) {
    index.push_back(i);
    flip.push_back(false);
  }

  cost = computeCost();
}


AnnealState &AnnealState::operator=(const AnnealState &o) {
  index = o.index;
  flip = o.flip;
  cost = o.cost;

  return *this;
}


void AnnealState::flipIndex(unsigned i) {
  flip[index[i]] = !flip[index[i]];
}


double AnnealState::computeCost(unsigned first, unsigned second) const {
  const Path &path1 = paths.at(index[first]);
  const Path &path2 = paths.at(index[second]);

  const Vector3D &p1 =
    flip[index[first]] ? path1.startPoint() : path1.endPoint();
  const Vector3D &p2 =
    flip[index[second]] ? path2.endPoint() : path2.startPoint();

  return p1.distance(p2);
}


double AnnealState::computeCost() const {
  double cost = 0;

  for (unsigned i = 0; i < index.size() - 1; i++)
    cost += computeCost(i, i + 1);

  return cost;
}


double AnnealState::swapDelta(unsigned first, unsigned second) {
  double delta = 0;

  if (first) {
    delta -= computeCost(first - 1, first);
    delta += computeCost(first - 1, second);
  }

  if (second < index.size() - 1) {
    delta -= computeCost(second, second + 1);
    delta += computeCost(first, second + 1);
  }

  if (first + 1 != second) {
    if (first < index.size() - 1) {
      delta -= computeCost(first, first + 1);
      delta += computeCost(second, first + 1);
    }

    if (second) {
      delta -= computeCost(second - 1, second);
      delta += computeCost(second - 1, first);
    }

  } else {
    delta -= computeCost(first, second);
    delta += computeCost(second, first);
  }

  return delta;
}


double AnnealState::reverseDelta(unsigned first, unsigned second) {
  double delta = 0;

  if (first) {
    delta -= computeCost(first - 1, first);
    flipIndex(second);
    delta += computeCost(first - 1, second);
    flipIndex(second);
  }

  if (second < index.size() - 1) {
    delta -= computeCost(second, second + 1);
    flipIndex(first);
    delta += computeCost(first, second + 1);
    flipIndex(first);
  }

  return delta;
}


double AnnealState::flipDelta(unsigned i) {
  double delta = 0;

  if (i) delta -= computeCost(i - 1, i);
  if (i < index.size() - 1) delta -= computeCost(i, i + 1);

  flipIndex(i);
  if (i) delta += computeCost(i - 1, i);
  if (i < index.size() - 1) delta += computeCost(i, i + 1);
  flipIndex(i);

  return delta;
}


void AnnealState::acceptSwap(unsigned first, unsigned second) {
  std::swap(index[first], index[second]);
}


void AnnealState::acceptReverse(unsigned first, unsigned second) {
  for (unsigned i = first; i <= second; i++) flipIndex(i);

  unsigned length = second - first + 1;
  unsigned half = length / 2;

  for (unsigned i = 0; i < half; i++)
    std::swap(index[first + i], index[second - i]);
}


void AnnealState::acceptFlip(unsigned i) {
  flipIndex(i);
}
