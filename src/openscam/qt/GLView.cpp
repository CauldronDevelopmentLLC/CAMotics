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

#include "GLView.h"

#include "QtWin.h"

#include <QGLFormat>

using namespace OpenSCAM;


GLView::GLView(QWidget *parent) :
  QGLWidget(QGLFormat(QGL::AlphaChannel | QGL::SampleBuffers), parent), id(0),
  qtWin(0) {}


void GLView::mousePressEvent(QMouseEvent *event) {
  if (qtWin) qtWin->glViewMousePressEvent(id, event);
}


void GLView::mouseMoveEvent(QMouseEvent *event) {
  if (qtWin) qtWin->glViewMouseMoveEvent(id, event);
}


void GLView::wheelEvent(QWheelEvent *event) {
  if (qtWin) qtWin->glViewWheelEvent(id, event);
}


void GLView::initializeGL() {
  if (qtWin) qtWin->initializeGL(id);
}


void GLView::resizeGL(int w, int h) {
  if (qtWin) qtWin->resizeGL(id, w, h);
}


void GLView::paintGL() {
  if (qtWin) qtWin->paintGL(id);
}
