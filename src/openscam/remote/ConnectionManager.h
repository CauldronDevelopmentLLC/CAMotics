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

#ifndef OPENSCAM_CONNECTION_MANAGER_H
#define OPENSCAM_CONNECTION_MANAGER_H

#include "Connection.h"

#include <openscam/Geom.h>

#include <cbang/SmartPointer.h>


namespace cb {class Options;}

namespace OpenSCAM {
  class ConnectionManager {
    std::string connectionPass;
    std::string enablePass;
    cb::SmartPointer<Connection> connection;

    std::string connectionStr;
    bool connected;

    uint32_t programLine;
    std::string programStatus;
    std::string mode;
    cb::Vector3D position;
    bool machine;
    bool estop;

    bool synchronize;

  public:
    ConnectionManager(cb::Options &options);

    bool isConnected() const {return connected;}
    uint32_t getProgramLine() const {return programLine;}
    const std::string &getProgramStatus() const {return programStatus;}
    const std::string &getMode() const {return mode;}
    const cb::Vector3D &getPosition() const {return position;}
    bool getMachine() const {return machine;}
    bool getEStop() const {return estop;}

    bool getSynchronize() const {return synchronize;}
    void setSynchronize(bool x) {synchronize = x;}
    void toggleSynchronize() {synchronize = !synchronize;}

    void init();
    bool update();
  };
}

#endif // OPENSCAM_CONNECTION_MANAGER_H

