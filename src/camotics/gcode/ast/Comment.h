/******************************************************************************\

    CAMotics is an Open-Source CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef CAMOTICS_COMMENT_H
#define CAMOTICS_COMMENT_H

#include "Entity.h"

namespace CAMotics {
  class Comment : public Entity {
    std::string text;
    bool paren;

  public:
    Comment(const std::string &text, bool paren) : text(text), paren(paren) {}

    const std::string &getText() const {return text;}
    bool getParen() const {return paren;}

    // From Entity
    void print(std::ostream &stream) const;
  };
}

#endif // CAMOTICS_COMMENT_H

