/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
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

#include "Renderer.h"

#include "RenderJob.h"

#include <camotics/Grid.h>
#include <camotics/cutsim/CutWorkpiece.h>
#include <camotics/contour/CompositeSurface.h>

#include <cbang/log/Logger.h>
#include <cbang/time/TimeInterval.h>
#include <cbang/time/Timer.h>
#include <cbang/String.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


cb::SmartPointer<Surface>
Renderer::render(CutWorkpiece &cutWorkpiece, unsigned threads,
                 double resolution, RenderMode mode) {
  // Setup
  task->begin();

  // Increase bounds a little
  Rectangle3R bbox = cutWorkpiece.getBounds();
  real off = resolution * 0.90;
  bbox = bbox.grow(Vector3R(off, off, off));

  // Divide work
  vector<Grid> jobBoxes;
  Grid(bbox, resolution).partition(jobBoxes, threads * 4);
  unsigned totalJobCount = jobBoxes.size();
  LOG_DEBUG(1, "Partitioned in to " << totalJobCount << " jobs");

  LOG_INFO(1, "Computing surface bounded by " << bbox << " at "
           << resolution << " grid resolution");

  // Run jobs
  SmartPointer<CompositeSurface> surface = new CompositeSurface;
  typedef list<SmartPointer<RenderJob> > jobs_t;
  jobs_t jobs;
  double lastUpdate = 0;
  while (!task->shouldQuit() && !(jobBoxes.empty() && jobs.empty())) {

    // Start new jobs
    while (!jobBoxes.empty() && jobs.size() < threads) {
      SmartPointer<RenderJob> job =
        new RenderJob(cutWorkpiece, mode, jobBoxes.back());
      job->start();
      jobs.push_back(job);
      jobBoxes.pop_back();
    }


    // Reap completed jobs
    jobs_t::iterator it;
    for (it = jobs.begin(); it != jobs.end() && !task->shouldQuit();)
      if ((*it)->getState() == Thread::THREAD_DONE) {
        (*it)->join();
        surface->add((*it)->getSurface());
        it = jobs.erase(it);
      } else it++;


    // Update Progress
    double progress = 0;

    // Add running jobs
    for (it = jobs.begin(); it != jobs.end() && !task->shouldQuit(); it++)
      progress += (*it)->getProgress();

    // Add completed jobs
    progress += totalJobCount - jobBoxes.size() - jobs.size();
    progress /= totalJobCount;

    task->update(progress, "Rendering surface");

    // Log progress
    double now = Timer::now();
    if (lastUpdate + 1 < now) {
      lastUpdate = now;
      LOG_INFO(2, String::printf("Progress: %0.2f%%", progress * 100)
               << " Time: " << TimeInterval(task->getTime())
               << " ETA: " << TimeInterval(task->getETA()));
    }

    // Sleep
    Timer::sleep(0.25);
  }


  // Clean up remaining jobs in case of an early exit
  for (jobs_t::iterator it = jobs.begin(); it != jobs.end(); it++)
    (*it)->join();

  if (shouldQuit()) {
    task->end();
    LOG_INFO(1, "Render aborted");
    return 0;
  }

  // Done
  double delta = task->end();
  LOG_INFO(1, "Time: " << TimeInterval(delta)
           << " Triangles: " << surface->getCount()
           << " Triangles/sec: "
           << String::printf("%0.2f", surface->getCount() / delta)
           << " Resolution: " << resolution);


  return surface;
}
