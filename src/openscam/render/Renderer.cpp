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

#include "Renderer.h"

#include "RenderJob.h"

#include <openscam/contour/CompositeSurface.h>

#include <cbang/log/Logger.h>
#include <cbang/time/TimeInterval.h>
#include <cbang/time/Timer.h>
#include <cbang/os/SystemInfo.h>
#include <cbang/config/Options.h>
#include <cbang/String.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


Renderer::Renderer(cb::Options &options) :
  mode(RenderMode::MCUBES_MODE), resolution(1),
  threads(SystemInfo::instance().getCPUCount()), autoRender(true), smooth(true),
  progress(0), eta(0), dirty(true), interrupt(false), updated(false) {

  options.pushCategory("Renderer");
  options.addTarget("threads", threads, "The number of render threads");
  options.addTarget("auto_render", autoRender,
                    "Automatically render when simulation changes");
  options.addTarget("smooth", smooth, "Smooth the rendered results by "
                    "averaging adjacent vertex normals.");
  options.popCategory();
}


void Renderer::setCutWorkpiece(const SmartPointer<CutWorkpiece> &cutWorkpiece) {
  SmartLock lock(this);
  if (autoRender) dirty = interrupt = true;
  this->cutWorkpiece = cutWorkpiece;
}


uint64_t Renderer::getSamples() const {
  return cutWorkpiece.isNull() ? 0 : cutWorkpiece->getSampleCount();
}


SmartPointer<Surface> Renderer::getSurface() {
  SmartLock lock(this);

  if (!nextSurface.isNull()) {
    surface = nextSurface;
    nextSurface = 0;
  }

  return surface;
}


static void boxSplit(vector<Rectangle3R> &boxes, Rectangle3R box,
                     unsigned count) {
  if (count < 2) {
    boxes.push_back(box);
    return;
  }

  Rectangle3R left(box);
  Rectangle3R right(box);

  // Split on largest dimension (reduces overlap)
  if (box.getLength() <= box.getWidth() && box.getHeight() <= box.getWidth())
    right.rmin.x() = left.rmax.x() = (box.rmin.x() + box.rmax.x()) / 2;

  else if (box.getWidth() <= box.getLength() &&
           box.getHeight() <= box.getLength())
    right.rmin.y() = left.rmax.y() = (box.rmin.y() + box.rmax.y()) / 2;

  else right.rmin.z() = left.rmax.z() = (box.rmin.z() + box.rmax.z()) / 2;

  boxSplit(boxes, left, count - 2);
  boxSplit(boxes, right, count - 2);
}


void Renderer::run() {
  while (!shouldShutdown()) {
    interrupt = false;
    if (!dirty) {
      Timer::sleep(0.1);
      continue;
    }
    dirty = false;
    lock();
    SmartPointer<CutWorkpiece> cutWorkpiece = this->cutWorkpiece;
    unlock();
    if (cutWorkpiece.isNull() || !cutWorkpiece->isValid()) continue;

    double start = Timer::now();

    cutWorkpiece->clearSampleCount();
    cutWorkpiece->getToolSweep()->clearHitTests();

    // Increase bounds a little
    Rectangle3R bbox = cutWorkpiece->getBounds();
    real off = 2 * resolution - 0.00001;
    bbox = bbox.grow(Vector3R(off, off, off));

    // Divide work
    vector<Rectangle3R> jobBoxes;
    boxSplit(jobBoxes, bbox, threads);
    unsigned totalJobCount = jobBoxes.size();

    LOG_INFO(1, "Computing surface bounded by " << bbox << " at "
             << resolution << " grid resolution in " << jobBoxes.size()
             << " boxes");

    // Run jobs
    lock();
    double resolution = this->resolution;
    RenderMode mode = this->mode;
    unlock();

    SmartPointer<CompositeSurface> surface = new CompositeSurface;
    typedef list<SmartPointer<RenderJob> > jobs_t;
    jobs_t jobs;
    while (!shouldShutdown() && !interrupt &&
           !(jobBoxes.empty() && jobs.empty())) {
      // Start new jobs
      while (!jobBoxes.empty() && jobs.size() < threads) {
        Rectangle3R jobBox = jobBoxes.back();
        jobBoxes.pop_back();

        SmartPointer<RenderJob> job =
          new RenderJob(cutWorkpiece, mode, resolution, jobBox);
        job->start();
        jobs.push_back(job);
      }

      // Reap completed jobs
      for (jobs_t::iterator it = jobs.begin(); it != jobs.end();)
        if ((*it)->getState() == Thread::THREAD_DONE) {
          (*it)->join();
          surface->add((*it)->getSurface());
          it = jobs.erase(it);
        } else it++;

      // Update Progress
      lock();
      progress = 0;
      // Running jobs
      for (jobs_t::iterator it = jobs.begin(); it != jobs.end(); it++)
        progress += (*it)->getProgress();
      // Completed jobs
      progress += totalJobCount - jobBoxes.size() - jobs.size();
      progress /= totalJobCount;

      double delta = (Timer::now() - start);
      if (progress && 1 < delta) eta = delta / progress - delta;
      else eta = 0;
      unlock();

      Timer::sleep(0.1);
    }

    // Clean up remaining jobs in case of an early exit
    for (jobs_t::iterator it = jobs.begin(); it != jobs.end(); it++)
      (*it)->join();

    progress = 0;
    eta = 0;

    if (shouldShutdown() || interrupt) continue;

    if (smooth) {
      LOG_INFO(1, "Smoothing");
      surface->smooth();
    }

    // Done
    double delta = Timer::now() - start;
    LOG_INFO(1, "Time: " << TimeInterval(delta)
             << " Triangles: " << surface->getCount()
             << " Triangles/sec: "
             << String::printf("%0.2f", surface->getCount() / delta)
             << " Resolution: " << resolution);

    lock();
    nextSurface = surface;
    unlock();

    updated = true;
  }
}
