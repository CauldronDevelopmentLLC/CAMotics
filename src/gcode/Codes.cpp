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

#include "Codes.h"

#include <cbang/Math.h>

#include <ctype.h>

using namespace std;
using namespace GCode;

typedef ModalGroup MG;
typedef VarTypes VT;


ostream &GCode::operator<<(ostream &stream, const Code &code) {
  stream << code.type << code.number / 10;
  if (code.number % 10) stream << '.' << (code.number % 10);
  return stream << " (" << code.description << ')';
}


// From http://www.linuxcnc.org/docs/2.4/html/gcode_main.html
const Code Codes::codes[] = {
  {'F', 0, 3, MG::MG_ZERO, VT::VT_NONE,
   "Set Feed Rate"},
  {'S', 0, 4, MG::MG_ZERO, VT::VT_NONE,
   "Set Spindle Speed"},
  {'T', 0, 5, MG::MG_ZERO, VT::VT_NONE,
   "Select Tool"},
  {0},
};


const Code Codes::gcodes[] = {
  {'G', 0, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Rapid Linear Motion"},
  {'G', 10, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Linear Motion"},
  {'G', 20, 20, MG::MG_MOTION, VT::VT_ANGLE,
   "Clockwise Arc"},
  {'G', 30, 20, MG::MG_MOTION, VT::VT_ANGLE,
   "Counterclockwise Arc"},

  {'G', 40, 10, MG::MG_ZERO, VT::VT_P,
   "Dwell"},

  {'G', 51, 20, MG::MG_MOTION, VT::VT_X | VT::VT_Y | VT::VT_I | VT::VT_J,
   "Quadratic B-spline"},
  {'G', 52, 20, MG::MG_MOTION, VT::VT_AXIS | VT::VT_P | VT::VT_L,
   "Open NURBs Block"},
  {'G', 53, 20, MG::MG_MOTION, VT::VT_NONE,
   "Close NURBs Block"},

  {'G', 70, 2, MG::MG_LATHE, VT::VT_NONE,
   "Lathe Diameter Mode"},
  {'G', 80, 2, MG::MG_LATHE, VT::VT_NONE,
   "Lathe Radius Mode"},

  {'G', 100, 19, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_R | VT::VT_AXIS |
   VT::VT_ANGLE | VT::VT_Q,
   "System Codes"},

  {'G', 170, 11, MG::MG_PLANE, VT::VT_NONE,
   "XY Plane Selection"},
  {'G', 171, 11, MG::MG_PLANE, VT::VT_NONE,
   "UV Plane Selection"},
  {'G', 180, 11, MG::MG_PLANE, VT::VT_NONE,
   "ZX Plane Selection"},
  {'G', 181, 11, MG::MG_PLANE, VT::VT_NONE,
   "WU Plane Selection"},
  {'G', 190, 11, MG::MG_PLANE, VT::VT_NONE,
   "YZ Plane Selection"},
  {'G', 191, 11, MG::MG_PLANE, VT::VT_NONE,
   "VW Plane Selection"},

  {'G', 200, 12, MG::MG_UNITS, VT::VT_NONE,
   "Use Inches"},
  {'G', 210, 12, MG::MG_UNITS, VT::VT_NONE,
   "Use Millimeters"},

  {'G', 280, 19, MG::MG_ZERO, VT::VT_NONE, // Used for homing on 3D printers
   "Go to Predefined Position 1"},
  {'G', 281, 19, MG::MG_ZERO, VT::VT_NONE,
  "Set Predefined Position 1"},
  {'G', 282, 19, MG::MG_ZERO, VT::VT_NONE,
  "Set Axes unhomed"},
  {'G', 283, 19, MG::MG_ZERO, VT::VT_NONE,
  "Set Axes home positions"},
  {'G', 300, 19, MG::MG_ZERO, VT::VT_NONE,
   "Go to Predefined Position 2"},
  {'G', 301, 19, MG::MG_ZERO, VT::VT_NONE,
   "Set Predefined Position 2"},

  {'G', 330, 20, MG::MG_MOTION, VT::VT_XYZ | VT::VT_K,
   "Spindle-Synchronized Motion"},
  {'G', 331, 20, MG::MG_MOTION, VT::VT_XYZ | VT::VT_K,
   "Rigid Tapping"},

  {'G', 382, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Straight Probe toward workpiece w/ error signal"},
  {'G', 383, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Straight Probe toward workpiece wo/ error signal"},
  {'G', 384, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Straight Probe away from workpiece w/ error signal"},
  {'G', 385, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Straight Probe away from workpiece wo/ error signal"},

  {'G', 386, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Seek active switch w/ error signal"},
  {'G', 387, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Seek active switch wo/ error signal"},
  {'G', 388, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Seek inactive switch w/ error signal"},
  {'G', 389, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Seek inactive switch wo/ error signal"},

  {'G', 400, 13, MG::MG_CUTTER_RADIUS, VT::VT_NONE,
   "Cutter Radius Compensation Off"},
  {'G', 410, 13, MG::MG_CUTTER_RADIUS, VT::VT_D,
   "Left Cutter Radius Compensation"},
  {'G', 411, 13, MG::MG_CUTTER_RADIUS, VT::VT_D | VT::VT_L,
   "Left Dynamic Cutter Radius Compensation"},
  {'G', 420, 13, MG::MG_CUTTER_RADIUS, VT::VT_D,
   "Right Cutter Radius Compensation"},
  {'G', 421, 13, MG::MG_CUTTER_RADIUS, VT::VT_D | VT::VT_L,
   "Right Dynamic Cutter Radius Compensation"},

  {'G', 430, 14, MG::MG_TOOL_OFFSET, VT::VT_H,
   "Activate Tool Length Compensation"},
  {'G', 431, 14, MG::MG_TOOL_OFFSET, VT::VT_AXIS,
   "Activate Dynamic Tool Length Compensation"},
  {'G', 490, 14, MG::MG_TOOL_OFFSET, VT::VT_NONE,
   "Cancel Tool Length Compensation"},

  {'G', 520, 19, MG::MG_ZERO, VT::VT_AXIS,
   "Set Coordinate System Offsets"},
  {'G', 530, 19, MG::MG_ZERO, VT::VT_NONE,
   "Move in Absolute Coordinates"},

  {'G', 540, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 1"},
  {'G', 550, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 2"},
  {'G', 560, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 3"},
  {'G', 570, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 4"},
  {'G', 580, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 5"},
  {'G', 590, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 6"},
  {'G', 591, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 7"},
  {'G', 592, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 8"},
  {'G', 593, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 9"},

  {'G', 610, 16, MG::MG_ZERO, VT::VT_NONE,
   "Set Exact Path Control Mode"},
  {'G', 611, 16, MG::MG_ZERO, VT::VT_NONE,
   "Set Exact Stop Control Mode"},
  {'G', 640, 16, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "Set Best Possible Speed Control Mode"},

  {'G', 730, 20, MG::MG_MOTION, VT::VT_XYZ | VT::VT_ABC | VT::VT_RLQ,
   "Drilling Cycle with Chip Breaking"},
  {'G', 760, 20, MG::MG_MOTION, VT::VT_P | VT::VT_Z | VT::VT_IJK |  VT::VT_RLQ |
   VT::VT_H | VT::VT_E, "Threading Cycle"},
  {'G', 800, 20, MG::MG_MOTION, VT::VT_NONE,
   "Cancel Modal Motion"},
  {'G', 810, 20, MG::MG_MOTION, VT::VT_CANNED,
   "Drilling Cycle"},
  {'G', 820, 20, MG::MG_MOTION, VT::VT_CANNED | VT::VT_P,
   "Drilling Cycle w/ Dwell"},
  {'G', 830, 20, MG::MG_MOTION, VT::VT_CANNED,
   "Peck Drilling"},
  {'G', 840, 20, MG::MG_MOTION, VT::VT_CANNED,
   "Right-Hand Tapping"},
  {'G', 850, 20, MG::MG_MOTION, VT::VT_CANNED,
   "Boring, No Dwell, Feed Out"},
  {'G', 860, 20, MG::MG_MOTION, VT::VT_CANNED | VT::VT_P,
   "Boring, Spindle Stop, Rapid Out"},
  {'G', 870, 20, MG::MG_MOTION, VT::VT_CANNED,
   "Back Boring"},
  {'G', 880, 20, MG::MG_MOTION, VT::VT_CANNED,
   "Boring, Spindle Stop, Manual Out"},
  {'G', 890, 20, MG::MG_MOTION, VT::VT_CANNED | VT::VT_P,
   "Boring, Dwell, Feed Out"},

  {'G', 900, 17, MG::MG_DISTANCE, VT::VT_NONE,
   "XYZ Absolute Distance Mode"},
  {'G', 901, 17, MG::MG_ARC_DISTANCE, VT::VT_NONE,
   "IJK Absolute Distance Mode"},
  {'G', 910, 17, MG::MG_DISTANCE, VT::VT_NONE,
   "XYZ Incremental Distance Mode"},
  {'G', 911, 17, MG::MG_ARC_DISTANCE, VT::VT_NONE,
   "IJK Incremental Distance Mode"},

  {'G', 920, 19, MG::MG_ZERO, VT::VT_AXIS,
   "Set Coordinate System Offsets"},
  {'G', 921, 19, MG::MG_ZERO, VT::VT_AXIS,
   "Reset Coordinate System Offsets"},
  {'G', 922, 19, MG::MG_ZERO, VT::VT_AXIS,
   "Disable Coordinate System Offsets"},
  {'G', 923, 19, MG::MG_ZERO, VT::VT_AXIS,
   "Enable Coordinate System Offsets"},

  {'G', 930, 19, MG::MG_FEED_RATE, VT::VT_NONE,
   "Set Feed Rate Inverse Time Mode"},
  {'G', 940, 19, MG::MG_FEED_RATE, VT::VT_NONE,
   "Set Feed Rate Units per Minute Mode"},
  {'G', 950, 19, MG::MG_FEED_RATE, VT::VT_NONE,
   "Set Feed Rate Units per Revolution Mode"},

  {'G', 960, 2, MG::MG_ZERO, VT::VT_D | VT::VT_S,
   "Spindle Constant Surface Speed Mode"},
  {'G', 970, 2, MG::MG_ZERO, VT::VT_NONE,
   "Spindle Control RPM Mode"},

  {'G', 980, 18, MG::MG_RETURN_MODE, VT::VT_NONE,
   "Set Canned Cycle Return R"},
  {'G', 990, 18, MG::MG_RETURN_MODE, VT::VT_NONE,
   "Set Canned Cycle Return Last"},
  {0},
};


const Code Codes::g10codes[] = {
  {'G', 10, 19, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_R | VT::VT_AXIS |
   VT::VT_I | VT::VT_J | VT::VT_Q,
   "Set Tool Table"},
  {'G', 20, 19, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_R | VT::VT_AXIS,
   "Set Coordinate System"},
  {'G', 100, 19, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_R | VT::VT_X |
   VT::VT_Z | VT::VT_Q,
   "Set Tool Table To Current Offsets"},
  {'G', 200, 19, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_AXIS,
   "Set Coordinate System To Current Offsets"},
  {0},
};


const Code Codes::mcodes[] = {
  {'M', 00, 21, MG::MG_STOPPING, VT::VT_NONE,
   "Pause"},
  {'M', 10, 21, MG::MG_STOPPING, VT::VT_NONE,
   "Pause If Stopped"},
  {'M', 20, 12, MG::MG_STOPPING, VT::VT_NONE,
   "End Program"},

  {'M', 30, 7, MG::MG_SPINDLE, VT::VT_NONE,
   "Start Spindle Clockwise"},
  {'M', 40, 7, MG::MG_SPINDLE, VT::VT_NONE,
   "Start Spindle Counterclockwise"},
  {'M', 50, 7, MG::MG_SPINDLE, VT::VT_NONE,
   "Stop Spindle"},

  {'M', 60, 6, MG::MG_TOOL_CHANGE, VT::VT_NONE,
   "Manual Tool Change"},

  {'M', 70, 8, MG::MG_COOLANT, VT::VT_NONE,
   "Turn Mist Coolant On"},
  {'M', 80, 8, MG::MG_COOLANT, VT::VT_NONE,
   "Turn Flood Coolant On"},
  {'M', 90, 8, MG::MG_COOLANT, VT::VT_NONE,
   "Turn All Coolant Off"},

  {'M', 300, 21, MG::MG_STOPPING, VT::VT_NONE,
   "Change Pallet Shuttles and End"},

  {'M', 480, 9, MG::MG_OVERRIDE, VT::VT_NONE,
   "Enable Spindle Speed & Feed Override"},
  {'M', 490, 9, MG::MG_OVERRIDE, VT::VT_NONE,
   "Disable Spindle Speed & Feed Override"},

  {'M', 500, 9, MG::MG_OVERRIDE, VT::VT_P,
   "Feed Override Control"},
  {'M', 510, 9, MG::MG_OVERRIDE, VT::VT_P,
   "Spindle Speed Override Control"},
  {'M', 520, 9, MG::MG_OVERRIDE, VT::VT_P,
   "Adaptive Feed Control"},
  {'M', 530, 9, MG::MG_OVERRIDE, VT::VT_P,
   "Feed Stop Control"},

  {'M', 600, 21, MG::MG_STOPPING, VT::VT_NONE,
   "Change Pallet Shuttles and Pause"},

  {'M', 610, 5, MG::MG_ZERO, VT::VT_Q,
   "Set Current Tool Number"},

  {'M', 620, 20, MG::MG_ZERO, VT::VT_P,
   "Turn On Digital Output Synchronized w/ Motion"},
  {'M', 630, 20, MG::MG_ZERO, VT::VT_P,
   "Turn Off Digital Output Synchronized w/ Motion"},
  {'M', 640, 20, MG::MG_ZERO, VT::VT_P,
   "Turn On Digital Output Immediately"},
  {'M', 650, 20, MG::MG_ZERO, VT::VT_P,
   "Turn Off Digital Output Immediately"},

  {'M', 660, 20, MG::MG_ZERO, VT::VT_P | VT::VT_E | VT::VT_L | VT::VT_Q,
   "Input Control"},

  {'M', 670, 20, MG::MG_ZERO, VT::VT_E | VT::VT_Q,
   "Analog Output Synchronized w/ Motion"},
  {'M', 680, 20, MG::MG_ZERO, VT::VT_E | VT::VT_Q,
   "Immediate Analog Output"},
  {0},
};


const Code *Codes::find(char _type, double _number, double _L) {
  char type = toupper(_type);
  unsigned number = round(_number * 10);
  unsigned L = round(_L * 10);

  const Code *table = 0;

  switch (type) {
  case 'G':
    if (number == 100 && L) {
      number = L;
      table = g10codes;

    } else table = gcodes;
    break;

  case 'M': table = mcodes; break;

  default:
    for (int i = 0; codes[i].type; i++)
      if (codes[i].type == type) return &codes[i];
    return 0;
  }

  // A linear search will do pig
  for (int i = 0; table[i].type; i++)
    if (table[i].number == number) return &table[i];

  return 0;
}
