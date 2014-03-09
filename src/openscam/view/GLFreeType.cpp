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

/*
 * A quick and simple opengl font library that uses GNU freetype2, written
 * and distributed as part of a tutorial for nehe.gamedev.net.
 * Sven Olsen, 2003
 */
#include "GLFreeType.h"
#include "GL.h"

#include <cbang/Exception.h>
#include <cbang/String.h>

#include <cbang/util/Resource.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_TRIGONOMETRY_H

#include <vector>
#include <string>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


namespace OpenSCAM {
  extern const DirectoryResource resource0;
}


// This function gets the first power of 2 >= the int that we pass it.
static int next_pow_2(int a) {
  int rval = 2;
  while (rval < a) rval <<= 1;
  return rval;
}


GLFreeType::GLFreeType(const string &fname, unsigned h, float lineHeight) :
  h(h), lineHeight(lineHeight), listBase(0) {
  // Create and initilize a freetype font library.
  FT_Library library;
  if (FT_Init_FreeType(&library)) THROW("FT_Init_FreeType failed");

  // The object which holds information on a given font is called a "face"
  FT_Face face;

  const Resource *data = OpenSCAM::resource0.find(fname);
  if (!data) THROWS("Failed to find font: " << fname);

  // This is where we load in the font information from the file.
  // Of all the places where the code might die, this is the most likely,
  // as FT_New_Face will die if the font file does not exist or is somehow
  // broken.
  int err;
  if ((err = FT_New_Memory_Face(library, (uint8_t *)data->getData(),
                                data->getLength(), 0, &face)))
    THROWS("FT_New_Memory_Face() failed to read: " << fname << ": " << err);

  // For some twisted reason, Freetype measures font size in terms of
  // 1 / 64ths of pixels.  Thus, to make a font h pixels high, we need
  // to request a size of h * 64.0 (h << 6 is just a prettier way of
  // writting h * 64)
  FT_Set_Char_Size(face, h << 6, h << 6, 96, 96);

  // Here we ask opengl to allocate resources for all the textures and displays
  // lists which we are about to create.
  listBase = glGenLists(128);
  glGenTextures(128, textures);

  // This is where we actually create each of the fonts display lists.
  for (int i = 0; i < 128; i++) displayList(face, i);

  // We don't need the face information now that the display
  // lists have been created, so we free the assosiated resources.
  FT_Done_Face(face);

  FT_Done_FreeType(library);
}


GLFreeType::~GLFreeType() {
  glDeleteLists(listBase, 128);
  glDeleteTextures(128, textures);
}


void GLFreeType::dimensions(const string &s, float &width,
                            float &height) const {
  height = 0;
  width = 0;

  if (s.empty()) return;

  float w = 0;
  height = lineHeight * h;
  for (unsigned i = 0; i < s.length(); i++) {
    unsigned char c = (unsigned char)s[i];

    if (c == '\n') {
      w = 0;
      height += lineHeight * h;
    } else w += widths[c];

    if (width < w) width = w;
  }
}


float GLFreeType::width(const string &s) const {
  float width;
  float height;
  dimensions(s, width, height);
  return width;
}


float GLFreeType::height(const string &s) const {
  float width;
  float height;
  dimensions(s, width, height);
  return height;
}


// Much like Nehe's glPrint function, but modified to work with freetype fonts.
void GLFreeType::print(float x, float y, const string &s,
                       unsigned center) const {
  glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
  glMatrixMode(GL_MODELVIEW);
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glListBase(listBase);

  // Split lines
  vector<string> lines;
  String::tokenize(s, lines, "\n", true);

  for (unsigned i = 0; i < lines.size(); i++) {
    float offset = 0;
    if (center) offset = (center - width(lines[i])) / 2;

    // When each character is draw it modifies the current matrix so that the
    // next character will be drawn immediatly after it.
    glPushMatrix();

    glTranslatef(x + offset, y - lineHeight * h * i, 0);
    glCallLists((GLsizei)lines[i].length(), GL_UNSIGNED_BYTE, lines[i].c_str());
    
    glPopMatrix();
  }

  glPopAttrib();

  glFlush(); // Windows needs this or the fonts come out wrong
}


