/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "PlanQueue.h"
#include "PlanCommand.h"

using namespace GCode;
using namespace cb;


PlanQueue::~PlanQueue() {while (!empty()) pop();}


void PlanQueue::push(const SmartPointer<PlanCommand> &cmd) {
  if (empty()) last = first = cmd;
  else {
    cmd->prev = last;
    last->next = cmd;
    last = cmd;
  }
}


SmartPointer<PlanCommand> PlanQueue::pop() {
  if (empty()) THROW("Plan queue empty");

  SmartPointer<PlanCommand> cmd = first;
  first = cmd->next;

  if (first.isNull()) last = 0;
  else {
    first->prev = 0;
    cmd->next = 0;
  }

  return cmd;
}
