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

#include <cbang/StdTypes.h>

#include <string>
#include <ios>


namespace CAMotics {
  class SHA256 {
    uint8_t data[64];
	uint32_t datalen;
    uint64_t bitlen;
	uint32_t state[8];

  public:
    SHA256() {init();}

    void init();
    void update(const uint8_t data[], size_t len);
    void update(const char *data, std::streamsize len);
    void update(const std::string &data);
    void finalize(uint8_t hash[32]);
    std::string finalize();

  protected:
    void transform();
  };
}
