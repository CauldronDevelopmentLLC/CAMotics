/******************************************************************************\

    CAMotics is an Open-Source CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "ToolPathThread.h"

#include <camotics/cutsim/Project.h>

#include <cbang/js/Javascript.h>
#include <cbang/util/DefaultCatch.h>

using namespace CAMotics;
using namespace cb;


ToolPathThread::ToolPathThread(int event, QWidget *parent,
                               const SmartPointer<CutSim> &cutSim,
                               const SmartPointer<Project> &project) :
  CutSimThread(event, parent, cutSim),
  tools(new ToolTable(*project->getToolTable())) {

  for (Project::iterator it = project->begin(); it != project->end(); it++)
    files.push_back((*it)->getAbsolutePath());
}


void ToolPathThread::run() {
  v8::Locker locker;
  try {
    path = cutSim->computeToolPath(tools, files);
  } CATCH_ERROR;
  completed();
}
