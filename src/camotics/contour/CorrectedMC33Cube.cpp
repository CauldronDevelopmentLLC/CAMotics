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

#include "CorrectedMC33Cube.h"
#include "CorrectedMC33LUTs.h"

#include <cbang/Exception.h>

#include <math.h>

using namespace CAMotics;


bool CorrectedMC33Cube::testFace(int8_t face) const {
  double A, B, C, D;

  switch (face < 0 ? -face : face) {
  case 1: A = cube[0]; B = cube[4]; C = cube[5]; D = cube[1]; break;
  case 2: A = cube[1]; B = cube[5]; C = cube[6]; D = cube[2]; break;
  case 3: A = cube[2]; B = cube[6]; C = cube[7]; D = cube[3]; break;
  case 4: A = cube[3]; B = cube[7]; C = cube[4]; D = cube[0]; break;
  case 5: A = cube[0]; B = cube[3]; C = cube[2]; D = cube[1]; break;
  case 6: A = cube[4]; B = cube[7]; C = cube[6]; D = cube[5]; break;
  default: THROW("Invalid face " << face);
  };

  return 0 <= face * A * (A * C - B * D); // face and A invert signs
}


bool CorrectedMC33Cube::testInterior(uint8_t ccase, uint8_t config,
                                     int8_t s) const {
  switch (ccase) {
  case 4:
    return interiorAmbiguity(1, s) + interiorAmbiguity(2, s) +
      interiorAmbiguity(5, s);

  case 6: return interiorAmbiguity(abs(test6[config][0]), s);

  case 7:
    return interiorAmbiguity(1, -s) + interiorAmbiguity(2, -s) +
      interiorAmbiguity(5, -s);

  case 10: return interiorAmbiguity(abs(test10[config][0]), s);

  case 12:
    return interiorAmbiguity(abs(test12[config][0]), s) +
      interiorAmbiguity(abs(test12[config][1]), s);
  }

  THROW("Invalid interior test case " << ccase);
}


int CorrectedMC33Cube::interiorAmbiguityEdge(int face, int s) const {
  switch (face) {
  case 1: case 3:
    if (0 < cube[1] * s && 0 < cube[7] * s) return 4;
    if (0 < cube[0] * s && 0 < cube[6] * s) return 5;
    if (0 < cube[3] * s && 0 < cube[5] * s) return 6;
    if (0 < cube[2] * s && 0 < cube[4] * s) return 7;
    break;

  case 2: case 4:
    if (0 < cube[1] * s && 0 < cube[7] * s) return 0;
    if (0 < cube[2] * s && 0 < cube[4] * s) return 1;
    if (0 < cube[3] * s && 0 < cube[5] * s) return 2;
    if (0 < cube[0] * s && 0 < cube[6] * s) return 3;
    break;

  case 0: case 5: case 6:
    if (0 < cube[0] * s && 0 < cube[6] * s) return 8;
    if (0 < cube[1] * s && 0 < cube[7] * s) return 9;
    if (0 < cube[2] * s && 0 < cube[4] * s) return 10;
    if (0 < cube[3] * s && 0 < cube[5] * s) return 11;
    break;
  }

  THROW("Invalid interior ambiguity test on face " << face);
}


int CorrectedMC33Cube::interiorAmbiguity(int face, int s) const {
  static const uint8_t lut[12][8] = {
    {0, 1, 2, 3, 4, 5, 6, 7},
    {3, 2, 6, 7, 0, 1, 5, 4},
    {2, 3, 0, 1, 6, 7, 4, 5},
    {1, 0, 4, 5, 2, 3, 7, 6},
    {2, 1, 5, 6, 3, 0, 4, 7},
    {3, 0, 4, 7, 2, 1, 5, 6},
    {0, 3, 2, 1, 4, 7, 6, 5},
    {1, 2, 6, 5, 0, 3, 7, 4},
    {4, 0, 1, 5, 7, 3, 2, 6},
    {5, 1, 2, 6, 4, 0, 3, 7},
    {6, 2, 3, 7, 5, 1, 0, 4},
    {7, 3, 0, 4, 6, 2, 1, 5},
  };

  int edge = interiorAmbiguityEdge(face, s);
  if (edge < 0) return 0;
  const uint8_t *e = lut[edge];

  double a = (cube[e[0]] - cube[e[1]]) * (cube[e[7]] - cube[e[6]]) -
             (cube[e[4]] - cube[e[5]]) * (cube[e[3]] - cube[e[2]]);

  if (0 < a) return 1;

  double b = cube[e[6]] * (cube[e[0]] - cube[e[1]]) +
             cube[e[1]] * (cube[e[7]] - cube[e[6]]) -
             cube[e[2]] * (cube[e[4]] - cube[e[5]]) -
             cube[e[5]] * (cube[e[3]] - cube[e[2]]);

  double t = -b / (2 * a);
  if (t < 0 || 1 < t) return 1;

  double At = cube[e[1]] + (cube[e[0]] - cube[e[1]]) * t;
  double Bt = cube[e[5]] + (cube[e[4]] - cube[e[5]]) * t;
  double Ct = cube[e[6]] + (cube[e[7]] - cube[e[6]]) * t;
  double Dt = cube[e[2]] + (cube[e[3]] - cube[e[2]]) * t;

  return At * Ct - Bt * Dt < 0;
}


