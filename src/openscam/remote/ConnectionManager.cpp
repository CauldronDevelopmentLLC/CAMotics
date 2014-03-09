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

#include "ConnectionManager.h"

#include <cbang/String.h>
#include <cbang/config/Options.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


ConnectionManager::ConnectionManager(Options &options) :
  connected(false), programLine(0), machine(false),
  estop(false), synchronize(false) {

  options.pushCategory("EMC2 Remote");
  options.addTarget("connection", connectionStr,
                    "address and port of a remove EMC2 connection");
  options.addTarget("connection-pass", connectionPass,
                    "EMC2 connection password");
  options.addTarget("enable-pass", enablePass, "EMC2 enable password, "
                    "defaults to the connection password");
  options.popCategory();
}


void ConnectionManager::init() {
  // Connection
  if (!connectionStr.empty()) {
    if (connectionPass.empty()) THROW("Must set connection password");
    if (enablePass.empty()) enablePass = connectionPass;
    connection = new Connection(connectionStr, connectionPass, enablePass);

    connection->addUpdate("program_line");
    connection->addUpdate("program_status");
    connection->addUpdate("estop");
    connection->addUpdate("machine");
    connection->addUpdate("mode");
    connection->addUpdate("rel_act_pos");
  }
}


bool ConnectionManager::update() {
  // Update connection information
  if (!connection.isNull() && connection->wasUpdated()) {
    connected = connection->isConnected();

    if (connected)
      try {
        programLine = String::parseU32(connection->get("PROGRAM_LINE"));
        programStatus = connection->get("PROGRAM_STATUS");
        estop = connection->get("ESTOP") == "ON";
        machine = connection->get("MACHINE") == "ON";
        mode = connection->get("MODE");

        vector<string> tokens;
        String::tokenize(connection->get("REL_ACT_POS"), tokens);
        if (3 <= tokens.size())
          position = Vector3D(String::parseDouble(tokens[0]),
                              String::parseDouble(tokens[1]),
                              String::parseDouble(tokens[2]));

      } catch (...) {} // Ignore errors

    return true;
  }

  return false;
}
