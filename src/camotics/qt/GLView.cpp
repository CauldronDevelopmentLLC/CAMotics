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

#include "GLView.h"

#include "QtWin.h"

#include <camotics/view/GL.h>
#include <camotics/view/View.h>

#include <cbang/Catch.h>
#include <cbang/log/Logger.h>

#include <QGLFormat>
#include <QMessageBox>

using namespace CAMotics;


GLView::GLView(QWidget *parent) : QOpenGLWidget(parent), enabled(true) {
  QSurfaceFormat format = QSurfaceFormat::defaultFormat();
  format.setSamples(4);
  setFormat(format);
}


QtWin &GLView::getQtWin() const {
  QtWin *qtWin = dynamic_cast<QtWin *>(window());
  if (!qtWin) THROW("QtWin not found");
  return *qtWin;
}


View &GLView::getView() const {return getQtWin().getView();}
void GLView::redraw(bool now) {getQtWin().redraw(now);}


void GLView::mousePressEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton)
    getView().startRotation(event->x(), event->y());

  else if (event->buttons() & (Qt::RightButton | Qt::MidButton))
    getView().startTranslation(event->x(), event->y());
}


void GLView::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    getView().updateRotation(event->x(), event->y());
    redraw(true);

  } else if (event->buttons() & (Qt::RightButton | Qt::MidButton)) {
    getView().updateTranslation(event->x(), event->y());
    redraw(true);
  }
}


void GLView::wheelEvent(QWheelEvent *event) {
  if (event->delta() < 0) getView().zoomIn();
  else getView().zoomOut();

  redraw(true);
}


void GLView::initializeGL() {
  if (!enabled) return;

  try {
    LOG_DEBUG(5, "initializeGL()");
    getView().glInit();
    return;
  } CATCH_ERROR;

  enabled = false;
}


void GLView::resizeGL(int w, int h) {
  if (!enabled) return;
  LOG_DEBUG(5, "resizeGL(" << w << ", " << h << ")");
  getView().resize(w, h);
}


void GLView::paintGL() {
  if (!enabled) return;
  LOG_DEBUG(5, "paintGL()");
  getGLFuncs().glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  getView().draw();
}
