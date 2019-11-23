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

#include <string>
#include <map>

#include <cbang/geom/Matrix.h>


namespace CAMotics {
  class GLShader;

  class GLProgram {
    unsigned program = 0;
    std::map<std::string, unsigned> uniforms;

  public:
    GLProgram();
    ~GLProgram();

    bool inUse() const;

    unsigned get() const {return program;}

    void attach(const GLShader &shader);
    void attach(const std::string &resource, int type);
    void bindAttribute(const std::string &name, unsigned id);

    void link();
    void use() const;
    void unuse() const;

    unsigned getUniform(const std::string &name);

    void set(const std::string &name, double v0);
    void set(const std::string &name, double v0, double v1);
    void set(const std::string &name, double v0, double v1, double v2);
    void set
    (const std::string &name, double v0, double v1, double v2, double v3);

    void set(const std::string &name, const cb::Vector3F &v);
    void set(const std::string &name, const cb::Vector4F &v);
    void set(const std::string &name, const cb::Vector3D &v);
    void set(const std::string &name, const cb::Vector4D &v);

    void set(const std::string &name, int v0);
    void set(const std::string &name, int v0, int v1);
    void set(const std::string &name, int v0, int v1, int v2);
    void set(const std::string &name, int v0, int v1, int v2, int v3);

    void set(const std::string &name, const cb::Matrix4x4F &m);
    void set(const std::string &name, const cb::Matrix4x4D &m);
    void set(const std::string &name, const cb::Matrix3x3F &m);
    void set(const std::string &name, const cb::Matrix3x3D &m);
  };
}
