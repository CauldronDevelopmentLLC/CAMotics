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

#pragma once


#include <cbang/SmartPointer.h>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>


namespace GCode {class Tool;}

namespace CAMotics {
  namespace Project {class Project;}

  class ProjectModel : public QAbstractItemModel {
    Q_OBJECT;

    cb::SmartPointer<Project::Project> project;

  public:
    typedef enum {
      NULL_ITEM,
      PROJECT_ITEM,
      PATHS_ITEM,
      FILE_ITEM,
      TOOLS_ITEM,
      TOOL_ITEM,
    } item_t;

    ProjectModel(const cb::SmartPointer<Project::Project> &project,
                 QObject *parent = 0);
    ~ProjectModel();

    void setProject(const cb::SmartPointer<Project::Project> &project);

    std::string getFile(unsigned i) const;
    std::string getFile(const QModelIndex &index) const;

    GCode::Tool &getTool(unsigned i) const;
    GCode::Tool &getTool(const QModelIndex &index) const;
    std::string getToolString(unsigned i) const;
    QModelIndex getToolIndex(unsigned number) const;

    QModelIndex createIndex(int row, int column, item_t type,
                            unsigned offset) const;
    using QAbstractItemModel::createIndex;
    item_t getType(const QModelIndex &index) const;
    unsigned getOffset(const QModelIndex &index) const;
    void invalidate() {layoutAboutToBeChanged(); layoutChanged();}

    // From QAbstractItemModel
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
  };
}
