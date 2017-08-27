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

#include "GLView.h"

#include "QtWin.h"

#include <cbang/log/Logger.h>

#include <QGLFormat>

using namespace CAMotics;


GLView::GLView(QWidget *parent) :QOpenGLWidget(parent) {
  QSurfaceFormat format = QSurfaceFormat::defaultFormat();
  format.setSamples(4);
  setFormat(format);
}


QtWin &GLView::getQtWin() const {
  QtWin *qtWin = dynamic_cast<QtWin *>(window());
  if (!qtWin) THROW("QtWin not found");
  return *qtWin;
}


void GLView::mousePressEvent(QMouseEvent *event) {
  getQtWin().glViewMousePressEvent(event);
}


void GLView::mouseMoveEvent(QMouseEvent *event) {
  getQtWin().glViewMouseMoveEvent(event);
}


void GLView::wheelEvent(QWheelEvent *event) {
  getQtWin().glViewWheelEvent(event);
}


void GLView::initializeGL() {getQtWin().initializeGL();}
void GLView::resizeGL(int w, int h) {getQtWin().resizeGL(w, h);}
void GLView::paintGL() {getQtWin().paintGL();}
