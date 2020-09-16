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

#include "Codes.h"

#include <cbang/String.h>
#include <cbang/Math.h>

#include <ctype.h>

using namespace std;
using namespace cb;
using namespace GCode;

typedef ModalGroup MG;
typedef VarTypes VT;


bool Code::operator<(const Code &o) const {
  return type < o.type || (type == o.type && number < o.number);
}


Code Code::parse(const string &s) {
  if (1 < s.length()) {
    char code = toupper(s[0]);
    unsigned number = round(10 * String::parseDouble(s.substr(1)));
    if (isalpha(code)) return {code, number};
  }

  THROW("Invalid code '" << s << "'");
}


ostream &GCode::operator<<(ostream &stream, const Code &code) {
  stream << code.type << code.number / 10;
  if (code.number % 10) stream << '.' << (code.number % 10);
  if (code.description) stream << " (" << code.description << ')';
  return stream;
}


// From http://www.linuxcnc.org/docs/2.4/html/gcode_main.html
const Code Codes::codes[] = {
  {'F', 0, PRI_FEED_RATE, MG::MG_ZERO, VT::VT_NONE,
   "Set Feed Rate"},
  {'S', 0, PRI_SPEED, MG::MG_ZERO, VT::VT_NONE,
   "Set Spindle Speed"},
  {'T', 0, PRI_TOOL, MG::MG_ZERO, VT::VT_NONE,
   "Select Tool"},
  {0},
};


