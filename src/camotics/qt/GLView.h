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
#include <cbang/util/SmartFunctor.h>

#include <QOpenGLWidget>


class QOpenGLDebugLogger;
class QOpenGLDebugMessage;


namespace CAMotics {
  class QtWin;
  class View;

  class GLView : public QOpenGLWidget {
    Q_OBJECT;

    cb::SmartPointer<QOpenGLDebugLogger> logger;
    bool enabled;

  public:
    GLView(QWidget *parent = 0);
    ~GLView();

    bool isEnabled() const {return enabled;}

    QtWin &getQtWin() const;
    View &getView() const;
    void redraw(bool now = false);

  protected:
    // From QWidget
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void destroy(bool destroyWindow, bool destroySubWindows);

    // From OpenGLWidget
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    typedef cb::SmartPointer<cb::SmartFunctor<GLView> > SmartLog;
    SmartLog startLog();
    void logErrors();
    void handleLoggedMessage(const QOpenGLDebugMessage &msg);
  };
}
