/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "PlannerSink.h"
#include "PlannerConfig.h"

#include <gcode/machine/MachineAdapter.h>

#include <cbang/SmartPointer.h>
#include <cbang/geom/Vector.h>

#include <list>


namespace GCode {
  class LinePlanner : public MachineAdapter {
    PlannerSink &sink;
    const PlannerConfig config;

    cb::Vector4D position;
    cb::Vector4D execPos;
    cb::Vector4D lastUnit;
    double lastExitVel;


    struct Point {
      cb::Vector4D position;
      double length;

      double entryVel;
      double exitVel;
      double deltaV;

      double maxVel;
      double maxAccel;
      double maxJerk;

      double times[7];

      Point();
    };


    typedef std::list<Point> points_t;
    points_t points;

  public:
    LinePlanner(PlannerSink &sink, const PlannerConfig &config) :
      sink(sink), config(config) {}

    // From MachineInterface
    void start();
    void end();
    void move(const Axes &axes, bool rapid);

  protected:
    void exec(const Point &p);
    void exec();
    bool isFinal(points_t::iterator it) const;
    bool plan(points_t::iterator it);
    void backplan(points_t::iterator it);
  };
}
