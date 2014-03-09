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

#ifndef OPENSCAM_CONTOUR_GENERATOR_H
#define OPENSCAM_CONTOUR_GENERATOR_H

#include "Surface.h"
#include "FieldFunction.h"

#include <cbang/SmartPointer.h>
#include <cbang/os/Mutex.h>
#include <cbang/util/SmartLock.h>


namespace OpenSCAM {
  class ContourGenerator : cb::Mutex {
  protected:
    bool interrupt;

    double lastTime;
    double progress;
    double eta;

  public:
    ContourGenerator() : interrupt(false), lastTime(0), progress(0), eta(0) {}

    void stop() {interrupt = true;}
    void updateProgress(double progress);
    double getProgress() const {cb::SmartLock lock(this); return progress;}
    double getETA() const {cb::SmartLock lock(this); return eta;}

    virtual cb::SmartPointer<Surface> getSurface() = 0;
    virtual void run(FieldFunction &func, const Rectangle3R &bbox,
                     real resolution) = 0;
  };
}

#endif // OPENSCAM_CONTOUR_GENERATOR_H

