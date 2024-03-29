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

#include "GLComposite.h"
#include "GLProgram.h"

#include <cbang/Catch.h>

using namespace CAMotics;


void GLComposite::glDraw(GLContext &gl) {
  for (unsigned i = 0; i < objects.size(); i++) {
    GLObject &o = *objects[i];
    if (!o.isVisible()) continue;
    // If color picking, only draw pickable objects
    if (gl.getPicking() && !o.isPickable()) continue;

    bool identity = o.getTransform().isIdentity();

    if (!identity) gl.pushMatrix(o.getTransform());

    GLProgram &program = gl.getProgram();

    program.set("doPicking", gl.getPicking());
    program.set("light.enabled", o.getLight());
    if (o.getColor() != Color()) gl.setColor(o.getColor());

    TRY_CATCH_ERROR(o.glDraw(gl));

    if (!identity) gl.popMatrix();
  }
}
