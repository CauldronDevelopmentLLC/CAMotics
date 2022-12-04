/******************************************************************************\

  CAMotics is an Open-Source simulation and CAM software.
  Copyright (C) 2011-2019 Joseph Coffland <joseph@cauldrondevelopment.com>

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
  class LineCommand;


  class LinePlanner : public MachineState {
    PlannerConfig config;

    // Move state
    double lastExitVel;
    bool seeking;
    bool firstMove;

    typedef List<PlannerCommand> cmd_t;
    cmd_t pre;
    cmd_t cmds;
    cmd_t out;

    uint64_t nextID = 1;

    std::string filename;
    int line;
    double speed;
    bool rapidAutoOff;

    double time;
    double distance;

  public:
    LinePlanner(const PlannerConfig &config);
    LinePlanner();

    double getTime() const {return time;}
    double getDistance() const {return distance;}

    void reset();
    void setConfig(const PlannerConfig &config);
    void checkSoftLimits(const Axes &p);
    bool isEmpty() const;
    bool hasMove() const;
    const PlannerCommand &next();
    uint64_t next(cb::JSON::Sink &sink);
    uint64_t next(MachineInterface &machine);
    void setActive(uint64_t id);
    void stop();
    bool restart(uint64_t id, const Axes &position);
    void dumpQueue(cb::JSON::Sink &sink);

    // From MachineInterface
    void start();
    void end();
    void setSpeed(double speed);
    void setSpinMode(spin_mode_t mode, double max);
    void setPathMode(path_mode_t mode, double motionBlending, double naiveCAM);
    void input(port_t port, input_mode_t mode, double timeout);
    void seek(port_t port, bool active, bool error);
    void output(port_t port, double value);
    void dwell(double seconds);
    void move(const Axes &position, int axes, bool rapid, double time);
    void arc(const cb::Vector3D &offset, const cb::Vector3D &target,
             double angle, plane_t plane)
      {CBANG_THROW("LinePlanner does not implement arc()");}
    void pause(pause_t type);
    void set(const std::string &name, double value, Units units);
    void setLocation(const cb::LocationRange &location);
    void comment(const std::string &s) const {} // TODO
    void message(const std::string &s);

  protected:
    uint64_t getNextID();
    bool idLess(uint64_t a, uint64_t b) const;

    template <typename T>
    void pushSetCommand(const std::string &name, const T &value);
    virtual void push(PlannerCommand *cmd);
    bool merge(LineCommand *next, LineCommand *prev, double lastSpeed);
    double computeMaxAccel(const cb::Vector3D &v) const;
    double computeJunctionVelocity(const cb::Vector3D &v, double radius) const;
    unsigned blendSegments(double arcError, double arcAngle, double radius);
    void blend(LineCommand *next, LineCommand *prev, double lastSpeed,
               int lastLine);
    void enqueue(LineCommand *lc, bool rapid);
    bool isFinal(PlannerCommand *cmd) const;
    void plan(PlannerCommand *cmd);
    bool planOne(PlannerCommand *cmd);

    bool isAccelLimited(double Vi, double Vt, double maxAccel,
                        double maxJerk) const;
    double peakAccelFromDeltaV(double Vi, double Vt, double jerk) const;
    double peakAccelFromLength(double Vi, double jerk, double length) const;
    double peakVelocity(double Vi, double maxAccel, double maxJerk,
                        double length) const;
    double speedUp(PlannerCommand *cmd, double Vi) const;
    double computeLength(double Vi, double Vt, double maxAccel,
                         double maxJerk) const;
    double planVelocityTransition(double Vi, double Vt, double maxAccel,
                                  double maxJerk, double *times) const;
    double computeJunctionVelocity(const Axes &unitA, const Axes &unitB,
                                   double deviation, double accel) const;
  };
}
