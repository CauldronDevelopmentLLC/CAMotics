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

#include "QtApp.h"

#include "QtWin.h"
#include "QApplication.h"

#include <cbang/Info.h>
#include <cbang/log/Logger.h>
#include <cbang/os/SystemInfo.h>

#include <QTranslator>

#include <vector>

using namespace std;
using namespace cb;
using namespace CAMotics;


QtApp::QtApp() :
  // NOTE MSVC requires the explicit _hasFeature reference
  CAMotics::Application("CAMotics", CAMotics::Application::_hasFeature),
  threads(SystemInfo::instance().getCPUCount()) {
  options.add("qt-style", "Set Qt style");
  options.add("fullscreen", "Start in fullscreen mode.")->setDefault(false);
  options.add("auto-play", "Automatically start tool path playback.")
    ->setDefault(false);
  options.add("play-speed", "Set playback speed.")->setDefault(1);
  options.add("auto-close", "Automatically exit after tool path playback is "
              "complete.  Only valid with 'auto-play'")
    ->setDefault(false);
  options.addTarget("threads", threads, "GCode::Number of simulation threads.");

  // Configure Logger
  Logger &logger = Logger::instance();
  logger.setLogTime(false);
  logger.setLogNoInfoHeader(true);
  logger.setLogCRLF(false);
  logger.setLogColor(true);

  // Configure command line
  cmdLine.setAllowConfigAsFirstArg(false);
  cmdLine.setAllowPositionalArgs(true);
  cmdLine.setWarnOnInvalidArgs(true);
}


QtApp::~QtApp() {}


int QtApp::init(int argc, char *argv[]) {
  this->argc = argc;
  this->argv = argv;

  int ret = Application::init(argc, argv);
  if (ret < 0) return ret;

  QGuiApplication guiApp(argc, argv);
  QScreen *screen = guiApp.primaryScreen();
  Info::instance().add("System", "DPI", String(screen->logicalDotsPerInch()));

  printInfo();

  // Get project file
  const vector<string> &args = cmdLine.getPositionalArgs();
  if (1 <= args.size()) {
    projectFile = args[0];
    if (1 < args.size()) LOG_WARNING("Ignoring extra positional arguments");
  }

  return 0;
}


void QtApp::run() {
  vector<const char *> args;
  args.push_back(argv[0]);
  if (options["qt-style"].hasValue()) {
    args.push_back("-style");
    args.push_back(strdup(options["qt-style"].toString().c_str()));
  }

  // These must come before QApplication is constructed.
  string org = Info::instance().get(getName(), "Org");
  QCoreApplication::setOrganizationName(QString::fromUtf8(org.c_str()));
  QCoreApplication::setApplicationName(QString::fromUtf8(getName().c_str()));
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
  QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

  int argc = args.size();
  QApplication qtApp(argc, (char **)&args[0]);

  QtWin qtWin(*this, qtApp);
  qtWin.init();

  // Options
  if (options["fullscreen"].toBoolean())
    qtWin.setWindowState(qtWin.windowState() | Qt::WindowFullScreen);

  if (projectFile.empty()) qtWin.loadDefaultExample();
  else qtWin.openProject(projectFile);

  qtWin.getView().setSpeed(options["play-speed"].toInteger());

  if (options["auto-play"].toBoolean()) {
    qtWin.setAutoPlay();
    if (options["auto-close"].toBoolean()) qtWin.setAutoClose();
  }

  // Start it up
  qtWin.show();
  qtApp.exec();
}


void QtApp::requestExit() {Application::requestExit();}
