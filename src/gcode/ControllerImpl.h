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
#include "Addresses.h"

#include <gcode/machine/MachineUnitAdapter.h>

#include <map>
#include <string>


namespace GCode {
  class ControllerImpl : public Controller, public VarTypes, public ModalGroup,
    public MachineEnum {
  public:
    typedef enum {
      DIR_OFF,
      DIR_CLOCKWISE,
      DIR_COUNTERCLOCKWISE,
    } dir_t;


    typedef enum {
      EXACT_PATH_MODE,
      EXACT_STOP_MODE,
      CONTINUOUS_MODE,
    } path_mode_t;


    typedef enum {
      RETURN_TO_R,
      RETURN_TO_OLD_Z,
    } return_mode_t;


  protected:
    MachineUnitAdapter machine;
    ToolTable tools;

    // Block variables
    static const int MAX_VAR = 26;
    double vars[MAX_VAR];
    bool used[MAX_VAR];
    cb::SmartPointer<Entity> varExprs[MAX_VAR];

    // Numbered and named parameters
    double params[MAX_ADDRESS];
    typedef std::map<std::string, double> named_t;
    named_t named;

    // State variables
    MachineInterface::plane_t plane;
    bool latheDiameterMode;          // TODO unsupported
    bool cutterRadiusComp;           // TODO unsupported
    bool toolLengthComp;             // TODO unsupported
    path_mode_t pathMode;            // TODO unsupported
    return_mode_t returnMode;
    double motionBlendingTolerance;  // TODO unsupported
    double naiveCamTolerance;        // TODO unsupported
    bool modalMotion;                // TODO unsupported
    bool incrementalDistanceMode;
    bool arcIncrementalDistanceMode;
    bool moveInAbsoluteCoords;
    MachineInterface::feed_mode_t feedMode;
    MachineInterface::spin_mode_t spinMode;
    dir_t spindleDir;
    double speed;
    double maxSpindleSpeed;

  public:
    ControllerImpl(MachineInterface &machine,
                   const ToolTable &tools = ToolTable());

    // Vars
    double getVar(char c) const;
    const cb::SmartPointer<Entity> &getVarExpr(char c) const;
    double getOffsetVar(char c, bool absolute) const;
    std::string getVarGroupStr(const char *group, bool usedOnly = true) const;
    static VarTypes::enum_t getVarType(char letter);

    // Spindle
    void setSpindleDir(dir_t dir);

    // Coolant
    void setMistCoolant(bool enable);
    void setFloodCoolant(bool enable);

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
    double getAxisAbsolutePosition(char axis) const;
    void setAxisAbsolutePosition(char axis, double pos);
    double getAxisOffset(char axis) const;
    double getAxisPosition(char axis) const;
    Axes getAbsolutePosition() const;
    void setAxisPosition(char axis, double pos);
    void setAbsolutePosition(const Axes &axes);
    Axes getNextPosition(int vars, bool absolute) const;

    // Move
    void doMove(const Axes &pos, bool rapid);
    void makeMove(int vars, bool rapid, bool absolute);
    void moveAxis(char axis, double value, bool rapid);
    void arc(int vars, bool clockwise);
    void straightProbe(int vars, bool towardWorkpiece, bool signalError);
    void seek(int vars, bool active, bool error);
    void drill(int vars, bool dwell, bool feedOut, bool spindleStop);

    // Dwell
    void dwell(double seconds);

    // Coordindate System
    void setCoordSystemRotation(unsigned cs, double rotation);
    void setCoordSystemOffset(unsigned cs, char axis, bool relative);
    void setCoordSystem(int vars, bool relative);

    // Tool
    const Tool &getTool(unsigned tool) const {return tools.get(tool);}
    unsigned getCurrentTool() const {return (unsigned)get(TOOL_NUMBER);}
    void setToolTable(int vars, bool relative);
    void toolChange(bool manual = false);
    void loadToolOffsets(unsigned tool);
    void loadToolVarOffsets(int vars);

    // Predefined locations
    void storePredefined1();
    void storePredefined2();
    void loadPredefined1(int vars);
    void loadPredefined2(int vars);

    // Offsets
    void setGlobalOffsets(int vars);
    void resetGlobalOffsets(bool clearMemory);
    void restoreGlobalOffsets();

    // Homing
    void setAxisHomed(int vars, bool homed);

    // Compensation
    void setCutterRadiusComp(int vars, bool left, bool dynamic);

    // End program
    void end();

    // From Controller
    double get(unsigned addr) const;
    void set(unsigned addr, double value);
    bool has(const std::string &name) const;
    double get(const std::string &name) const;
    void set(const std::string &name, double value);

    void setVar(char c, double value);
    void setVarExpr(char c, const cb::SmartPointer<Entity> &entity);

    void setLocation(const cb::LocationRange &location);
    void setFeed(double feed);
    void setSpeed(double speed);
    void setTool(unsigned tool);

    void newBlock();
    void execute(const Code &code, int vars);
  };
}
