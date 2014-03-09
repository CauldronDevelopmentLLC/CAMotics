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

#ifndef OPENSCAM_GL_FREE_TYPE_H
#define OPENSCAM_GL_FREE_TYPE_H

#include <string>

struct FT_FaceRec_;
typedef struct FT_FaceRec_ *FT_Face;

// Wrap everything in a namespace, that we can use common
// function names like "print" without worrying about
// overlapping with anyone else's code.
namespace OpenSCAM {
  // This holds all of the information related to any
  // freetype font that we want to create.
  class GLFreeType {
    float h; //< Holds the height of the font
    float lineHeight;
    unsigned textures[128]; //< Holds the texture id's
    unsigned widths[128]; //< Holds the character widths
    unsigned listBase; //< Holds the first display list id

  public:
    // The init function will create a font of
    // of the height h from the file fname.
    GLFreeType(const std::string &fname, unsigned h, float lineHeight = 1.75);

    // Free all the resources assosiated with the font
    ~GLFreeType();

    float getLineHeight() const {return lineHeight * h;}

    void dimensions(const std::string &s, float &width, float &height) const;
    float width(const std::string &s) const;
    float height(const std::string &s) const;

    // The flagship function of the library - this thing will print
    // out text at window coordinates x,y, using the font ft_font.
    // The current modelview matrix will also be applied to the text.
    void print(float x, float y, const std::string &s,
               unsigned center = 0) const;

  protected:
    void displayList(FT_Face face, unsigned char ch);
  };
}

#endif // OPENSCAM_GL_FREE_TYPE_H
