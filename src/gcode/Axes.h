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

#include <cbang/Exception.h>
#include <cbang/Math.h>
#include <cbang/geom/Vector.h>
#include <cbang/geom/Matrix.h>
#include <cbang/json/Serializable.h>

#include <cctype>


namespace GCode {
  class Axes : public cb::Vector<9, double>, public cb::JSON::Serializable {
  public:
    typedef cb::Vector<9, double> Super_T;

    static const char *AXES;

    Axes(double v = 0) : Super_T(v) {}
    Axes(const double data[9]) : Super_T(data) {}
    Axes(const cb::Vector<9, double> &v) : cb::Vector<9, double>(v) {}

    void setFrom(const Axes &o);
    double get(char c) const {return getIndex(toIndex(c));}
    void set(char c, double value) {setIndex(toIndex(c), value);}
    double getIndex(unsigned i) const {return data[i];}
    void setIndex(unsigned i, double value) {
      if (!cb::Math::isfinite(value))
        THROW(toAxis(i) << " position is invalid");
      data[i] = value;
    }

    double getX() const {return data[0];}
    double getY() const {return data[1];}
    double getZ() const {return data[2];}
    double getA() const {return data[3];}
    double getB() const {return data[4];}
    double getC() const {return data[5];}
    double getU() const {return data[6];}
    double getV() const {return data[7];}
    double getW() const {return data[8];}

    void setX(double value) {data[0] = value;}
    void setY(double value) {data[1] = value;}
    void setZ(double value) {data[2] = value;}
    void setA(double value) {data[3] = value;}
    void setB(double value) {data[4] = value;}
    void setC(double value) {data[5] = value;}
    void setU(double value) {data[6] = value;}
    void setV(double value) {data[7] = value;}
    void setW(double value) {data[8] = value;}

    cb::Vector3D getXYZ() const {return cb::Vector3D(getX(), getY(), getZ());}
    cb::Vector3D getABC() const {return cb::Vector3D(getA(), getB(), getC());}
    cb::Vector3D getUVW() const {return cb::Vector3D(getU(), getV(), getW());}

    cb::Vector2D getXY() const {return cb::Vector2D(getX(), getY());}
    cb::Vector2D getXZ() const {return cb::Vector2D(getX(), getZ());}
    cb::Vector2D getYZ() const {return cb::Vector2D(getY(), getZ());}

    cb::Vector2D getAB() const {return cb::Vector2D(getA(), getB());}
    cb::Vector2D getAC() const {return cb::Vector2D(getA(), getC());}
    cb::Vector2D getBC() const {return cb::Vector2D(getB(), getC());}

    cb::Vector2D getUV() const {return cb::Vector2D(getU(), getV());}
    cb::Vector2D getUW() const {return cb::Vector2D(getU(), getW());}
    cb::Vector2D getVW() const {return cb::Vector2D(getV(), getW());}

    void setXYZ(const cb::Vector3D &v) {setX(v[0]); setY(v[1]); setZ(v[2]);}
    void setXYZ(double x, double y, double z) {setX(x); setY(y); setZ(z);}
    void setABC(const cb::Vector3D &v) {setA(v[0]); setB(v[1]); setC(v[2]);}
    void setABC(double a, double b, double c) {setA(a); setB(b); setC(c);}
    void setUVW(const cb::Vector3D &v) {setU(v[0]); setV(v[1]); setW(v[2]);}
    void setUVW(double u, double v, double w) {setU(u); setV(v); setW(w);}

    void applyXYZMatrix(const cb::Matrix4x4D &m);
    void applyABCMatrix(const cb::Matrix4x4D &m);
    void applyUVWMatrix(const cb::Matrix4x4D &m);

    inline static bool isAxis(char axis) {
      switch (std::toupper(axis)) {
      case 'X': case 'Y': case 'Z':
      case 'A': case 'B': case 'C':
      case 'U': case 'V': case 'W':
        return true;
      }

      return false;
    }

    inline static unsigned toIndex(char axis) {
      switch (std::toupper(axis)) {
      case 'X': return 0;
      case 'Y': return 1;
      case 'Z': return 2;
      case 'A': return 3;
      case 'B': return 4;
      case 'C': return 5;
      case 'U': return 6;
      case 'V': return 7;
      case 'W': return 8;
      default: THROW("Invalid axis " << axis);
      }
    }

    inline static char toAxis(unsigned i, bool lower = false) {
      switch (i) {
      case 0: return lower ? 'x' : 'X';
      case 1: return lower ? 'y' : 'Y';
      case 2: return lower ? 'z' : 'Z';
      case 3: return lower ? 'a' : 'A';
      case 4: return lower ? 'b' : 'B';
      case 5: return lower ? 'c' : 'C';
      case 6: return lower ? 'u' : 'U';
      case 7: return lower ? 'v' : 'V';
      case 8: return lower ? 'w' : 'W';
      default: THROW("Invalid axis index " << i);
      }
    }

    inline static std::string toAxisName(unsigned i, bool lower = false) {
      return std::string(1, toAxis(i, lower));
    }

    // From cb::JSON::Serializable
    void read(const cb::JSON::Value &value);
    void write(cb::JSON::Sink &sink) const;
  };
}
