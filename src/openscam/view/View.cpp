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

#include "View.h"

#include "GL.h"

#include <cbang/Math.h>
#include <cbang/log/Logger.h>
#include <cbang/config/Options.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


View::View(ValueSet &valueSet) :
  values(valueSet), path(new ToolPathView(valueSet)), workpiece(new CuboidView),
  flags(SHOW_PATH_FLAG | SHOW_TOOL_FLAG | SHOW_SURFACE_FLAG |
        SHOW_WORKPIECE_BOUNDS_FLAG),
  fps(0), speed(1), reverse(true), frames(0), frameTimes(0), lastTime(0)  {

  values.add("play_speed", speed);
  values.add("play_direction", reverse);
  values.add("view_flags", flags);
}


void View::incSpeed() {
  if (speed < (1 << 16)) speed <<= 1;
  values.updated();
}


void View::decSpeed() {
  if (1 < speed) speed >>= 1;
  values.updated();
}


void View::frameTime(double delta) {
  frames++;
  frameTimes += delta;
}


void View::setToolPath(const SmartPointer<ToolPath> &toolPath) {
  path->setPath(toolPath);
}


void View::setWorkpiece(const Rectangle3R &bounds) {
  workpiece->setBounds(bounds);
}


void View::setSurface(const cb::SmartPointer<Surface> &surface) {
  this->surface = surface;
}


bool View::update() {
  bool changed = false;

  // Animate
  if (isFlagSet(PLAY_FLAG)) {
    double now = Timer::now();
    if (lastTime) {
      double delta = (now - lastTime) * speed;
      if (reverse) path->decTime(delta);
      else path->incTime(delta);

      if (reverse) {
        if (path->getTime() == 0) {
          if (!isFlagSet(LOOP_FLAG)) clearFlag(PLAY_FLAG);
          changeDirection();
        }

      } else if (path->getTime() == path->getTotalTime()) {
        if (!isFlagSet(LOOP_FLAG)) clearFlag(PLAY_FLAG);
        changeDirection();
      }

      changed = true;
    }
    lastTime = now;

  } else lastTime = 0;

  // FPS
  if (fpsTimer.every(1) && frames) {
    View::fps = frames / frameTimes;
    frames = 0;
    frameTimes = 0;
  }

  return changed;
}
