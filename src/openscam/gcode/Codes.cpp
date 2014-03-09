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

#include "Codes.h"

using namespace std;
using namespace OpenSCAM;

typedef ModalGroup MG;
typedef VarTypes VT;


ostream &OpenSCAM::operator<<(ostream &stream, const Code &code) {
  return stream << code.type << code.number << " (" << code.description << ')';
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
  {'G', 1, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Linear Motion"},
  {'G', 2, 20, MG::MG_MOTION, VT::VT_ANGLE,
   "Clockwise Arc"},
  {'G', 3, 20, MG::MG_MOTION, VT::VT_ANGLE,
   "Counterclockwise Arc"},

  {'G', 4, 10, MG::MG_ZERO, VT::VT_P,
   "Dwell"},

  {'G', 5.1, 20, MG::MG_MOTION, VT::VT_X | VT::VT_Y | VT::VT_I | VT::VT_J,
   "Quadratic B-spline"},
  {'G', 5.2, 20, MG::MG_MOTION, VT::VT_AXIS | VT::VT_P | VT::VT_L,
   "Open NURBs Block"},
  {'G', 5.3, 20, MG::MG_MOTION, VT::VT_NONE,
   "Close NURBs Block"},

  {'G', 7, 2, MG::MG_LATHE, VT::VT_NONE,
   "Lathe Diameter Mode"},
  {'G', 8, 2, MG::MG_LATHE, VT::VT_NONE,
   "Lathe Radius Mode"},

  {'G', 10, 19, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_R | VT::VT_AXIS |
   VT::VT_ANGLE | VT::VT_Q,
   "System Codes"},

  {'G', 17, 11, MG::MG_PLANE, VT::VT_NONE,
   "XY Plane Selection"},
  {'G', 17.1, 11, MG::MG_PLANE, VT::VT_NONE,
   "UV Plane Selection"},
  {'G', 18, 11, MG::MG_PLANE, VT::VT_NONE,
   "ZX Plane Selection"},
  {'G', 18.1, 11, MG::MG_PLANE, VT::VT_NONE,
   "WU Plane Selection"},
  {'G', 19, 11, MG::MG_PLANE, VT::VT_NONE,
   "YZ Plane Selection"},
  {'G', 19.1, 11, MG::MG_PLANE, VT::VT_NONE,
   "VW Plane Selection"},

  {'G', 20, 12, MG::MG_UNITS, VT::VT_NONE,
   "Use Inches"},
  {'G', 21, 12, MG::MG_UNITS, VT::VT_NONE,
   "Use Millimeters"},

  {'G', 28, 19, MG::MG_ZERO, VT::VT_NONE,
   "Go to Predefined Position 1"},
  {'G', 28.1, 19, MG::MG_ZERO, VT::VT_NONE, 
  "Set Predefined Position 1"},
  {'G', 30, 19, MG::MG_ZERO, VT::VT_NONE,
   "Go to Predefined Position 2"},
  {'G', 30.1, 19, MG::MG_ZERO, VT::VT_NONE,
   "Set Predefined Position 2"},

  {'G', 33, 20, MG::MG_MOTION, VT::VT_XYZ | VT::VT_K,
   "Spindle-Synchronized Motion"},
  {'G', 33.1, 20, MG::MG_MOTION, VT::VT_XYZ | VT::VT_K,
   "Rigid Tapping"},

  {'G', 38.2, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Straight Probe toward workpiece w/ error signal"},
  {'G', 38.3, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Straight Probe toward workpiece wo/ error signal"},
  {'G', 38.4, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Straight Probe away from workpiece w/ error signal"},
  {'G', 38.5, 20, MG::MG_MOTION, VT::VT_AXIS,
   "Straight Probe away from workpiece wo/ error signal"},

  {'G', 40, 13, MG::MG_CUTTER_RADIUS, VT::VT_NONE,
   "Cutter Radius Compensation Off"},
  {'G', 41, 13, MG::MG_CUTTER_RADIUS, VT::VT_D,
   "Left Cutter Radius Compensation"},
  {'G', 41.1, 13, MG::MG_CUTTER_RADIUS, VT::VT_D | VT::VT_L,
   "Left Dynamic Cutter Radius Compensation"},
  {'G', 42, 13, MG::MG_CUTTER_RADIUS, VT::VT_D,
   "Right Cutter Radius Compensation"},
  {'G', 42.1, 13, MG::MG_CUTTER_RADIUS, VT::VT_D | VT::VT_L,
   "Right Dynamic Cutter Radius Compensation"},

  {'G', 43, 14, MG::MG_TOOL_OFFSET, VT::VT_H,
   "Activate Tool Length Compensation"},
  {'G', 43.1, 14, MG::MG_TOOL_OFFSET, VT::VT_AXIS,
   "Activate Dynamic Tool Length Compensation"},
  {'G', 49, 14, MG::MG_TOOL_OFFSET, VT::VT_NONE,
   "Cancel Tool Length Compensation"},

  {'G', 53, 19, MG::MG_ZERO, VT::VT_NONE,
   "Move in Absolute Coordinates"},

  {'G', 54, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 1"},
  {'G', 55, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 2"},
  {'G', 56, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 3"},
  {'G', 57, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 4"},
  {'G', 58, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 5"},
  {'G', 59, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 6"},
  {'G', 59.1, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 7"},
  {'G', 59.2, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 8"},
  {'G', 59.3, 15, MG::MG_COORD_SYSTEM, VT::VT_NONE,
   "Select Coordinate System 9"},

  {'G', 61, 16, MG::MG_ZERO, VT::VT_NONE,
   "Set Exact Path Control Mode"},
  {'G', 61.1, 16, MG::MG_ZERO, VT::VT_NONE,
   "Set Exact Stop Control Mode"},
  {'G', 64, 16, MG::MG_ZERO, VT::VT_P | VT::VT_Q,
   "Set Best Possible Speed Control Mode"},

  {'G', 73, 20, MG::MG_MOTION, VT::VT_XYZ | VT::VT_ABC | VT::VT_RLQ,
   "Drilling Cycle with Chip Breaking"},
  {'G', 76, 20, MG::MG_MOTION, VT::VT_P | VT::VT_Z | VT::VT_IJK |  VT::VT_RLQ |
   VT::VT_H | VT::VT_E, "Threading Cycle"},
  {'G', 80, 20, MG::MG_MOTION, VT::VT_NONE,
   "Cancel Modal Motion"},
  {'G', 81, 20, MG::MG_MOTION, VT::VT_CANNED,
   "Drilling Cycle"},
  {'G', 82, 20, MG::MG_MOTION, VT::VT_CANNED | VT::VT_P,
   "Drilling Cycle w/ Dwell"},
  {'G', 83, 20, MG::MG_MOTION, VT::VT_CANNED,
   "Peck Drilling"},
  {'G', 84, 20, MG::MG_MOTION, VT::VT_CANNED,
   "Right-Hand Tapping"},
  {'G', 85, 20, MG::MG_MOTION, VT::VT_CANNED,
   "Boring, No Dwell, Feed Out"},
  {'G', 86, 20, MG::MG_MOTION, VT::VT_CANNED | VT::VT_P,
   "Boring, Spindle Stop, Rapid Out"},
  {'G', 87, 20, MG::MG_MOTION, VT::VT_CANNED,
   "Back Boring"},
  {'G', 88, 20, MG::MG_MOTION, VT::VT_CANNED,
   "Boring, Spindle Stop, Manual Out"},
  {'G', 89, 20, MG::MG_MOTION, VT::VT_CANNED | VT::VT_P,
   "Boring, Dwell, Feed Out"},

  {'G', 90, 17, MG::MG_DISTANCE, VT::VT_NONE,
   "XYZ Absolute Distance Mode"},
  {'G', 90.1, 17, MG::MG_DISTANCE, VT::VT_NONE,
   "IJK Absolute Distance Mode"},
  {'G', 91, 17, MG::MG_DISTANCE, VT::VT_NONE,
   "XYZ Incremental Distance Mode"},
  {'G', 91.1, 17, MG::MG_DISTANCE, VT::VT_NONE,
   "IJK Incremental Distance Mode"},

  {'G', 92, 19, MG::MG_ZERO, VT::VT_AXIS,
   "Set Coordinate System Offsets"},
  {'G', 92.1, 19, MG::MG_ZERO, VT::VT_AXIS,
   "Reset Coordinate System Offsets"},
  {'G', 92.2, 19, MG::MG_ZERO, VT::VT_AXIS,
   "Disable Coordinate System Offsets"},
  {'G', 92.3, 19, MG::MG_ZERO, VT::VT_AXIS,
   "Enable Coordinate System Offsets"},

  {'G', 93, 19, MG::MG_FEED_RATE, VT::VT_NONE,
   "Set Feed Rate Inverse Time Mode"},
  {'G', 94, 19, MG::MG_FEED_RATE, VT::VT_NONE,
   "Set Feed Rate Units per Minute Mode"},
  {'G', 95, 19, MG::MG_FEED_RATE, VT::VT_NONE,
   "Set Feed Rate Units per Revolution Mode"},

  {'G', 96, 2, MG::MG_ZERO, VT::VT_D | VT::VT_S,
   "Spindle Constant Surface Speed Mode"},
  {'G', 97, 2, MG::MG_ZERO, VT::VT_NONE,
   "Spindle Control RPM Mode"},

  {'G', 98, 18, MG::MG_RETURN_MODE, VT::VT_NONE,
   "Set Canned Cycle Return R"},
  {'G', 99, 18, MG::MG_RETURN_MODE, VT::VT_NONE,
   "Set Canned Cycle Return Last"},
  {0},
};


const Code Codes::g10codes[] = {
  {'G', 1, 19, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_R | VT::VT_AXIS |
   VT::VT_I | VT::VT_J | VT::VT_Q,
   "Set Tool Table"},
  {'G', 2, 19, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_R | VT::VT_AXIS,
   "Set Coordinate System"},
  {'G', 10, 19, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_R | VT::VT_X |
   VT::VT_Z | VT::VT_Q,
   "Set Tool Table To Current Offsets"},
  {'G', 20, 19, MG::MG_ZERO, VT::VT_L | VT::VT_P | VT::VT_AXIS,
   "Set Coordinate System To Current Offsets"},
  {0},
};


const Code Codes::mcodes[] = {
  {'M', 0, 21, MG::MG_STOPPING, VT::VT_NONE,
   "Pause"},
  {'M', 1, 21, MG::MG_STOPPING, VT::VT_NONE,
   "Pause If Stopped"},
  {'M', 2, 12, MG::MG_STOPPING, VT::VT_NONE,
   "End Program"},

  {'M', 3, 7, MG::MG_SPINDLE, VT::VT_NONE,
   "Start Spindle Clockwise"},
  {'M', 4, 7, MG::MG_SPINDLE, VT::VT_NONE,
   "Start Spindle Counterclockwise"},
  {'M', 5, 7, MG::MG_SPINDLE, VT::VT_NONE,
   "Stop Spindle"},

  {'M', 6, 6, MG::MG_TOOL_CHANGE, VT::VT_NONE,
   "Manual Tool Change"},

  {'M', 7, 8, MG::MG_COOLANT, VT::VT_NONE,
   "Turn Mist Coolant On"},
  {'M', 8, 8, MG::MG_COOLANT, VT::VT_NONE,
   "Turn Flood Coolant On"},
  {'M', 9, 8, MG::MG_COOLANT, VT::VT_NONE,
   "Turn All Coolant Off"},

  {'M', 30, 21, MG::MG_STOPPING, VT::VT_NONE,
   "Change Pallet Shuttles and End"},

  {'M', 48, 9, MG::MG_OVERRIDE, VT::VT_NONE,
   "Enable Spindle Speed & Feed Override"},
  {'M', 49, 9, MG::MG_OVERRIDE, VT::VT_NONE,
   "Disable Spindle Speed & Feed Override"},

  {'M', 50, 9, MG::MG_OVERRIDE, VT::VT_P,
   "Feed Override Control"},
  {'M', 51, 9, MG::MG_OVERRIDE, VT::VT_P,
   "Spindle Speed Override Control"},
  {'M', 52, 9, MG::MG_OVERRIDE, VT::VT_P,
   "Adaptive Feed Control"},
  {'M', 53, 9, MG::MG_OVERRIDE, VT::VT_P,
   "Feed Stop Control"},

  {'M', 60, 21, MG::MG_STOPPING, VT::VT_NONE,
   "Change Pallet Shuttles and Pause"},

  {'M', 61, 5, MG::MG_ZERO, VT::VT_Q,
   "Set Current Tool Number"},

  {'M', 62, 20, MG::MG_ZERO, VT::VT_P,
   "Turn On Digital Output Synchronized w/ Motion"},
  {'M', 63, 20, MG::MG_ZERO, VT::VT_P,
   "Turn Off Digital Output Synchronized w/ Motion"},
  {'M', 64, 20, MG::MG_ZERO, VT::VT_P,
   "Turn On Digital Output Immediately"},
  {'M', 65, 20, MG::MG_ZERO, VT::VT_P,
   "Turn Off Digital Output Immediately"},

  {'M', 66, 20, MG::MG_ZERO, VT::VT_P | VT::VT_E | VT::VT_L | VT::VT_Q,
   "Input Control"},

  {'M', 67, 20, MG::MG_ZERO, VT::VT_E | VT::VT_Q,
   "Analog Output Synchronized w/ Motion"},
  {'M', 68, 20, MG::MG_ZERO, VT::VT_E | VT::VT_Q,
   "Immediate Analog Output"},
  {0},
};


const Code *Codes::find(char type, float number, float L) {
  const Code *table = 0;

  switch (type) {
  case 'G':
    if (number == 10 && L) {
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
