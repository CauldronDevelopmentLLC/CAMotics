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
#include "List.h"

#include <gcode/machine/MachineState.h>

#include <cbang/SmartPointer.h>


namespace cb {namespace JSON {class Sink;}}


namespace GCode {
  class LinePlanner : public MachineState {
    PlannerConfig config;

    // Move state
    double lastExitVel;
    bool seeking;

    typedef List<PlannerCommand> cmd_t;
    cmd_t cmds;
    cmd_t out;

    uint64_t nextID;
    int line;

  public:
    LinePlanner();

    void setConfig(const PlannerConfig &config);
    bool isDone() const;
    bool hasMove() const;
    uint64_t next(cb::JSON::Sink &sink);
    void setActive(uint64_t id);
    bool restart(uint64_t id, const Axes &position);

    // From MachineInterface
    void start();
    void end();
    void setSpeed(double speed);
    void changeTool(unsigned tool);
    //void wait(port_t port, bool active, double timeout);
    void seek(port_t port, bool active, bool error);
    void output(port_t port, double value);
    void dwell(double seconds);
    void move(const Axes &axes, bool rapid);
    //void arc(const Axes &offset, double angle, plane_t plane);
    void pause(bool optional);
    void set(const std::string &name, double value);
    void setLocation(const cb::LocationRange &location);

  protected:
    template <typename T>
    void pushSetCommand(const std::string &name, const T &value);
    void push(PlannerCommand *cmd);
    bool isFinal(PlannerCommand *cmd) const;
    void plan(PlannerCommand *cmd);
    bool planOne(PlannerCommand *cmd);

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
    double computeJunctionVelocity(const Axes &unitA, const Axes &unitB,
                                   double deviation, double accel) const;
  };
}