// Return tunnel orientation if the interior is empty (two faces)
int CorrectedMC33Cube::testCase13() const {
  double a = (cube[0] - cube[1]) * (cube[7] - cube[6]) -
             (cube[4] - cube[5]) * (cube[3] - cube[2]);
  double b = cube[6] * (cube[0] - cube[1]) + cube[1] * (cube[7] - cube[6]) -
             cube[2] * (cube[4] - cube[5]) - cube[5] * (cube[3] - cube[2]);
  double c = cube[1] * cube[6] - cube[5] * cube[2];

  double delta = b * b - 4 * a * c;
  double t1 = (-b + sqrt(delta)) / (2 * a);
  double t2 = (-b - sqrt(delta)) / (2 * a);

  if (t1 < 1 && 0 < t1 && t2 < 1 && 0 < t2) {
    double At1 = cube[1] + (cube[0] - cube[1]) * t1;
    double Bt1 = cube[5] + (cube[4] - cube[5]) * t1;
    double Ct1 = cube[6] + (cube[7] - cube[6]) * t1;
    double Dt1 = cube[2] + (cube[3] - cube[2]) * t1;

    double x1 = (At1 - Dt1) / (At1 + Ct1 - Bt1 - Dt1);
    double y1 = (At1 - Bt1) / (At1 + Ct1 - Bt1 - Dt1);

    double At2 = cube[1] + (cube[0] - cube[1]) * t2;
    double Bt2 = cube[5] + (cube[4] - cube[5]) * t2;
    double Ct2 = cube[6] + (cube[7] - cube[6]) * t2;
    double Dt2 = cube[2] + (cube[3] - cube[2]) * t2;

    double x2 = (At2 - Dt2) / (At2 + Ct2 - Bt2 - Dt2);
    double y2 = (At2 - Bt2) / (At2 + Ct2 - Bt2 - Dt2);

    // if it's a tunnel
    if (x1 < 1 && 0 < x1 && x2 < 1 && 0 < x2 && y1 < 1 &&
        0 < y1 && y2 < 1 && 0 < y2) {
      double tm = (t1 + t2) / 2.0;
      double Atm = cube[1] + (cube[0] - cube[1]) * tm;
      double Btm = cube[5] + (cube[4] - cube[5]) * tm;
      double Ctm = cube[6] + (cube[7] - cube[6]) * tm;
      double Dtm = cube[2] + (cube[3] - cube[2]) * tm;

      double s = (Atm * Ctm - Btm * Dtm) / (Atm + Ctm - Btm - Dtm);
      return 0 < s ? 1 : -1;
    }
  }

  return 0;
}


