/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
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

#include "QtApp.h"

#include "QtWin.h"
#include "QApplication.h"

#include <cbang/Info.h>
#include <cbang/log/Logger.h>

#include <vector>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


QtApp::QtApp(QWidget *parent) : OpenSCAM::Application("OpenSCAM") {
  options.add("qt-style", "Set Qt style");
  options.add("fullscreen", "Start in fullscreen mode.")->setDefault(false);
  options.add("auto-play", "Automatically start tool path playback.")
    ->setDefault(false);
  options.add("play-speed", "Set playback speed.")->setDefault(1);
  options.add("auto-close", "Automatically exit after tool path playback is "
              "complete.  Only valid with 'auto-play'")
    ->setDefault(false);

  // Configure Logger
  Logger &logger = Logger::instance();
  logger.setLogTime(false);
  logger.setLogNoInfoHeader(true);

  // Configure command line
  cmdLine.setAllowConfigAsFirstArg(false);
  cmdLine.setAllowPositionalArgs(true);
  cmdLine.setWarnOnInvalidArgs(true);
}


QtApp::~QtApp() {}


int QtApp::init(int argc, char *argv[]) {
  this->argc = argc;
  this->argv = argv;

  int ret = cb::Application::init(argc, argv);
  if (ret < 0) return ret;

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

  int argc = args.size();
  QApplication qtApp(argc, (char **)&args[0]);

  string org = Info::instance().get(getName(), "Organization");
  QCoreApplication::setOrganizationName(QString::fromAscii(org.c_str()));
  QCoreApplication::setApplicationName(QString::fromAscii(getName().c_str()));

  QtWin qtWin(*this);
  qtWin.init();

  // Options
  if (options["fullscreen"].toBoolean())
    qtWin.setWindowState(qtWin.windowState() | Qt::WindowFullScreen);

  if (projectFile.empty()) qtWin.newProject();
  else qtWin.openProject(projectFile);

  qtWin.getView()->setSpeed(options["play-speed"].toInteger());

  if (options["auto-play"].toBoolean()) {
    qtWin.setAutoPlay();
    if (options["auto-close"].toBoolean()) qtWin.setAutoClose();
  }

  // Start it up
  qtWin.show();
  qtApp.exec();
}
