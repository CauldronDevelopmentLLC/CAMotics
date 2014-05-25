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

#include "ProjectModel.h"

#include <openscam/sim/ToolTable.h>
#include <openscam/cutsim/Project.h>

#include <cbang/SStream.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


ProjectModel::ProjectModel(const SmartPointer<Project> &project,
                           QObject *parent) :
  QAbstractItemModel(parent), project(project) {
}


ProjectModel::~ProjectModel() {}


void ProjectModel::setProject(const SmartPointer<Project> &project) {
  beginResetModel();
  this->project = project;
  endResetModel();
}


string ProjectModel::getFile(unsigned i) const {
  return project->getFile(i)->getRelativePath();
}


string ProjectModel::getFile(const QModelIndex &index) const {
  return getFile(getOffset(index));
}


Tool &ProjectModel::getTool(unsigned i) const {
  ToolTable &tools = *project->getToolTable();

  ToolTable::iterator it = tools.begin();
  it++; // Skip tool zero
  for (; i && it != tools.end(); it++) i--;

  if (it == tools.end()) THROW("Invalid tool index");

  return *it->second;
}


Tool &ProjectModel::getTool(const QModelIndex &index) const {
  return getTool(getOffset(index));
}


string ProjectModel::getToolString(unsigned i) const {
  Tool &tool = getTool(i);
  return SSTR(tool.getNumber() << ": " << tool.getText());
}


QModelIndex ProjectModel::getToolIndex(unsigned number) const {
  // Find tool index
  ToolTable &tools = *project->getToolTable();
  int row = -1; // Skip tool zero
  for (ToolTable::iterator it = tools.begin(); it != tools.end(); it++) {
    if (it->first == number) break;
    row++;
  }

  return createIndex(row, 0, TOOL_ITEM, row);
}


QModelIndex ProjectModel::createIndex(int row, int column, item_t type,
                                      unsigned index) const {
  return createIndex(row, column, index << 8 | type);
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
  case FILE_ITEM: return QString::fromAscii(getFile(offset).c_str());
  case TOOLS_ITEM: return QString("Tools");
  case TOOL_ITEM: return QString::fromAscii(getToolString(offset).c_str());
  case WORKPIECE_ITEM: return QString("Workpiece");
  default: return QVariant();
  }
}


QModelIndex ProjectModel::index(int row, int column,
                                const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent)) return QModelIndex();

  switch (getType(parent)) {
  case PATHS_ITEM: return createIndex(row, column, FILE_ITEM, row);
  case TOOLS_ITEM: return createIndex(row, column, TOOL_ITEM, row);
  case NULL_ITEM:
  case PROJECT_ITEM:
    switch (row) {
    case 0: return createIndex(row, column, PATHS_ITEM);
    case 1: return createIndex(row, column, TOOLS_ITEM);
    case 2: return createIndex(row, column, WORKPIECE_ITEM);
    }
    // Fall through

  default: return QModelIndex();
  }
}


QModelIndex ProjectModel::parent(const QModelIndex &index) const {
  switch (getType(index)) {
  case PATHS_ITEM:
  case TOOLS_ITEM:
  case WORKPIECE_ITEM:
    return createIndex(0, 0, PROJECT_ITEM);
  case FILE_ITEM: return createIndex(0, 0, PATHS_ITEM);
  case TOOL_ITEM: return createIndex(1, 0, TOOLS_ITEM);
  default: return QModelIndex();
  }
}


int ProjectModel::rowCount(const QModelIndex &parent) const {
  switch (getType(parent)) {
  case NULL_ITEM:
  case PROJECT_ITEM: return 3;
  case PATHS_ITEM: return project->getFileCount();
  case TOOLS_ITEM: return project->getToolTable()->size() - 1;
  default: return 0;
  }
}


int ProjectModel::columnCount(const QModelIndex &parent) const {
  return 1;
}