// Create a display list coresponding to the give character.
void GLFreeType::displayList(FT_Face face, unsigned char ch) {
  // The first thing we do is get FreeType to render our character
  // into a bitmap.This actually requires a couple of FreeType commands:

  // Load the Glyph for our character.
  if (FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), FT_LOAD_DEFAULT))
    THROW("FT_Load_Glyph failed");

  // Move the face's glyph into a Glyph object.
  FT_Glyph glyph;
  if (FT_Get_Glyph(face->glyph, &glyph)) THROW("FT_Get_Glyph failed");

  // Store the character's width
  widths[ch] = face->glyph->advance.x >> 6;

  // Convert the glyph to a bitmap.
  FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
  FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

  // This reference will make accessing the bitmap easier
  FT_Bitmap &bitmap = bitmap_glyph->bitmap;

  // Use our helper function to get the widths of the bitmap data that
  // we will need in order to create our texture.
  int width = next_pow_2(bitmap.width);
  int height = next_pow_2(bitmap.rows);

  // Allocate memory for the texture data.
  GLubyte *expanded_data = new GLubyte[2 * width * height];

  // Here we fill in the data for the expanded bitmap.  Notice that we
  // are using two channel bitmap(one for luminocity and one for
  // alpha), but we assign both luminocity and alpha to the value that
  // we find in the FreeType bitmap.
  // We use the ?: operator so that value which we use will be 0 if we
  // are in the padding zone, and whatever is the the Freetype bitmap
  // otherwise.
  for (int j = 0; j < height; j++)
    for (int i = 0; i < width; i++) {
      expanded_data[2 * (i + j * width)] = 255;
      expanded_data[2 * (i + j * width) + 1] =
        (i >= bitmap.width || j >= bitmap.rows) ?
        0 : bitmap.buffer[i + bitmap.width * j];
    }

  // Now we just setup some texture paramaters.
  glBindTexture(GL_TEXTURE_2D, textures[ch]);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

  // Here we actually create the texture itself, notice that we are
  // using GL_LUMINANCE_ALPHA to indicate that we are using 2 channel data.
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
               0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data);

  // With the texture created, we don't need to expanded data anymore
  delete[] expanded_data;

  // So now we can create the display list
  glNewList(listBase + ch, GL_COMPILE);

  glBindTexture(GL_TEXTURE_2D, textures[ch]);

  glPushMatrix();

  // first we need to move over a little so that the character has the
  // right amount of space between it and the one before it.
  glTranslatef((float)bitmap_glyph->left, 0.0, 0.0);

  // Now we move down a little in the case that the bitmap extends
  // past the bottom of the line (this is only true for characters
  // like 'g' or 'y'.
  glTranslatef(0.0, (float)bitmap_glyph->top - bitmap.rows, 0.0);

  // Now we need to account for the fact that many of our textures are
  // filled with empty padding space.  We figure what portion of the
  // texture is used by the actual character and store that
  // information in the x and y variables, then when we draw the quad,
  // we will only reference the parts of the texture that we contain
  // the character itself.
  float x = (float)bitmap.width / (float)width;
  float y = (float)bitmap.rows / (float)height;

  // Here we draw the texturemaped quads.  The bitmap that we got from
  // FreeType was not oriented quite like we would like it to be, so
  // we need to link the texture to the quad so that the result will
  // be properly aligned.
  glBegin(GL_QUADS);
  glTexCoord2d(0,0); glVertex2f(0.0, (float)bitmap.rows);
  glTexCoord2d(0,y); glVertex2f(0.0, 0.0);
  glTexCoord2d(x,y); glVertex2f((float)bitmap.width, 0.0);
  glTexCoord2d(x,0); glVertex2f((float)bitmap.width, (float)bitmap.rows);
  glEnd();
  glPopMatrix();
  glTranslatef((float)(face->glyph->advance.x >> 6), 0.0, 0.0);

  // Finnish the display list
  glEndList();

  FT_Done_Glyph(glyph);
}
