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

#include "Connection.h"

#include <cbang/Exception.h>
#include <cbang/String.h>

#include <cbang/util/SmartLock.h>
#include <cbang/util/DefaultCatch.h>

#include <cbang/socket/SocketDevice.h>

#include <cbang/time/Time.h>
#include <cbang/time/Timer.h>

#include <cctype>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


Connection::Connection(const IPAddress &addr, const string &password,
                       const string &enablePass) :
  addr(addr), password(password), enablePass(enablePass),
  lastConnectAttempt(0), lastUpdate(0), authenticated(false), updated(false) {
  start();
}


bool Connection::isConnected() const {
  SmartLock lock(this);
  return isOpen() && authenticated;
}


bool Connection::wasUpdated() {
  SmartLock lock(this);
  bool x = updated;
  updated = false;
  return x;
}


void Connection::addUpdate(const string &name) {
  SmartLock lock(this);
  updates.insert(name);
}


const string &Connection::get(const string &name) const {
  SmartLock lock(this);
  vars_t::const_iterator it = vars.find(name);
  if (it == vars.end()) THROWS("Variable '" << name << "' not set");
  return it->second;
}


void Connection::run() {
  SocketStream stream(*this);
  Timer timer;

  while (!shouldShutdown())
    try {
      // Check stream
      if (stream.bad()) {
        LOG_INFO(1, "Disconnected");
        updated = true;
        stream.clear();
        close();

      } else if (stream.fail() || stream.eof()) stream.clear();

      // Connect
      if (!isOpen()) {
        int64_t wait = (lastConnectAttempt + 5) - Time::now();
        if (0 < wait) Timer::sleep(wait);

        lastConnectAttempt = Time::now();
        LOG_INFO(1, "Connecting to " << addr);
        setBlocking(true);
        connect(addr);
        setBlocking(false);
        authenticated = false;
        updated = true;
        lastUpdate = 0;
      }

      if (isOpen()) {
        // Authenticate
        if (!authenticated) {
          LOG_DEBUG(5, "Authenticating Connection");
          stream
            << "hello " << password << " OpenSCAM 1.0\r\n"
            << "set enable " << enablePass << "\r\n"
            << "set echo off\r\n"
            << "set update auto\r\n";

          stream.flush();
          char buffer[1024];
          while (true) {
            stream.readsome(buffer, 1024);
            if (!stream.gcount() || stream.fail()) break;
            LOG_DEBUG(5, String::escapeC(buffer));
          }

          // TODO check for ACK

          authenticated = true;
          updated = true;
        }

        // Request updates
        if (lastUpdate + 0.25 < Timer::now()) {
          LOG_DEBUG(5, "Requesting updates");

          lock();
          updates_t::iterator it;
          for (it = updates.begin(); it != updates.end(); it++)
            stream << "get " << *it << "\r\n";
          unlock();

          stream.flush();
          lastUpdate = Timer::now();
        }

        // Read vars
        char buffer[4096];
        while (true) {
          LOG_DEBUG(5, "Reading variables");

          stream.peek();
          if (stream.eof() || stream.fail()) break;
          stream.getline(buffer, 4096);
          if (!stream.gcount() || stream.fail()) break;

          LOG_DEBUG(5, "Line: " << String::escapeC(buffer));

          // Find space
          char *ptr = buffer;
          bool isKey = true;
          while (*ptr && !isspace(*ptr)) {
            if (!isupper(*ptr) && *ptr != '_') isKey = false;
            ptr++;
          }

          if (!isKey) continue;

          if (*ptr) *ptr++ = 0; // Terminate key

          // Set it
          string value = String::trim(ptr);
          lock();
          vars_t::iterator it = vars.find(buffer);
          if (it == vars.end() || it->second != value) updated = true;
          vars[buffer] = value;
          unlock();
        }
      }

      timer.throttle(0.1);
    } CBANG_CATCH_ERROR;

  // Close
  if (isOpen()) {
    if (authenticated) {
      stream << "quit\n";
      stream.flush();
    }

    close();
  }
}
