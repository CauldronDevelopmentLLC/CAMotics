/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef OPENSCAM_SURFACE_THREAD_H
#define OPENSCAM_SURFACE_THREAD_H

#include "CutSimThread.h"

#include <openscam/Geom.h>
#include <openscam/cutsim/ToolPath.h>
#include <openscam/contour/Surface.h>


namespace OpenSCAM {
  class SurfaceThread : public CutSimThread {
    cb::SmartPointer<ToolPath> path;
    Rectangle3R bounds;
    double resolution;
    double time;
    bool smooth;
    cb::SmartPointer<Surface> surface;

  public:
    SurfaceThread(int event, QWidget *parent,
                  const cb::SmartPointer<CutSim> &cutSim,
                  const cb::SmartPointer<ToolPath> &path,
                  const Rectangle3R &bounds, double resolution, double time,
                  bool smooth) :
      CutSimThread(event, parent, cutSim), path(path), bounds(bounds),
      resolution(resolution), time(time), smooth(smooth) {}

    const cb::SmartPointer<Surface> &getSurface() const {return surface;}

    // From cb::Thread
    void run();
  };
}

#endif // OPENSCAM_SURFACE_THREAD_H

