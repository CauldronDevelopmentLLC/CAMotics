/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
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

#ifndef CAMOTICS_GLVIEW_H
#define CAMOTICS_GLVIEW_H

#include <QtGlobal>
#if defined(_WIN32) && QT_VERSION < 0x050000
#include <winsock2.h> // Must come before below
#endif

#include <QGLWidget>


namespace CAMotics {
  class QtWin;

  class GLView : public QGLWidget {
    Q_OBJECT;

  public:
    GLView(QWidget *parent = 0);
    ~GLView() {}

    QtWin &getQtWin() const;

    // From QWidget
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    // From QGLWidget
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
  };
}

#endif // CAMOTICS_GLVIEW_H
