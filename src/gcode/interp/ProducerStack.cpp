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

#include "ProducerStack.h"

#include "ProgramProducer.h"

#include <gcode/parse/Parser.h>
#include <cbang/io/StringStreamInputSource.h>


using namespace GCode;
using namespace cb;
using namespace std;


void ProducerStack::push(const SmartPointer<Producer> &producer) {
  producers.push_back(producer);
}


void ProducerStack::push(const InputSource &source) {push(new Parser(source));}


void ProducerStack::push(Program &program) {
  push(new ProgramProducer(SmartPointer<Program>::Phony(&program)));
}


void ProducerStack::push(const string &gcode, const string &path) {
  push(StringStreamInputSource(gcode, path));
}


SmartPointer<Producer> ProducerStack::peek() {
  if (empty()) THROW("ProducerStack empty");
  return producers.back();
}


SmartPointer<Producer> ProducerStack::pop() {
  SmartPointer<Producer> producer = peek();
  producers.pop_back();
  return producer;
}


void ProducerStack::unwind() {
  // Unwind stack in correct order
  while (!empty()) producers.pop_back();
}


bool ProducerStack::hasMore() const {
  for (auto it = producers.rbegin(); it != producers.rend(); it++)
    if ((*it)->hasMore()) return true;

  return false;
}


cb::SmartPointer<Block> ProducerStack::next() {
  while (!peek()->hasMore()) producers.pop_back();
  if (empty()) THROW("ProducerStack empty");
  return producers.back()->next();
}
