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

#include "Controller.h"
#include "VarTypes.h"
#include "ModalGroup.h"
#include "ToolTable.h"

#include <gcode/machine/MachineUnitAdapter.h>


namespace GCode {
  class ControllerImpl :
    public Controller, public VarTypesEnumerationBase,
    public ModalGroupEnumerationBase, public MachineEnum,
    public UnitsEnumerationBase {

    MachineUnitAdapter machine;
    ToolTable tools;

    // Block variables
    static const int MAX_VAR = 26;
    double varValues[MAX_VAR];

    Axes position; //< In current units

    typedef enum {
      SYNC_NONE,
      SYNC_SEEK,
      SYNC_PROBE,
      SYNC_INPUT,
    } synchronize_state_t;

    synchronize_state_t syncState;
    bool offsetParamChanged;

    // State variables
    unsigned currentMotionMode;
    MachineInterface::plane_t plane;
    bool latheDiameterMode;          // TODO unsupported
    bool cutterRadiusComp;           // TODO unsupported
    bool toolLengthComp;             // TODO unsupported
    path_mode_t pathMode;            // TODO unsupported
    return_mode_t returnMode;
    double motionBlendingTolerance;  // TODO unsupported
    double naiveCamTolerance;        // TODO unsupported
    bool incrementalDistanceMode;
    bool arcIncrementalDistanceMode;
    bool moveInAbsoluteCoords;
    dir_t spindleDir;
    double speed;

  public:
    ControllerImpl(MachineInterface &machine,
                   const ToolTable &tools = ToolTable());

    // Vars
    double getVar(char c) const;
    std::string getVarGroupStr(const char *group) const;
    static VarTypes::enum_t getVarType(char letter);

    // Units
    Units getUnits() const;
    void setUnits(Units units);

    // Feed & speed
    void setFeedMode(feed_mode_t mode);
    void setSpindleDir(dir_t dir);

    // Coolant
    void setMistCoolant(bool enable);
    void setFloodCoolant(bool enable);

    // I/O
    void digitalOutput(unsigned index, bool enable, bool synchronized);
    void input(unsigned index, bool digital, input_mode_t mode, double timeout);

    // Plane
    void setPlane(MachineInterface::plane_t plane);
    const char *getPlaneAxes() const;
    const char *getPlaneOffsets() const;
    char getPlaneXAxis() const {return getPlaneAxes()[0];}
    char getPlaneYAxis() const {return getPlaneAxes()[1];}
    char getPlaneZAxis() const {return getPlaneAxes()[2];}
    unsigned getPlaneXVarType() const {return getVarType(getPlaneXAxis());}
    unsigned getPlaneYVarType() const {return getVarType(getPlaneYAxis());}
    unsigned getPlaneZVarType() const {return getVarType(getPlaneZAxis());}

    // Position
    double getAxisOffset(char axis) const;
    double getAxisPosition(char axis) const;
    double getAxisAbsolutePosition(char axis) const;
    void setAxisAbsolutePosition(char axis, double pos);
    Axes getAbsolutePosition() const;
    void setAbsolutePosition(const Axes &axes);
    Axes getNextAbsolutePosition(int vars, bool incremental) const;

    // Move
    void doMove(const Axes &pos, bool rapid);
    void makeMove(int vars, bool rapid, bool incremental);
    void moveAxis(char axis, double value, bool rapid);
    void arc(int vars, bool clockwise);
    void straightProbe(int vars, bool towardWorkpiece, bool signalError);
    void seek(int vars, bool active, bool error);
    void drill(int vars, bool dwell, bool feedOut, bool spindleStop);

    // Dwell
    void dwell(double seconds);

    // Tool
    const Tool &getTool(unsigned tool) const {return tools.get(tool);}
    unsigned getCurrentTool() const {return (unsigned)get(TOOL_NUMBER);}
    void setToolTable(int vars, bool relative);
    void toolChange();
    void loadToolOffsets(unsigned tool);
    void loadToolVarOffsets(int vars);

    // Predefined locations
    void storePredefined(bool first);
    void loadPredefined(bool first, int vars);

    // Offsets
    void setCoordSystem(unsigned cs);
    void setCoordSystemOffsets(int vars, bool relative);
    double getAxisGlobalOffset(char axis) const;
    void setAxisGlobalOffset(char axis, double offset);
    void setGlobalOffsets(int vars);
    void resetGlobalOffsets(bool clear);
    void restoreGlobalOffsets();
    void updateOffsetParams();

    // Homing
    void setHomed(int vars, bool homed);

    // Compensation
    void setCutterRadiusComp(int vars, bool left, bool dynamic);

    // Program control
    void end();

    // Params
    double get(address_t addr, Units units) const;
    void set(address_t addr, double value, Units units);
    double get(const std::string &name, Units units) const;
    void set(const std::string &name, double value, Units units);

    // From Controller
    void message(const std::string &text);

    double get(address_t addr) const;
    void set(address_t addr, double value);
    bool has(const std::string &name) const;
    double get(const std::string &name) const;
    void set(const std::string &name, double value);
    void setVar(char c, double value);

    unsigned getCurrentMotionMode() {return currentMotionMode;}
    void setCurrentMotionMode(unsigned mode);

    bool isSynchronizing() const {return syncState != SYNC_NONE;}
    void synchronize(double result);

    void setLocation(const cb::LocationRange &location);
    void setFeed(double feed);
    void setSpeed(double speed);
    void setTool(unsigned tool);

    void startBlock();
    bool execute(const Code &code, int vars);
    void endBlock() {}
  };
}
