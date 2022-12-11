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

#include "QApplication.h"
#include <QFileOpenEvent>
#include <QtDebug>

#include <cbang/Catch.h>

using namespace cb;


CAMotics::QApplication::QApplication(int &argc, char *argv[]) :
  ::QApplication(argc, argv) {}


bool CAMotics::QApplication::notify(QObject *receiver, QEvent *e) {
  try {
    return ::QApplication::notify(receiver, e);
  } CATCH_ERROR;

  return false;
}


bool CAMotics::QApplication::event(QEvent *event) {
  // https://doc.qt.io/qt-5/qfileopenevent.html
  if (event->type() == QEvent::FileOpen) {
    // macOS file open event; file double-clicked or dropped on Dock CAMotics
    QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
    qDebug() << "Open file " << openEvent->file();
    // TODO: getMainWindow(), unless there is an easier openProject()
    // getMainWindow()->openProject((openEvent->file()).toStdString());
    return true;
  }

  return ::QApplication::event(event);
}
