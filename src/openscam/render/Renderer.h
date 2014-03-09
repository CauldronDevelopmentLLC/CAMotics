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

#ifndef OPENSCAM_RENDERER_H
#define OPENSCAM_RENDERER_H

#include "RenderMode.h"

#include <openscam/cutsim/CutWorkpiece.h>
#include <openscam/contour/Surface.h>

#include <cbang/os/Thread.h>
#include <cbang/os/Mutex.h>
#include <cbang/SmartPointer.h>
#include <cbang/util/SmartLock.h>


namespace cb {class Options;}

namespace OpenSCAM {
  class Renderer : public cb::Thread, public cb::Mutex {
  protected:
    cb::SmartPointer<CutWorkpiece> cutWorkpiece;
    cb::SmartPointer<Surface> surface;
    cb::SmartPointer<Surface> nextSurface;

    RenderMode mode;
    real resolution;
    unsigned threads;
    bool autoRender;
    bool smooth;

    double progress;
    double eta;

    bool dirty;
    bool interrupt;
    bool updated;

  public:
    Renderer(cb::Options &options);

    void setCutWorkpiece(const cb::SmartPointer<CutWorkpiece> &cutWorkpiece);
    void setMode(RenderMode mode) {cb::SmartLock lock(this); this->mode = mode;}
    RenderMode getMode() const {cb::SmartLock lock(this); return mode;}
    real getResolution() const {cb::SmartLock lock(this); return resolution;}
    void setResolution(real x) {cb::SmartLock lock(this); resolution = x;}
    bool getSmooth() const {return smooth;}
    void setSmooth(bool x) {smooth = x;}
    void toggleSmooth() {smooth = !smooth;}
    uint64_t getSamples() const;
    cb::SmartPointer<Surface> getSurface();
    void releaseSurface() {surface.release();}
    double getProgress() {cb::SmartLock lock(this); return progress;}
    double getETA() {cb::SmartLock lock(this); return eta;}

    void reload() {surface.release(); interrupt = true; dirty = true;}
    void stopRender() {interrupt = true;}
    void stop() {Thread::stop(); interrupt = true;}
    bool wasUpdated() {bool x = updated; updated = false; return x;}

    void drawBB() {if (!cutWorkpiece.isNull()) cutWorkpiece->drawBB();}

    // From Thread
    void run();
  };
}

#endif // OPENSCAM_RENDERER_H

