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

#include "NetList.h"

#include <cbang/Exception.h>

using namespace std;
using namespace OpenSCAM::PCB;


void NetList::write(ostream &stream) const {
  stream << "NetList()\n(";
  Layout::write(stream);
  stream << ")";
}


void NetList::read(istream &stream) {
  readWS(stream);
  int c = stream.get();
  if (c != ')')  THROW("missing ')' on NetList");

  readWS(stream);
  c = stream.get();
  if (c != '(')  THROW("missing '(' on NetList");

  Layout::read(stream);
}


void NetList::merge(const NetList &netList) {
  if (netList.objects.empty())
    suffix += netList.suffix;

  else {
    const_iterator it = netList.objects.begin();

    (*it)->prefix = suffix + (*it)->prefix;

    for (; it != netList.objects.end(); it++)
      objects.push_back(*it);
    
    suffix = netList.suffix;
  }
}
