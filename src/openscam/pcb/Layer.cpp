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

#include "Layer.h"

#include <cbang/Exception.h>

using namespace std;
using namespace OpenSCAM::PCB;


void Layer::write(ostream &stream) const {
  stream << "Layer(" << number << " \"" << name << "\")\n(";
  Layout::write(stream);
  stream << ")";
}


void Layer::read(istream &stream) {
  stream >> number;
  name = readString(stream);
    
  readWS(stream);
  int c = stream.get();
  if (c != ')')  THROW("missing ')' on Layer");

  readWS(stream);
  c = stream.get();
  if (c != '(')  THROW("missing '(' on Layer");

  Layout::read(stream);
}


void Layer::merge(const Layer &layer) {
  const_iterator it = layer.objects.begin();

  if (it != layer.objects.end())
    (*it)->prefix = suffix + (*it)->prefix;

  for (; it != layer.objects.end(); it++)
    objects.push_back(*it);

  suffix = layer.suffix;
}
