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


namespace GCode {
  class PlanCommand {
  public:
    cb::SmartPointer<PlanCommand> next;
    cb::SmartPointer<PlanCommand> prev;

    typedef enum {
      PLAN_START,
      PLAN_END,
      PLAN_SPEED,
      PLAN_TOOL,
      PLAN_DWELL,
      PLAN_PAUSE,
      PLAN_MOVE,
    } type_t;

    virtual ~PlanCommand() {}

    virtual type_t getType() const = 0;
    virtual double getMaxEntryVelocity(const Axes &dir) const {return 0;}
    virtual bool plan() {return true;}
    virtual void write(PlannerSink &sink) = 0;
  };


  class PlanStartCommand : public PlanCommand {
  public:
    // From PlanCommand
    type_t getType() const {return PLAN_START;}
    void write(PlannerSink &sink) {sink.start();}
  };


  class PlanEndCommand : public PlanCommand {
  public:
    // From PlanCommand
    type_t getType() const {return PLAN_END;}
    void write(PlannerSink &sink) {sink.end();}
  };


  class PlanSpeedCommand : public PlanCommand {
    double speed;

  public:
    PlanSpeedCommand(double speed) : speed(speed) {}

    // From PlanCommand
    type_t getType() const {return PLAN_SPEED;}
    double getMaxEntryVelocity(const Axes &dir) const
    {return next->getMaxEntryVelocity(dir);}
    void write(PlannerSink &sink) {sink.setSpeed(speed);}
  };


  class PlanToolCommand : public PlanCommand {
    unsigned tool;

  public:
    PlanToolCommand(unsigned tool) : tool(tool) {}

    // From PlanCommand
    type_t getType() const {return PLAN_TOOL;}
    void write(PlannerSink &sink) {sink.setTool(tool);}
  };


  class PlanDwellCommand : public PlanCommand {
    double seconds;

  public:
    PlanDwellCommand(double seconds) : seconds(seconds) {}

    // From PlanCommand
    type_t getType() const {return PLAN_DWELL;}
    void write(PlannerSink &sink) {sink.dwell(seconds);}
  };


  class PlanPauseCommand : public PlanCommand {
    bool optional;

  public:
    PlanPauseCommand(bool optional) : optional(optional) {}

    // From PlanCommand
    type_t getType() const {return PLAN_PAUSE;}
    void write(PlannerSink &sink) {sink.pause(optional);}
  };


  class PlanMoveCommand : public PlanCommand {
    const PlannerConfig &config;

    Axes target;
    double maxVelocity;

    double length;
    Axes dir;
    Axes velocity;

  public:
    PlanMoveCommand(const PlannerConfig &config, const Axes &start,
                    const Axes &end, double feed);

    double computeJunctionVelocity(const Axes &dir) const;

    // From PlanCommand
    type_t getType() const {return PLAN_MOVE;}
    double getMaxEntryVelocity(const Axes &dir) const;
    void write(PlannerSink &sink) {sink.move(target, maxVelocity);}
  };
}
