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
#include "PlannerCommand.h"

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
    double lastExitVel;

    typedef std::list<cb::SmartPointer<PlannerCommand> > cmds_t;
    cmds_t cmds;
    cmds_t output;

    uint64_t nextID;
    int line;

  public:
    LinePlanner(const PlannerConfig &config) :
    config(config), lastExitVel(0), nextID(1), line(-1) {}

    bool hasMove() const;
    void next(cb::JSON::Sink &sink);
    void release(uint64_t id);
    void restart(uint64_t id, const Axes &position);

    // From MachineInterface
    //void reset();
    void start();
    void end();
    void setSpeed(double speed, spin_mode_t mode, double max);
    void setTool(unsigned tool);
    //double input(unsigned port, input_mode_t mode, double timeout, bool error)
    //void output(unsigned port, double value, bool sync)
    void dwell(double seconds);
    void move(const Axes &axes, bool rapid);
    //void arc(const cb::Vector3D &offset, double angle, plane_t plane);
    void pause(bool optional);
    //bool synchronize(double timeout);
    //void abort();
    void setLocation(const cb::LocationRange &location);

  protected:
    void push(const cb::SmartPointer<PlannerCommand> &cmd);
    bool isFinal(cmds_t::const_iterator it) const;
    void plan(cmds_t::iterator it);
    bool planOne(cmds_t::iterator it);

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