const int8_t *CorrectedMC33Cube::process(unsigned &count) const {
  uint8_t ccase = cases[lutEntry][0];
  uint8_t config = cases[lutEntry][1];

  switch (ccase) {
  case 0: return 0;
  case 1: count = 1; return tiling1[config]; break;
  case 2: count = 2; return tiling2[config]; break;

  case 3:
    if (testFace(test3[config])) {count = 4; return tiling3_2[config];}
    else {count = 2; return tiling3_1[config];}
    break;

  case 4:
    if (testInterior(ccase, config, test4[config])) {
      count = 2;
      return tiling4_1[config];

    } else {count = 6; return tiling4_2[config];}
    break;

  case 5: count = 3; return tiling5[config]; break;

  case 6:
    if (testFace(test6[config][0])) {count = 5; return tiling6_2[config];}
    else {
      if (testInterior(ccase, config, test6[config][1])) {
        count = 3;
        return tiling6_1_1[config];

      } else {count = 9; return tiling6_1_2[config];}
    }
    break;

  case 7: {
    unsigned subconfig = 0;
    for (unsigned i = 0; i < 3; i++)
      if (testFace(test7[config][i])) subconfig += 1 << i;

    switch (subconfig) {
    case 0: count = 3; return tiling7_1[config];    break;
    case 1: count = 5; return tiling7_2[config][0]; break;
    case 2: count = 5; return tiling7_2[config][1]; break;
    case 3: count = 9; return tiling7_3[config][0]; break;
    case 4: count = 5; return tiling7_2[config][2]; break;
    case 5: count = 9; return tiling7_3[config][1]; break;
    case 6: count = 9; return tiling7_3[config][2]; break;

    case 7:
      if (testInterior(ccase, config, test7[config][3])) {
        count = 5;
        return tiling7_4_1[config];

      } else {count = 9; return tiling7_4_2[config];}
      break;
    }
    break;
  }

  case 8: count = 2; return tiling8[config]; break;
  case 9: count = 4; return tiling9[config]; break;

  case 10:
    if (testFace(test10[config][0])) {
      if (testFace(test10[config][1])) {
        if (testInterior(ccase, config, -test10[config][2])) {
          count = 4;
          return tiling10_1_1_[config];

        } else {count = 8; return tiling10_1_2[5 - config];}

      } else {count = 8; return tiling10_2[config];}

    } else {
      if (testFace(test10[config][1])) {
        count = 8;
        return tiling10_2_[config];

      } else {
        if (testInterior(ccase, config, test10[config][2])) {
          count = 4;
          return tiling10_1_1[config];

        } else {count = 8; return tiling10_1_2[config];}
      }
    }
    break;

  case 11: count = 4; return tiling11[config]; break;

  case 12:
    if (testFace(test12[config][0])) {
      if (testFace(test12[config][1])) {
        if (testInterior(ccase, config, -test12[config][2])) {
          count = 4;
          return tiling12_1_1_[config];

        } else {count = 8; return tiling12_1_2[23 - config];}

      } else {count = 8; return tiling12_2[config];}

    } else {
      if (testFace(test12[config][1])) {
        count = 8;
        return tiling12_2_[config];

      } else {
        if (testInterior(ccase, config, test12[config][2])) {
          count = 4; return tiling12_1_1[config];
        } else {count = 8; return tiling12_1_2[config];}
      }
    }
    break;

  case 13: {
    unsigned subconfig = 0;
    for (unsigned i = 0; i < 6; i++)
      if (testFace(test13[config][i])) subconfig += 1 << i;

    int8_t sub = subconfig13[subconfig];
    switch (sub) {
    case 0: count = 4; return tiling13_1[config]; break;

    case 1: case 2: case 3: case 4: case 5: case 6:
      count = 6;
      return tiling13_2[config][sub - 1];
      break;

    case 7: case 8: case 9: case 10: case 11: case 12: case 13: case 14:
    case 15: case 16: case 17: case 18:
      count = 10;
      return tiling13_3[config][sub - 7];
      break;

    case 19: case 20: case 21: case 22:
      count = 12;
      return tiling13_4[config][sub - 19];
      break;

    case 23: {
      int tunnel = testCase13();

      if (config == 0) {
        if (tunnel == 1)       {count = 10; return tiling13_5_2[0][0];}
        else if (tunnel == -1) {count = 10; return tiling13_5_2[1][2];}
        else                   {count = 6;  return tiling13_5_1[0][0];}

      } else {
        if (tunnel == 1)       {count = 10; return tiling13_5_2[1][0];}
        else if (tunnel == -1) {count = 10; return tiling13_5_2[0][2];}
        else                   {count = 6;  return tiling13_5_1[1][0];}
      }
      break;
    }

    case 24: {
      int tunnel = testCase13();

      if (config == 0) {
        if (tunnel == 1)       {count = 10; return tiling13_5_2[0][1];}
        else if (tunnel == -1) {count = 10; return tiling13_5_2[1][0];}
        else                   {count = 6;  return tiling13_5_1[0][1];}

      } else {
        if (tunnel == 1)       {count = 10; return tiling13_5_2[1][1];}
        else if (tunnel == -1) {count = 10; return tiling13_5_2[0][3];}
        else                   {count = 6;  return tiling13_5_1[1][1];}
      }
      break;
    }

    case 25: {
      int tunnel = testCase13();

      if (config == 0) {
        if (tunnel == 1)       {count = 10; return tiling13_5_2[0][2];}
        else if (tunnel == -1) {count = 10; return tiling13_5_2[1][3];}
        else                   {count = 6;  return tiling13_5_1[0][2];}

      } else {
        if (tunnel == 1)       {count = 10; return tiling13_5_2[1][2];}
        else if (tunnel == -1) {count = 10; return tiling13_5_2[0][0];}
        else                   {count = 6;  return tiling13_5_1[1][2];}
      }
      break;
    }

    case 26: {
      int tunnel = testCase13();

      if (config == 0) {
        if (tunnel == 1)       {count = 10; return tiling13_5_2[0][3];}
        else if (tunnel == -1) {count = 10; return tiling13_5_2[1][1];}
        else                   {count = 6;  return tiling13_5_1[0][3];}

      } else {
        if (tunnel == 1)       {count = 10; return tiling13_5_2[1][3];}
        else if (tunnel == -1) {count = 10; return tiling13_5_2[0][2];}
        else                   {count = 6;  return tiling13_5_1[1][3];}
      }

      // 13.4 common node is negative
      // {count = 12; return tiling13_4[config][3];}
      break;
    }

    case 27: case 28: case 29: case 30: case 31: case 32: case 33: case 34:
    case 35: case 36: case 37: case 38:
      count = 10;
      return tiling13_3_[config][sub - 27];
      break;

    case 39: case 40: case 41: case 42: case 43: case 44:
      count = 6;
      return tiling13_2_[config][sub - 39];
      break;

    case 45: count = 4; return tiling13_1_[config]; break;

    default: THROW("Marching Cubes: Impossible case 13 sub config " << sub);
    }
    break;
  }

  case 14: count = 4; return tiling14[config]; break;
  }

  THROW("Invalid case " << (int)ccase);
}
