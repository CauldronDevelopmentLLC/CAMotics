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

#include <camotics/view/GLContext.h>
#include <camotics/view/View.h>

#include <cbang/Catch.h>
#include <cbang/log/Logger.h>

#include <QGLFormat>
#include <QMessageBox>
#include <QOpenGLDebugLogger>

using namespace CAMotics;
using namespace cb;
using namespace std;


GLView::GLView(QWidget *parent) : QOpenGLWidget(parent), enabled(true) {
  QSurfaceFormat format = QSurfaceFormat::defaultFormat();
  format.setSamples(4);

#ifdef DEBUG
  format.setOption(QSurfaceFormat::DebugContext);
#endif // DEBUG

  setFormat(format);

  // Build square spiral picking pattern, starting from center
  pickingPattern.push_back(vector<int>{0, 0});

  for (int radStep = 1; radStep <= pickingRadius; radStep++) {
    for (int topX = -radStep; topX < radStep; topX++) {
      pickingPattern.push_back(vector<int>{topX, radStep});
    }
    for (int rightY = radStep; rightY > -radStep; rightY--) {
      pickingPattern.push_back(vector<int>{radStep, rightY});
    }
    for (int botX = radStep; botX > -radStep; botX--) {
      pickingPattern.push_back(vector<int>{botX, -radStep});
    }
    for (int leftY = -radStep; leftY < radStep; leftY++) {
      pickingPattern.push_back(vector<int>{-radStep, leftY});
    }
  }
}


GLView::~GLView() {}


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

  else if (event->buttons() & Qt::MidButton)
    getView().startTranslation(event->x(), event->y());
}


void GLView::mouseDoubleClickEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    doPicking = true;
    xPicking = event->x();
    yPicking = event->y();
    redraw(true);
  }
}


void GLView::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    getView().updateRotation(event->x(), event->y());
    redraw(true);

  } else if (event->buttons() & Qt::MidButton) {
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

#ifdef DEBUG
    if (logger.isNull()) {
      logger = new QOpenGLDebugLogger(this);

      if (logger->initialize())
        connect(logger.get(), &QOpenGLDebugLogger::messageLogged, this,
                &GLView::handleLoggedMessage);

      else {
        logger.release();
        LOG_ERROR("initializeGL() Failed to initialize OpenGL logger");
      }
    }
#endif

    SmartLog log = startLog();
    getView().glInit();
    return;
  } CATCH_ERROR;

  enabled = false;
}


void GLView::resizeGL(int w, int h) {
  if (!enabled) return;
  LOG_DEBUG(5, "resizeGL(" << w << ", " << h << ")");

  SmartLog log = startLog();
  getView().glResize(w, h);
}


void GLView::paintGL() {
  if (!enabled) return;
  LOG_DEBUG(5, "paintGL()");
  SmartLog log = startLog();

  // If color picking, only draw pickable objects to a unsampled framebuffer
  if (doPicking) {
    getView().glDraw(true);
    doPicking = false;
    QImage image = grabFramebuffer();

    // Adjust mouse position with ratio of buffer dims by widget dims
    xPicking *= image.width()  / (float)width();
    yPicking *= image.height() / (float)height();

    // Search area around mouse for pickable objects
    int xMin = max(0, xPicking - pickingRadius);
    int xMax = min(image.width() - 1, xPicking + pickingRadius);
    int yMin = max(0, yPicking - pickingRadius);
    int yMax = min(image.height() - 1, yPicking + pickingRadius);
    vector<unsigned> moveList;

    for (int i = 0; i < pickingPattern.size(); i++) {
      int x = xPicking + pickingPattern[i][0];
      int y = yPicking + pickingPattern[i][1];

      if (x >= xMin && x <= xMax && y >= yMin && y <= yMax) {
        QColor c = image.pixelColor(x, y);

        if (c != QColor(0, 0, 0, 255)) {
          // Convert picked color back to tool path line number
          unsigned moveIndex = Color::toIndex(c.redF(), c.greenF(), c.blueF());

          if (!std::count(moveList.begin(), moveList.end(), moveIndex)) {
            moveList.push_back(moveIndex);
          }
        }
      }
    }

    if (!moveList.empty()) {
      auto nextMove = std::find(moveList.begin(), moveList.end(), selectedMove);

      // Rotate selection through found paths
      if (nextMove == moveList.end() || ++nextMove == moveList.end())
        nextMove = moveList.begin();

      selectedMove = *nextMove;

      // Set tool path based on picked move
      auto &path = *getView().path->getPath();

      if (selectedMove < path.size()) {
        auto &move = path.at(selectedMove);

        if (move.getFilename().isSet()) {
          getView().path->setByLine(*move.getFilename(), move.getLine());
          getQtWin().activateFile(*move.getFilename(), move.getLine());
          redraw(true);
        }
      }
    }
  }

  getView().glDraw();
}


namespace {
  int glDebugLevel(QOpenGLDebugMessage::Severity severity) {
    switch (severity) {
    case QOpenGLDebugMessage::MediumSeverity: return CBANG_LOG_WARNING_LEVEL;
    case QOpenGLDebugMessage::LowSeverity:    return CBANG_LOG_DEBUG_LEVEL(1);
    case QOpenGLDebugMessage::NotificationSeverity:
      return CBANG_LOG_DEBUG_LEVEL(3);
    default: return CBANG_LOG_ERROR_LEVEL;
    }
  }


  const char *glDebugSource(QOpenGLDebugMessage::Source source) {
    switch (source) {
    case QOpenGLDebugMessage::APISource:            return "API";
    case QOpenGLDebugMessage::WindowSystemSource:   return "GUI";
    case QOpenGLDebugMessage::ShaderCompilerSource: return "Shader";
    case QOpenGLDebugMessage::ThirdPartySource:     return "3rd";
    case QOpenGLDebugMessage::ApplicationSource:    return "App";
    case QOpenGLDebugMessage::OtherSource:          return "Other";
    default:                                        return "Unknown";
    }
  }


  const char *glDebugType(QOpenGLDebugMessage::Type type)  {
    switch (type) {
    case QOpenGLDebugMessage::ErrorType:              return "Error";
    case QOpenGLDebugMessage::DeprecatedBehaviorType: return "Deprecated";
    case QOpenGLDebugMessage::UndefinedBehaviorType:  return "Undefined";
    case QOpenGLDebugMessage::PortabilityType:        return "Portability";
    case QOpenGLDebugMessage::PerformanceType:        return "Performance";
    case QOpenGLDebugMessage::MarkerType:             return "Marker";
    case QOpenGLDebugMessage::GroupPushType:          return "Group Push";
    case QOpenGLDebugMessage::GroupPopType:           return "Group Pop";
    case QOpenGLDebugMessage::OtherType:              return "Other";
    default:                                          return "Unknown";
    }
  }
}


GLView::SmartLog GLView::startLog() {
  if (logger.isSet()) logger->startLogging();
  GLContext().clearErrors();
  return new SmartFunctor<GLView>(this, &GLView::logErrors);
}


void GLView::logErrors() {
  if (logger.isSet()) logger->stopLogging();
  GLContext().logErrors();
}


void GLView::handleLoggedMessage(const QOpenGLDebugMessage &msg) {
  if (msg.id() == 131218) return; // Annoying NVidia warning

  LOG_LEVEL(glDebugLevel(msg.severity()), "GL:"
            << glDebugSource(msg.source()) << ':'
            << glDebugType(msg.type()) << ':' << msg.message().toStdString());
}
