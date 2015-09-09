/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
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

#ifndef CBANG_ENUM_EXPAND
#ifndef TOKEN_TYPE_H
#define TOKEN_TYPE_H

#define CBANG_ENUM_NAME TokenType
#define CBANG_ENUM_NAMESPACE CAMotics
#define CBANG_ENUM_PATH camotics/gcode
#include <cbang/enum/MakeEnumeration.def>

#endif // TOKEN_TYPE_H
#else // CBANG_ENUM_EXPAND

CBANG_ENUM_EXPAND(EOF_TOKEN,            0)
CBANG_ENUM_EXPAND(COMMENT_TOKEN,        1)
CBANG_ENUM_EXPAND(PAREN_COMMENT_TOKEN,  2)
CBANG_ENUM_EXPAND(NUMBER_TOKEN,         3)
CBANG_ENUM_EXPAND(ID_TOKEN,             4)
CBANG_ENUM_EXPAND(EXP_TOKEN,            5)
CBANG_ENUM_EXPAND(MUL_TOKEN,            6)
CBANG_ENUM_EXPAND(DIV_TOKEN,            7)
CBANG_ENUM_EXPAND(ADD_TOKEN,            8)
CBANG_ENUM_EXPAND(SUB_TOKEN,            9)
CBANG_ENUM_EXPAND(OBRACKET_TOKEN,       10)
CBANG_ENUM_EXPAND(CBRACKET_TOKEN,       11)
CBANG_ENUM_EXPAND(OANGLE_TOKEN,         12)
CBANG_ENUM_EXPAND(CANGLE_TOKEN,         13)
CBANG_ENUM_EXPAND(ASSIGN_TOKEN,         14)
CBANG_ENUM_EXPAND(POUND_TOKEN,          15)
CBANG_ENUM_EXPAND(EOL_TOKEN,            16)

#endif // CBANG_ENUM_EXPAND
