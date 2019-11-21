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

#include "GradientBackground.h"

#include <vector>

using namespace CAMotics;
using namespace std;


void GradientBackground::glDraw(GLContext &gl) {
  vector<float> colors;
  colors.insert(colors.end(), top.data,    top.data    + 3);
  colors.insert(colors.end(), top.data,    top.data    + 3);
  colors.insert(colors.end(), bottom.data, bottom.data + 3);
  colors.insert(colors.end(), top.data,    top.data    + 3);
  colors.insert(colors.end(), bottom.data, bottom.data + 3);
  colors.insert(colors.end(), bottom.data, bottom.data + 3);

  const float v[] = {
    -1, -1, 0, 1, -1, 0, -1, 1, 0,
     1, -1, 0, 1,  1, 0, -1, 1, 0,
  };

  gl.glEnableVertexAttribArray(GL_ATTR_POSITION);
  gl.glVertexAttribPointer(GL_ATTR_POSITION, 3, GL_FLOAT, false, 0, v);

  gl.glEnableVertexAttribArray(GL_ATTR_COLOR);
  gl.glVertexAttribPointer(GL_ATTR_COLOR, 3, GL_FLOAT, false, 0, &colors[0]);

  gl.glDisable(GL_DEPTH_TEST);
  gl.glDrawArrays(GL_TRIANGLES, 0, 6);
  gl.glEnable(GL_DEPTH_TEST);

  gl.glDisableVertexAttribArray(GL_ATTR_POSITION);
  gl.glDisableVertexAttribArray(GL_ATTR_COLOR);
}
