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

#include "RenderJob.h"

#include <camotics/contour/MarchingCubes.h>
#include <camotics/contour/CorrectedMC33.h>
#include <camotics/contour/CubicalMarchingSquares.h>

#include <cbang/Exception.h>
#include <cbang/time/Timer.h>
#include <cbang/Catch.h>

using namespace cb;
using namespace CAMotics;


RenderJob::RenderJob(Condition &condition, FieldFunction &func, RenderMode mode,
                     GridTreeRef &tree) :
  condition(condition), func(func), tree(tree) {
  switch (mode) {
  case RenderMode::MCUBES_MODE: generator = new MarchingCubes;          break;
  case RenderMode::CMS_MODE:    generator = new CubicalMarchingSquares; break;
  default: THROW("Invalid or unsupported render mode " << mode);
  }
}


void RenderJob::run() {
  try {
    generator->run(func, tree);
  } CATCH_WARNING;

  condition.signal();
}


void RenderJob::stop() {
  if (!generator.isNull()) generator->interrupt();
}