const Code Codes::gcodes[] = {
  {'G', 0, PRI_MOTION, MG::MG_MOTION, VT::VT_AXIS,
   "Rapid Linear Motion"},
  {'G', 10, PRI_MOTION, MG::MG_MOTION, VT::VT_AXIS,
   "Linear Motion"},
  {'G', 20, PRI_MOTION, MG::MG_MOTION, VT::VT_ANGLE,
   "Clockwise Arc"},
  {'G', 30, PRI_MOTION, MG::MG_MOTION, VT::VT_ANGLE,
   "Counterclockwise Arc"},

  {'G', 40, PRI_DWELL, MG::MG_ZERO, VT::VT_P,
   "Dwell"},

  {'G', 50, PRI_MOTION, MG::MG_MOTION, VT::VT_X | VT::VT_Y | VT::VT_I |
   VT::VT_J | VT::VT_P | VT::VT_Q,
   "Cubic B-spline"},
  {'G', 51, PRI_MOTION, MG::MG_MOTION, VT::VT_X | VT::VT_Y | VT::VT_I |
   VT::VT_J,
   "Quadratic B-spline"},
  {'G', 52, PRI_MOTION, MG::MG_MOTION, VT::VT_AXIS | VT::VT_P | VT::VT_L,
   "Open NURBs Block"},
  {'G', 53, PRI_MOTION, MG::MG_MOTION, VT::VT_NONE,
   "Close NURBs Block"},

  {'G', 70, PRI_MODE, MG::MG_LATHE, VT::VT_NONE,
   "Lathe Diameter Mode"},
  {'G', 80, PRI_MODE, MG::MG_LATHE, VT::VT_NONE,
   "Lathe Radius Mode"},

  {'G', 100, PRI_OFFSETS, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_R |
   VT::VT_AXIS | VT::VT_ANGLE | VT::VT_Q,
   "System Codes"},

  {'G', 170, PRI_PLANE, MG::MG_PLANE, VT::VT_NONE,
   "XY Plane Selection"},
  {'G', 171, PRI_PLANE, MG::MG_PLANE, VT::VT_NONE,
   "UV Plane Selection"},
  {'G', 180, PRI_PLANE, MG::MG_PLANE, VT::VT_NONE,
   "ZX Plane Selection"},
  {'G', 181, PRI_PLANE, MG::MG_PLANE, VT::VT_NONE,
   "WU Plane Selection"},
  {'G', 190, PRI_PLANE, MG::MG_PLANE, VT::VT_NONE,
   "YZ Plane Selection"},
  {'G', 191, PRI_PLANE, MG::MG_PLANE, VT::VT_NONE,
   "VW Plane Selection"},

  {'G', 200, PRI_UNITS, MG::MG_UNITS, VT::VT_NONE,
   "Inches Mode"},
  {'G', 210, PRI_UNITS, MG::MG_UNITS, VT::VT_NONE,
   "Millimeters Mode"},

  // Used for homing on 3D printers
  {'G', 280, PRI_OFFSETS, MG::MG_ZERO, VT::VT_NONE,
   "Go to Predefined Position 1"},
  {'G', 281, PRI_OFFSETS, MG::MG_ZERO, VT::VT_NONE,
  "Set Predefined Position 1"},
  {'G', 282, PRI_OFFSETS, MG::MG_ZERO, VT::VT_NONE,
  "Set Axes unhomed"},
  {'G', 283, PRI_OFFSETS, MG::MG_ZERO, VT::VT_NONE,
  "Set Axes home positions"},
  {'G', 300, PRI_OFFSETS, MG::MG_ZERO, VT::VT_NONE,
   "Go to Predefined Position 2"},
  {'G', 301, PRI_OFFSETS, MG::MG_ZERO, VT::VT_NONE,
   "Set Predefined Position 2"},

  {'G', 330, PRI_MOTION, MG::MG_MOTION, VT::VT_XYZ | VT::VT_K,
   "Spindle-Synchronized Motion"},
  {'G', 331, PRI_MOTION, MG::MG_MOTION, VT::VT_XYZ | VT::VT_K,
   "Rigid Tapping"},

  {'G', 382, PRI_MOTION, MG::MG_MOTION, VT::VT_AXIS,
   "Straight Probe toward workpiece w/ error signal"},
  {'G', 383, PRI_MOTION, MG::MG_MOTION, VT::VT_AXIS,
   "Straight Probe toward workpiece wo/ error signal"},
  {'G', 384, PRI_MOTION, MG::MG_MOTION, VT::VT_AXIS,
   "Straight Probe away from workpiece w/ error signal"},
  {'G', 385, PRI_MOTION, MG::MG_MOTION, VT::VT_AXIS,
   "Straight Probe away from workpiece wo/ error signal"},

  {'G', 386, PRI_MOTION, MG::MG_MOTION, VT::VT_AXIS | VT::VT_P,
   "Seek active switch w/ error signal"},
  {'G', 387, PRI_MOTION, MG::MG_MOTION, VT::VT_AXIS | VT::VT_P,
   "Seek active switch wo/ error signal"},
  {'G', 388, PRI_MOTION, MG::MG_MOTION, VT::VT_AXIS | VT::VT_P,
   "Seek inactive switch w/ error signal"},
  {'G', 389, PRI_MOTION, MG::MG_MOTION, VT::VT_AXIS | VT::VT_P,
   "Seek inactive switch wo/ error signal"},

  {'G', 400, PRI_RADIUS_COMP, MG::MG_CUTTER_RADIUS, VT::VT_NONE,
   "Cutter Radius Compensation Off"},
  {'G', 410, PRI_RADIUS_COMP, MG::MG_CUTTER_RADIUS, VT::VT_D,
   "Left Cutter Radius Compensation"},
  {'G', 411, PRI_RADIUS_COMP, MG::MG_CUTTER_RADIUS, VT::VT_D | VT::VT_L,
   "Left Dynamic Cutter Radius Compensation"},
  {'G', 420, PRI_RADIUS_COMP, MG::MG_CUTTER_RADIUS, VT::VT_D,
   "Right Cutter Radius Compensation"},
  {'G', 421, PRI_RADIUS_COMP, MG::MG_CUTTER_RADIUS, VT::VT_D | VT::VT_L,
   "Right Dynamic Cutter Radius Compensation"},

  {'G', 430, PRI_LENGTH_COMP, MG::MG_TOOL_OFFSET, VT::VT_H,
   "Activate Tool Length Offset"},
  {'G', 431, PRI_LENGTH_COMP, MG::MG_TOOL_OFFSET, VT::VT_AXIS,
   "Dynamic Tool Length Offset"},
  {'G', 432, PRI_LENGTH_COMP, MG::MG_TOOL_OFFSET, VT::VT_H,
   "Additional Tool Length Offset"},
  {'G', 490, PRI_LENGTH_COMP, MG::MG_TOOL_OFFSET, VT::VT_NONE,
   "Cancel Tool Length Offset"},

  {'G', 520, PRI_OFFSETS, MG::MG_ZERO, VT::VT_AXIS,
   "Set Coordinate System Offsets"},
  {'G', 530, PRI_OFFSETS, MG::MG_ZERO, VT::VT_NONE,
   "Move in Absolute Coordinates"},

  {'G', 540, PRI_COORD_SYS, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 1"},
  {'G', 550, PRI_COORD_SYS, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 2"},
  {'G', 560, PRI_COORD_SYS, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 3"},
  {'G', 570, PRI_COORD_SYS, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 4"},
  {'G', 580, PRI_COORD_SYS, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 5"},
  {'G', 590, PRI_COORD_SYS, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 6"},
  {'G', 591, PRI_COORD_SYS, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 7"},
  {'G', 592, PRI_COORD_SYS, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 8"},
  {'G', 593, PRI_COORD_SYS, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 9"},

  {'G', 610, PRI_PATH, MG::MG_ZERO, VT::VT_NONE,
   "Set Exact Path Control Mode"},
  {'G', 611, PRI_PATH, MG::MG_ZERO, VT::VT_NONE,
   "Set Exact Stop Control Mode"},
  {'G', 640, PRI_PATH, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "Set Best Possible Speed Control Mode"},

  {'G', 700, PRI_UNITS, MG::MG_UNITS, VT::VT_NONE,
   "Mach3/4 Inches Mode, use G20 instead"},
  {'G', 710, PRI_UNITS, MG::MG_UNITS, VT::VT_NONE,
   "Mach3/4 Millimeters Mode, use G21 instead"},

  {'G', 730, PRI_MOTION, MG::MG_MOTION, VT::VT_XYZ | VT::VT_ABC | VT::VT_RLQ,
   "Drilling Cycle with Chip Breaking"},
  {'G', 760, PRI_MOTION, MG::MG_MOTION, VT::VT_P | VT::VT_Z | VT::VT_IJK |
   VT::VT_RLQ | VT::VT_H | VT::VT_E,
   "Threading Cycle"},
  {'G', 800, PRI_MOTION, MG::MG_MOTION, VT::VT_NONE,
   "Cancel Canned Cycle"},
  {'G', 810, PRI_MOTION, MG::MG_MOTION, VT::VT_CANNED,
   "Drilling Cycle"},
  {'G', 820, PRI_MOTION, MG::MG_MOTION, VT::VT_CANNED | VT::VT_P,
   "Drilling Cycle w/ Dwell"},
  {'G', 830, PRI_MOTION, MG::MG_MOTION, VT::VT_CANNED,
   "Peck Drilling"},
  {'G', 840, PRI_MOTION, MG::MG_MOTION, VT::VT_CANNED,
   "Right-Hand Tapping"},
  {'G', 850, PRI_MOTION, MG::MG_MOTION, VT::VT_CANNED,
   "Boring, No Dwell, Feed Out"},
  {'G', 860, PRI_MOTION, MG::MG_MOTION, VT::VT_CANNED | VT::VT_P,
   "Boring, Spindle Stop, Rapid Out"},
  {'G', 870, PRI_MOTION, MG::MG_MOTION, VT::VT_CANNED,
   "Back Boring"},
  {'G', 880, PRI_MOTION, MG::MG_MOTION, VT::VT_CANNED,
   "Boring, Spindle Stop, Manual Out"},
  {'G', 890, PRI_MOTION, MG::MG_MOTION, VT::VT_CANNED | VT::VT_P,
   "Boring, Dwell, Feed Out"},

  {'G', 900, PRI_DIST, MG::MG_DISTANCE, VT::VT_NONE,
   "XYZ Absolute Distance Mode"},
  {'G', 901, PRI_DIST, MG::MG_ARC_DISTANCE, VT::VT_NONE,
   "IJK Absolute Distance Mode"},
  {'G', 910, PRI_DIST, MG::MG_DISTANCE, VT::VT_NONE,
   "XYZ Incremental Distance Mode"},
  {'G', 911, PRI_DIST, MG::MG_ARC_DISTANCE, VT::VT_NONE,
   "IJK Incremental Distance Mode"},

  {'G', 920, PRI_OFFSETS, MG::MG_ZERO, VT::VT_AXIS,
   "Set Coordinate System Offsets"},
  {'G', 921, PRI_OFFSETS, MG::MG_ZERO, VT::VT_AXIS,
   "Reset Coordinate System Offsets"},
  {'G', 922, PRI_OFFSETS, MG::MG_ZERO, VT::VT_AXIS,
   "Disable Coordinate System Offsets"},
  {'G', 923, PRI_OFFSETS, MG::MG_ZERO, VT::VT_AXIS,
   "Enable Coordinate System Offsets"},

  {'G', 930, PRI_OFFSETS, MG::MG_FEED_RATE, VT::VT_NONE,
   "Set Feed Rate Inverse Time Mode"},
  {'G', 940, PRI_OFFSETS, MG::MG_FEED_RATE, VT::VT_NONE,
   "Set Feed Rate Units per Minute Mode"},
  {'G', 950, PRI_OFFSETS, MG::MG_FEED_RATE, VT::VT_NONE,
   "Set Feed Rate Units per Revolution Mode"},

  {'G', 960, PRI_MODE, MG::MG_ZERO, VT::VT_D | VT::VT_S,
   "Spindle Constant Surface Speed Mode"},
  {'G', 970, PRI_MODE, MG::MG_ZERO, VT::VT_NONE,
   "Spindle Control RPM Mode"},

  {'G', 980, PRI_RETRACT, MG::MG_RETURN_MODE, VT::VT_NONE,
   "Set Canned Cycle Return R"},
  {'G', 990, PRI_RETRACT, MG::MG_RETURN_MODE, VT::VT_NONE,
   "Set Canned Cycle Return Last"},
  {0},
};


const Code Codes::g10codes[] = {
  {'G', 10, PRI_OFFSETS, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_R |
   VT::VT_AXIS | VT::VT_I | VT::VT_J | VT::VT_Q,
   "L1 Set Tool Table"},
  {'G', 20, PRI_OFFSETS, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_R |
   VT::VT_AXIS,
   "L2 Set Coordinate System"},
  {'G', 100, PRI_OFFSETS, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_R |
   VT::VT_X | VT::VT_Z | VT::VT_Q,
   "L10 Set Tool Table To Current Offsets"},
  {'G', 110, PRI_OFFSETS, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_R |
   VT::VT_X | VT::VT_Z | VT::VT_Q,
   "L11 Set Tool Table To Current Coordinates"},
  {'G', 200, PRI_OFFSETS, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_AXIS,
   "L20 Set Coordinate System To Current Offsets"},
  {0},
};


const Code Codes::mcodes[] = {
  {'M', 00, PRI_STOP, MG::MG_STOPPING, VT::VT_NONE,
   "Pause"},
  {'M', 10, PRI_STOP, MG::MG_STOPPING, VT::VT_NONE,
   "Pause If Stopped"},
  {'M', 20, PRI_STOP, MG::MG_STOPPING, VT::VT_NONE,
   "End Program"},

  {'M', 30, PRI_SPINDLE, MG::MG_SPINDLE, VT::VT_NONE,
   "Start Spindle Clockwise"},
  {'M', 40, PRI_SPINDLE, MG::MG_SPINDLE, VT::VT_NONE,
   "Start Spindle Counterclockwise"},
  {'M', 50, PRI_SPINDLE, MG::MG_SPINDLE, VT::VT_NONE,
   "Stop Spindle"},

  {'M', 60, PRI_CHANGE_TOOL, MG::MG_TOOL_CHANGE, VT::VT_NONE,
   "Manual Tool Change"},

  {'M', 70, PRI_COOLANT, MG::MG_COOLANT, VT::VT_NONE,
   "Turn Mist Coolant On"},
  {'M', 71, PRI_COOLANT, MG::MG_COOLANT, VT::VT_NONE,
   "Turn Mist Coolant Off"},
  {'M', 80, PRI_COOLANT, MG::MG_COOLANT, VT::VT_NONE,
   "Turn Flood Coolant On"},
  {'M', 81, PRI_COOLANT, MG::MG_COOLANT, VT::VT_NONE,
   "Turn Flood Coolant Off"},
  {'M', 90, PRI_COOLANT, MG::MG_COOLANT, VT::VT_NONE,
   "Turn All Coolant Off"},

  {'M', 300, PRI_STOP, MG::MG_STOPPING, VT::VT_NONE,
   "Change Pallet Shuttles and End"},

  {'M', 480, PRI_OVERRIDES, MG::MG_OVERRIDE, VT::VT_NONE,
   "Enable Spindle Speed & Feed Override"},
  {'M', 490, PRI_OVERRIDES, MG::MG_OVERRIDE, VT::VT_NONE,
   "Disable Spindle Speed & Feed Override"},

  {'M', 500, PRI_OVERRIDES, MG::MG_OVERRIDE, VT::VT_P,
   "Feed Override Control"},
  {'M', 510, PRI_OVERRIDES, MG::MG_OVERRIDE, VT::VT_P,
   "Spindle Speed Override Control"},
  {'M', 520, PRI_OVERRIDES, MG::MG_OVERRIDE, VT::VT_P,
   "Adaptive Feed Control"},
  {'M', 530, PRI_OVERRIDES, MG::MG_OVERRIDE, VT::VT_P,
   "Feed Stop Control"},

  {'M', 600, PRI_STOP, MG::MG_STOPPING, VT::VT_NONE,
   "Change Pallet Shuttles and Pause"},

  {'M', 610, PRI_CHANGE_TOOL, MG::MG_TOOL_CHANGE, VT::VT_Q,
   "Set Current Tool Number"},

  {'M', 620, PRI_IO, MG::MG_ZERO, VT::VT_P,
   "Turn On Digital Output Synchronized w/ Motion"},
  {'M', 630, PRI_IO, MG::MG_ZERO, VT::VT_P,
   "Turn Off Digital Output Synchronized w/ Motion"},
  {'M', 640, PRI_IO, MG::MG_ZERO, VT::VT_P,
   "Turn On Digital Output Immediately"},
  {'M', 650, PRI_IO, MG::MG_ZERO, VT::VT_P,
   "Turn Off Digital Output Immediately"},

  {'M', 660, PRI_IO, MG::MG_ZERO, VT::VT_P | VT::VT_E | VT::VT_L | VT::VT_Q,
   "Input Control"},

  {'M', 670, PRI_IO, MG::MG_ZERO, VT::VT_E | VT::VT_Q,
   "Analog Output Synchronized w/ Motion"},
  {'M', 680, PRI_IO, MG::MG_ZERO, VT::VT_E | VT::VT_Q,
   "Immediate Analog Output"},

  {'M', 700, PRI_STATE, MG::MG_ZERO, VT::VT_NONE,
   "Save Modal State"},
  {'M', 710, PRI_STATE, MG::MG_ZERO, VT::VT_NONE,
   "Invalidate Stored Modal State"},
  {'M', 720, PRI_STATE, MG::MG_ZERO, VT::VT_NONE,
   "Restore Modal State"},
  {'M', 730, PRI_STATE, MG::MG_ZERO, VT::VT_NONE,
   "Save and Autorestore Modal State"},

  {'M', 1000, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1010, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1020, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1030, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1040, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1050, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1060, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1070, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1080, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1090, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1100, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1110, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1120, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1130, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1140, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1150, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1160, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1170, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1180, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1190, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1200, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1210, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1220, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1230, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1240, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1250, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1260, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1270, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1280, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1290, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1300, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1310, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1320, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1330, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1340, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1350, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1360, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1370, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1380, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1390, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1400, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1410, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1420, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1430, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1440, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1450, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1460, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1470, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1480, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1490, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1500, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1510, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1520, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1530, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1540, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1550, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1560, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1570, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1580, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1590, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1600, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1610, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1620, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1630, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1640, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1650, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1660, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1670, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1680, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1690, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1700, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1710, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1720, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1730, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1740, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1750, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1760, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1770, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1780, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1790, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1800, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1810, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1820, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1830, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1840, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1850, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1860, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1870, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1880, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1890, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1900, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1910, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1920, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1930, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1940, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1950, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1960, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1970, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1980, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {'M', 1990, PRI_USER, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "User Defined Command"},
  {0},
};


namespace {
  const Code *findCode(const Code *table, unsigned number) {
    // A linear search will do pig
    for (int i = 0; table[i].type; i++)
      if (table[i].number == number) return &table[i];

    return 0; // Not found
  }
}


const Code *Codes::find(char _type, double _number, double _L) {
  char type = toupper(_type);
  unsigned number = round(_number * 10);

  switch (type) {
  case 'G':
    if (number == 100 && _L) return findCode(g10codes, round(_L * 10));
    return findCode(gcodes, number);

  case 'M': return findCode(mcodes, number);

  default:
    for (int i = 0; codes[i].type; i++)
      if (codes[i].type == type) return &codes[i];
    return 0;
  }

  return 0;
}
