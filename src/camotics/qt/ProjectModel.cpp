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

#include "ProjectModel.h"

#include <gcode/ToolTable.h>
#include <camotics/project/Project.h>

#include <cbang/SStream.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


ProjectModel::ProjectModel(const SmartPointer<Project::Project> &project,
                           QObject *parent) :
  QAbstractItemModel(parent), project(project) {
}


ProjectModel::~ProjectModel() {}


void ProjectModel::setProject(const SmartPointer<Project::Project> &project) {
  beginResetModel();
  this->project = project;
  endResetModel();
}


string ProjectModel::getFile(unsigned i) const {
  return project->getFileRelativePath(i);
}


string ProjectModel::getFile(const QModelIndex &index) const {
  return getFile(getOffset(index));
}


GCode::Tool &ProjectModel::getTool(unsigned i) const {
  GCode::ToolTable &tools = project->getTools();

  GCode::ToolTable::iterator it = tools.begin();
  for (; i && it != tools.end(); it++) i--;

  if (it == tools.end()) THROW("Invalid tool index");

  return it->second;
}


GCode::Tool &ProjectModel::getTool(const QModelIndex &index) const {
  return getTool(getOffset(index));
}


string ProjectModel::getToolString(unsigned i) const {
  GCode::Tool &tool = getTool(i);
  return SSTR(tool.getNumber() << ": " << tool.getText());
}


QModelIndex ProjectModel::getToolIndex(unsigned number) const {
  // Find tool index
  GCode::ToolTable &tools = project->getTools();
  int row = 0;
  for (GCode::ToolTable::iterator it = tools.begin(); it != tools.end(); it++) {
    if (it->first == number) break;
    row++;
  }

  return createIndex(row, 0, TOOL_ITEM, row);
}


QModelIndex ProjectModel::createIndex(int row, int column, item_t type,
                                      unsigned offset) const {
  return createIndex(row, column, (offset << 8) | type);
}


ProjectModel::item_t ProjectModel::getType(const QModelIndex &index) const {
  if (!index.isValid()) return NULL_ITEM;
  return (item_t)(index.internalId() & 0xff);
}


unsigned ProjectModel::getOffset(const QModelIndex &index) const {
  if (!index.isValid()) return 0;
  return index.internalId() >> 8;
}


QVariant ProjectModel::data(const QModelIndex &index, int role) const {
  if (role != Qt::DisplayRole) return QVariant();

  unsigned offset = getOffset(index);

  switch (getType(index)) {
  case PROJECT_ITEM: return QString("Project");
  case PATHS_ITEM: return QString("Paths");
  case FILE_ITEM: return QString::fromUtf8(getFile(offset).c_str());
  case TOOLS_ITEM: return QString("GCode::Tools");
  case TOOL_ITEM: return QString::fromUtf8(getToolString(offset).c_str());
  default: return QVariant();
  }
}


QModelIndex ProjectModel::index(int row, int column,
                                const QModelIndex &parent) const {
  if (row < 0 || column < 0) return QModelIndex();

  switch (getType(parent)) {
  case NULL_ITEM: return createIndex(0, 0, PROJECT_ITEM);
  case PATHS_ITEM: return createIndex(row, column, FILE_ITEM, row);
  case TOOLS_ITEM: return createIndex(row, column, TOOL_ITEM, row);
  case PROJECT_ITEM:
    switch (row) {
    case 0: return createIndex(row, column, PATHS_ITEM);
    case 1: return createIndex(row, column, TOOLS_ITEM);
    }
    // Fall through

  default: return QModelIndex();
  }
}


QModelIndex ProjectModel::parent(const QModelIndex &index) const {
  switch (getType(index)) {
  case PATHS_ITEM: return createIndex(0, 0, PROJECT_ITEM);
  case TOOLS_ITEM: return createIndex(0, 0, PROJECT_ITEM);
  case FILE_ITEM: return createIndex(0, 0, PATHS_ITEM);
  case TOOL_ITEM: return createIndex(1, 0, TOOLS_ITEM);
  default: return QModelIndex();
  }
}


int ProjectModel::rowCount(const QModelIndex &parent) const {
  switch (getType(parent)) {
  case NULL_ITEM: return 1;
  case PROJECT_ITEM: return 2;
  case PATHS_ITEM: return project->getFileCount();
  case TOOLS_ITEM: return project->getTools().size();
  default: return 0;
  }
}


int ProjectModel::columnCount(const QModelIndex &parent) const {
  return 1;
}
