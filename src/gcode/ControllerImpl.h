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

#include "Controller.h"
#include "ModalGroup.h"
#include "ToolTable.h"
#include "Plane.h"

#include <gcode/machine/MachineUnitAdapter.h>


namespace GCode {
  class ControllerImpl :
    public Controller, public ModalGroupEnumerationBase, public MachineEnum,
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
      SYNC_PAUSE,
    } synchronize_state_t;

    synchronize_state_t syncState = SYNC_NONE;
    bool offsetParamChanged       = false;

    // State variables
    unsigned currentMotionMode    = 10;
    bool absoluteCoords           = false;

    typedef struct {
      bool autoRestore;

      Units units;                     // G20, G21
      plane_t plane;                   // G17-G19.1
      bool cutterRadiusComp;           // G40-G42.1 TODO unsupported
      double toolDiameter;             // TOOL_DIAMETER
      double toolOrientation;          // TOOL_ORIENTATION
      bool incrementalDistanceMode;    // G90, G91
      feed_mode_t feedMode;            // G93, G94, G95
      unsigned coordSystem;            // G54-G59.3 CURRENT_COORD_SYSTEM
      bool toolLengthComp;             // G43, G43.1, G49
      return_mode_t returnMode;        // G98, G99
      spin_mode_t spinMode;            // G96, G97
      double spinMax;
      bool arcIncrementalDistanceMode; // G90.1, G91.1
      bool latheDiameterMode;          // G7, G8 TODO unsupported
      path_mode_t pathMode;            // G61, G61.1, G64
      double feed;                     // F
      double speed;                    // S
      dir_t spindleDir;                // M3-M5
      bool mist;                       // M7, M9
      bool flood;                      // M8, M9
      double speedOverride;            // M48, M51 TODO unsupported
      double feedOverride;             // M40, M50 TODO unsupported
      bool adaptiveFeed;               // M52 TODO unsupported
      bool feedHold;                   // M53 TODO unsupported
      double motionBlendingTolerance;  // G64 TODO unsupported
      double naiveCamTolerance;        // G64
      bool moveInAbsoluteCoords;       // G53
    } state_t;

    state_t state;
    std::vector<cb::SmartPointer<state_t> > stateStack;

  public:
    ControllerImpl(MachineInterface &machine,
                   const ToolTable &tools = ToolTable());

    // Vars
    double getVar(char c) const;
    std::string getVarGroupStr(const char *group) const;

    // Units
    Units getUnits() const;
    void setUnits(Units units);

    // Feed & speed
    void setFeedMode(feed_mode_t mode);
    void setSpindleDir(dir_t dir);
    void setSpinMode(spin_mode_t mode, double max = 0);

    // Coolant
    void setMistCoolant(bool enable);
    void setFloodCoolant(bool enable);

    // I/O
    void digitalOutput(unsigned index, bool enable, bool synchronized);
    void input(unsigned index, bool digital, input_mode_t mode, double timeout);

    // Plane
    void setPlane(plane_t plane);
    Plane getPlane() {return Plane(state.plane);}

    // Path mode
    void setPathMode(path_mode_t mode, double motionBlending = 0,
                     double naiveCAM = 0);

    // Position
    double getAxisCSOffset(char axis, unsigned cs = 0) const;
    double getAxisToolOffset(char axis) const;
    double getAxisGlobalOffset(char axis) const;
    double getAxisOffset(char axis) const;
    double getAxisPosition(char axis) const;
    double getAxisAbsolutePosition(char axis) const;
    void setAxisAbsolutePosition(char axis, double pos, Units units);
    Axes getAbsolutePosition() const;
    void setAbsolutePosition(const Axes &axes, Units units);
    Axes getNextAbsolutePosition(int vars, bool incremental) const;
    bool isPositionChanging(int vars, bool incremental) const;

    // Move
    void move(const Axes &pos, int axes, bool rapid);
    void makeMove(int axes, bool rapid, bool incremental);
    void moveAxis(char axis, double value, bool rapid);
    void linear(int vars, bool rapid);
    void arc(int vars, bool clockwise);
    void straightProbe(int vars, bool towardWorkpiece, bool signalError);
    void seek(int vars, bool active, bool error);
    void drill(int vars, bool dwell, bool feedOut, bool spindleStop);

    // Dwell & Pause
    void dwell(double seconds);
    void pause(pause_t type);

    // Tool
    Tool &getTool(unsigned tool) {return tools.get(tool);}
    unsigned getCurrentTool() const {return (unsigned)get(TOOL_NUMBER);}
    void setTools(int vars, bool relative, bool cs9);
    void toolChange();
    void loadToolOffsets(unsigned tool, bool add);
    void loadToolVarOffsets(int vars);

    // Predefined locations
    void storePredefined(bool first);
    void loadPredefined(bool first, int vars);

    // Offsets
    void setCoordSystem(unsigned cs);
    void setCoordSystemOffsets(int vars, bool relative);
    void setAxisGlobalOffset(char axis, double offset);
    void setGlobalOffsets(int vars, bool relative);
    void resetGlobalOffsets(bool clear);
    void restoreGlobalOffsets();
    void updateOffsetParams();

    // Homing
    void setHomed(int vars, bool homed);

    // Compensation
    void setCutterRadiusComp(int vars, bool left, bool dynamic);

    // Program control
    void end();
    void stop();

    // Params
    double get(address_t addr, Units units) const;
    void set(address_t addr, double value, Units units);
    double get(const std::string &name, Units units) const;
    void set(const std::string &name, double value, Units units);

    // Modal State
    void saveModalState(bool autoRestore);
    void clearSavedModalState();
    void restoreModalState();

    // From Controller
    void message(const std::string &text);

    double get(address_t addr) const;
    void set(address_t addr, double value);
    bool has(const std::string &name) const;
    double get(const std::string &name) const;
    void set(const std::string &name, double value);
    void clear(const std::string &name);
    void setVar(char c, double value);

    unsigned getCurrentMotionMode() {return currentMotionMode;}
    void setCurrentMotionMode(unsigned mode);

    bool isSynchronizing() const {return syncState != SYNC_NONE;}
    void synchronize(double result);

    void setLocation(const cb::LocationRange &location);
    void setFeed(double feed);
    void setSpeed(double speed);
    void setTool(unsigned tool);

    void pushScope();
    void popScope();

    void startBlock();
    bool execute(const Code &code, int vars);
    void endBlock();
  };
}
