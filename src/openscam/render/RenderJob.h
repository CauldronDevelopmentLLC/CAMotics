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

#ifndef OPENSCAM_RENDER_JOB_H
#define OPENSCAM_RENDER_JOB_H

#include "RenderMode.h"

#include <openscam/contour/ContourGenerator.h>

#include <cbang/os/Thread.h>
#include <cbang/os/Mutex.h>
#include <cbang/util/SmartLock.h>

namespace OpenSCAM {
  class RenderJob :
    public cb::Thread, public cb::Mutex {

    cb::SmartPointer<ContourGenerator> generator;

    FieldFunction &func;
    real resolution;
    Rectangle3R bbox;

  public:
    RenderJob(FieldFunction &func, RenderMode mode, real resolution,
              const Rectangle3R &bbox);

    cb::SmartPointer<Surface> getSurface() const
    {return generator->getSurface();}

    double getProgress() {return generator->getProgress();}
    double getETA() {return generator->getETA();}

    // From Thread
    void run();
    void stop();
  };
}

#endif // OPENSCAM_RENDER_JOB_H

