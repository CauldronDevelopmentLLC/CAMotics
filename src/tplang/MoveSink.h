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

#ifndef TPLANG_MOVE_SINK_H
#define TPLANG_MOVE_SINK_H

#include "MachineAdapter.h"
#include "MoveStream.h"

#include <cbang/config/Options.h>


namespace tplang {
  class MoveSink : public MachineAdapter {
    MoveStream &stream;

    bool first;
    bool probePending;
    double rapidFeed;
    double time;

  public:
    MoveSink(MoveStream &stream, cb::Options &options);

    // From MachineInterface
    void reset();
    double input(unsigned port, input_mode_t mode, double timeout, bool error);

    void move(const tplang::Axes &axes, bool rapid);
    void arc(const cb::Vector3D &offset, double degrees, plane_t plane);
  };
}

#endif // TPLANG_MOVE_SINK_H

