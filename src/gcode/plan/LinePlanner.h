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

#include "PlannerConfig.h"

#include <gcode/machine/MachineAdapter.h>

#include <cbang/SmartPointer.h>
#include <cbang/geom/Vector.h>

#include <list>


namespace cb {namespace JSON {class Sink;}}


namespace GCode {
  class LinePlanner : public MachineAdapter {
    const PlannerConfig config;

    // Move state
    cb::Vector4D position;
    cb::Vector4D lastUnit;
    double lastExitVel;

    // Output state
    cb::Vector4D outputPos;


    struct Point {
      uint64_t line;

      cb::Vector4D position;
      double length;

      double entryVel;
      double exitVel;
      double deltaV;

      double maxVel;
      double maxAccel;
      double maxJerk;

      double times[7];

      Point(uint64_t line);
    };


    typedef std::list<Point> points_t;
    points_t points;
    points_t output;

  public:
    LinePlanner(const PlannerConfig &config) : config(config) {}

    bool hasMove() const;
    void next(cb::JSON::Sink &sink);
    void release(uint64_t line);
    void restart(uint64_t line, double length);

    // From MachineInterface
    void start();
    void end();
    void move(const Axes &axes, bool rapid);

  protected:
    bool isFinal(points_t::const_iterator it) const;
    bool plan(points_t::iterator it);
    void backplan(points_t::iterator it);

    bool isAccelLimited(double Vi, double Vt, double maxAccel,
                        double maxJerk) const;
    double peakAccelFromDeltaV(double Vi, double Vt, double jerk) const;
    double peakAccelFromLength(double Vi, double jerk, double length) const;
    double peakVelocity(double Vi, double maxAccel, double maxJerk,
                        double length) const;
    double computeLength(double Vi, double Vt, double maxAccel,
                         double maxJerk) const;
    double planVelocityTransition(double Vi, double Vt, double maxAccel,
                                  double maxJerk, double *times) const;
    double computeJunctionVelocity(const cb::Vector4D &unitA,
                                   const cb::Vector4D &unitB,
                                   double deviation, double accel) const;
  };
}
