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

#ifndef TPLANG_MACHINE_MATRIX_H
#define TPLANG_MACHINE_MATRIX_H

#include "MachineAdapter.h"
#include "TransMatrix.h"

#include <vector>


namespace tplang {
  class MachineMatrix : virtual public MachineAdapter {
    typedef std::vector<TransMatrix> matrices_t;
    matrices_t matrices[AXES_COUNT];

  public:
    MachineMatrix();

    void pushMatrix(axes_t matrix = XYZ);
    void popMatrix(axes_t matrix = XYZ);
    void loadIdentity(axes_t matrix = XYZ);
    void scale(double x, double y, double z, axes_t matrix = XYZ);
    void translate(double x, double y, double z, axes_t matrix = XYZ);
    void rotate(double angle, double x, double y, double z,
                axes_t matrix = XYZ);
    void reflect(double x, double y, double z, axes_t matrix = XYZ);

    // From MachineInterface
    void start();
    Axes getPosition() const;
    cb::Vector3D getPosition(axes_t axes) const;
    void move(const Axes &axes, bool rapid);
    void arc(const cb::Vector3D &offset, double angle, plane_t plane);

    void setMatrix(const cb::Matrix4x4D &m, axes_t matrix);

  protected:
    matrices_t &getMatrices(axes_t matrix);
    const matrices_t &getMatrices(axes_t matrix) const;
    TransMatrix &getTransMatrix(axes_t matrix);
    const TransMatrix &getTransMatrix(axes_t matrix) const;
    void updateMatrix(axes_t matrix);
  };
}

#endif // TPLANG_MACHINE_MATRIX_H

