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

#ifndef OPENSCAM_CONNECTION_H
#define OPENSCAM_CONNECTION_H

#include <cbang/StdTypes.h>
#include <cbang/os/Thread.h>
#include <cbang/net/IPAddress.h>
#include <cbang/socket/Socket.h>

#include <map>
#include <set>

namespace OpenSCAM {
  class Connection : public cb::Socket, public cb::Thread, public cb::Mutex {
    const cb::IPAddress addr;
    std::string password;
    std::string enablePass;

    uint64_t lastConnectAttempt;
    double lastUpdate;
    bool authenticated;
    bool updated;

    typedef std::map<std::string, std::string> vars_t;
    vars_t vars;
    typedef std::set<std::string> updates_t;
    updates_t updates;

  public:
    Connection(const cb::IPAddress &addr, const std::string &password,
               const std::string &enablePass);

    bool isConnected() const;
    bool wasUpdated();

    void addUpdate(const std::string &name);
    const std::string &get(const std::string &name) const;

    using Socket::get;

  protected:
    // From Thread
    void run();
  };
}

#endif // OPENSCAM_CONNECTION_H

