/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef OPENSCAM_CONTROLLER_H
#define OPENSCAM_CONTROLLER_H

#include "ToolTable.h"
#include "Addresses.h"
#include "CoordinateSystem.h"

#include <tplang/MachineInterface.h>
#include <tplang/MachineUnitAdapter.h>

#include <openscam/cutsim/Move.h>
#include <openscam/gcode/VarTypes.h>
#include <openscam/gcode/ModalGroup.h>

#include <cbang/LocationRange.h>
#include <cbang/SmartPointer.h>

#include <map>
#include <string>


namespace OpenSCAM {
  class Code;
  class Entity;

  struct EndProgram {}; // Exception

  class Controller : public VarTypes, public ModalGroup {
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

    static const int MAX_VAR = 26;

  protected:
    tplang::MachineUnitAdapter machine;
    cb::SmartPointer<ToolTable> tools;

    double params[MAX_ADDRESS];
    typedef std::map<std::string, double> named_t;
    named_t named;

    double vars[MAX_VAR];
    cb::SmartPointer<Entity> varExprs[MAX_VAR];
    bool used[MAX_VAR];

    // State variables
    const Code *activeMotion;
    CoordinateSystem coordSystems[9];
    tplang::MachineInterface::plane_t plane;
    bool latheDiameterMode;
    bool cutterRadiusComp;
    bool toolLengthComp;
    path_mode_t pathMode;
    return_mode_t returnMode;
    double motionBlendingTolerance;
    double naiveCamTolerance;
    bool modalMotion;
    bool incrementalDistanceMode;
    bool arcIncrementalDistanceMode;
    bool moveInAbsoluteCoords;

    tplang::MachineInterface::feed_mode_t feedMode;
    tplang::MachineInterface::spin_mode_t spinMode;
    dir_t spindleDir;
    double speed;
    double maxSpindleSpeed;

  public:
    Controller(tplang::MachineInterface &machine,
               const cb::SmartPointer<ToolTable> &tools = 0);
    virtual ~Controller() {}

    // State variables
    const cb::SmartPointer<ToolTable> &getToolTable() {return tools;}
    Tool &getTool(unsigned tool) {return *tools->get(tool);}
    const Tool &getTool(unsigned tool) const {return *tools->get(tool);}
    unsigned getCurrentTool() const {return (unsigned)get(TOOL_NUMBER);}

    const cb::LocationRange &getLocation() const;
    void setLocation(const cb::LocationRange &location);

    const Code *getActiveMotion() const {return activeMotion;}
    void setActiveMotion(const Code *activeMotion)
    {this->activeMotion = activeMotion;}

    CoordinateSystem &getCoordinateSystem(unsigned i);
    const CoordinateSystem &getCoordinateSystem(unsigned i) const;

    tplang::MachineInterface::plane_t getPlane() const {return plane;}
    void setPlane(tplang::MachineInterface::plane_t plane);

    bool getLatheDiameterMode() const {return latheDiameterMode;}
    bool getLatheRadiusMode() const {return !latheDiameterMode;}

    bool getCutterRadiusComp() const {return cutterRadiusComp;}
    bool getToolLengthComp() const {return toolLengthComp;}
    path_mode_t getPathMode() const {return pathMode;}
    double getMotionBlendingTolerance() const {return motionBlendingTolerance;}
    double getNaiveCamTolerance() const {return naiveCamTolerance;}

    bool getModalMotion() const {return modalMotion;}

    bool getAbsoluteDistanceMode() const {return !incrementalDistanceMode;}
    bool getIncrementalDistanceMode() const {return incrementalDistanceMode;}

    void setFeed(double feed);
    void setSpeed(double speed);
    dir_t getSpindleDir() const {return spindleDir;}
    void setSpindleDir(dir_t dir);

    void setMistCoolant(bool enable);
    bool getMistCoolant();
    void setFloodCoolant(bool enable);
    bool getFloodCoolant();

    // Parameters
    double get(unsigned addr) const
    {return addr < MAX_ADDRESS ? params[addr] : params[0];}
    void set(unsigned addr, double value)
    {if (addr < MAX_ADDRESS) params[addr] = value;}
    double get(const std::string &name) const;
    void set(const std::string &name, double value);

    // Variables
    void setVar(char c, double value)
    {vars[c - 'A'] = value; used[c - 'A'] = true;}
    double getVar(char c) const {return vars[c - 'A'];}
    void setVarExpr(char c, const cb::SmartPointer<Entity> &entity)
    {varExprs[c - 'A'] = entity;}
    const cb::SmartPointer<Entity> &getVarExpr(char c) const
    {return varExprs[c - 'A'];}
    double getOffsetVar(char c) const;

    static unsigned letterToVarType(char axis);
    static const char *getPlaneAxes(tplang::MachineInterface::plane_t plane);
    static const char *getPlaneOffsets(tplang::MachineInterface::plane_t plane);

    char getPlaneXAxis() const {return getPlaneAxes(getPlane())[0];}
    char getPlaneYAxis() const {return getPlaneAxes(getPlane())[1];}
    char getPlaneZAxis() const {return getPlaneAxes(getPlane())[2];}
    unsigned getPlaneXVarType() const {return letterToVarType(getPlaneXAxis());}
    unsigned getPlaneYVarType() const {return letterToVarType(getPlaneYAxis());}
    unsigned getPlaneZVarType() const {return letterToVarType(getPlaneZAxis());}

    void setTool(unsigned tool);

    // Position
    double getAxisAbsolutePosition(char axis) const;
    void setAxisAbsolutePosition(char axis, double pos);
    double getAxisOffset(char axis) const;
    double getAxisPosition(char axis) const;
    tplang::Axes getPosition(bool absolute = false) const;
    void setAxisPosition(char axis, double pos);
    void setPosition(const tplang::Axes &axes, bool absolute = false);
    tplang::Axes getNextPosition(int vars) const;

    std::string getVarGroupStr(const char *group, bool usedOnly = true) const;

    void reset();
    void newBlock();
    virtual void execute(const Code &code, int vars);
    void doMove(const tplang::Axes &pos, bool rapid);
    void makeMove(int vars, bool rapid);
    void moveAxis(char axis, double value, bool rapid);

    void arc(int vars, bool clockwise);
    void dwell(double seconds);
    void spindleSynchronizedMotion(bool rigidTap);
    void straightProbe(int vars, bool towardWorkpiece,
                               bool signalError);
    void drill(int vars, bool dwell, bool feedOut, bool spindleStop);
    void setToolTable(int vars, bool relative);
    void setCoordSystemOffset(unsigned cs, char axis, bool relative);
    void setCoordSystem(int vars, bool relative);
    void toolChange(bool manual = false);
    void loadToolOffsets(unsigned tool);
    void loadToolVarOffsets(int vars);
    void storePredefined1();
    void storePredefined2();
    void loadPredefined1();
    void loadPredefined2();
    void setGlobalOffsets(int vars);
    void resetGlobalOffsets(bool clearMemory);
    void restoreGlobalOffsets();
    void setCutterRadiusComp(int vars, bool left, bool dynamic);
    void end();
  };
}

#endif // OPENSCAM_CONTROLLER_H

