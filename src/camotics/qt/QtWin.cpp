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

#include "QtWin.h"
#include "Settings.h"

#include "ui_camotics.h"

#include <camotics/Geom.h>
#include <camotics/view/Viewer.h>
#include <camotics/cutsim/Project.h>
#include <camotics/cutsim/SimulationRun.h>
#include <camotics/cutsim/CutWorkpiece.h>
#include <camotics/remote/ConnectionManager.h>
#include <camotics/stl/STLWriter.h>
#include <camotics/cutsim/ToolPathTask.h>
#include <camotics/cutsim/SurfaceTask.h>
#include <camotics/cutsim/ReduceTask.h>
#include <camotics/opt/Opt.h>

#include <cbang/Application.h>
#include <cbang/os/SystemUtilities.h>
#include <cbang/os/DirectoryWalker.h>
#include <cbang/log/Logger.h>
#include <cbang/util/SmartInc.h>
#include <cbang/util/DefaultCatch.h>
#include <cbang/time/TimeInterval.h>

#include <QMouseEvent>
#include <QWheelEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QImageWriter>
#include <QMovie>
#include <QDesktopWidget>
#include <QDir>
#include <QStringListModel>

#include <vector>

using namespace std;
using namespace cb;
using namespace CAMotics;

#define LOCK_UI_UPDATES SmartInc<unsigned> inc(inUIUpdate)
#define PROTECT_UI_UPDATE if (inUIUpdate) return; LOCK_UI_UPDATES

const int QtWin::maxRecentsSize = 20;

QtWin::QtWin(Application &app) :
  QMainWindow(0), ui(new Ui::CAMoticsWindow), findDialog(false),
  findAndReplaceDialog(true), fileDialog(*this),
  taskCompleteEvent(0), app(app), options(app.getOptions()),
  connectionManager(new ConnectionManager(options)),
  view(new View(valueSet)), viewer(new Viewer), lastRedraw(0),
  dirty(false), simDirty(false), inUIUpdate(false), lastProgress(0),
  lastStatusActive(false), autoPlay(false), autoClose(false),
  sliderMoving(false) {

  ui->setupUi(this);

  // FileTabManager
  connect(ui->actionUndo, SIGNAL(triggered()),
          ui->fileTabManager, SLOT(on_actionUndo_triggered()));
  connect(ui->actionRedo, SIGNAL(triggered()),
          ui->fileTabManager, SLOT(on_actionRedo_triggered()));
  connect(ui->actionCut, SIGNAL(triggered()),
          ui->fileTabManager, SLOT(on_actionCut_triggered()));
  connect(ui->actionCopy, SIGNAL(triggered()),
          ui->fileTabManager, SLOT(on_actionCopy_triggered()));
  connect(ui->actionPaste, SIGNAL(triggered()),
          ui->fileTabManager, SLOT(on_actionPaste_triggered()));
  connect(ui->actionSelectAll, SIGNAL(triggered()),
          ui->fileTabManager, SLOT(on_actionSelectAll_triggered()));
  connect(&recentProjectsMapper, SIGNAL(mapped(QString)),
          this, SLOT(openRecentProjectsSlot(QString)));

  // Find
  connect(ui->console, SIGNAL(find()), &findDialog, SLOT(show()));
  connect(ui->console, SIGNAL(findNext()), &findDialog, SLOT(find()));
  connect(ui->console, SIGNAL(findResult(bool)), &findDialog,
          SLOT(findResult(bool)));
  connect(&findDialog, SIGNAL(find(QString, bool, int)), ui->console,
          SLOT(on_find(QString, bool, int)));

  // Find & Replace
  connect(ui->fileTabManager, SIGNAL(find()), &findAndReplaceDialog,
          SLOT(show()));
  connect(ui->fileTabManager, SIGNAL(findNext()), &findAndReplaceDialog,
          SLOT(find()));
  connect(ui->fileTabManager, SIGNAL(findResult(bool)), &findAndReplaceDialog,
          SLOT(findResult(bool)));
  connect(&findAndReplaceDialog, SIGNAL(find(QString, bool, int)),
          ui->fileTabManager, SLOT(on_find(QString, bool, int)));
  connect(&findAndReplaceDialog,
          SIGNAL(replace(QString, QString, bool, int, bool)),
          ui->fileTabManager,
          SLOT(on_replace(QString, QString, bool, int, bool)));

  // ConcurrentTaskManager
  taskMan.addObserver(this);
  taskCompleteEvent = QEvent::registerEventType();

  // Hide unimplemented console buttons
  ui->errorsCheckBox->setVisible(false);
  ui->warningsCheckBox->setVisible(false);

  // Hide unfinished optimize
  ui->actionOptimize->setVisible(false);

  // Load icons
  playIcon.addFile(QString::fromUtf8(":/icons/play.png"), QSize(),
                   QIcon::Normal, QIcon::Off);
  pauseIcon.addFile(QString::fromUtf8(":/icons/pause.png"), QSize(),
                    QIcon::Normal, QIcon::Off);
  forwardIcon.addFile(QString::fromUtf8(":/icons/forward.png"), QSize(),
                      QIcon::Normal, QIcon::Off);
  backwardIcon.addFile(QString::fromUtf8(":/icons/backward.png"), QSize(),
                       QIcon::Normal, QIcon::Off);

  // Load examples
  loadExamples();

  // Load recent projects
  loadRecentProjects();

  // Add docks to View menu
  QMenu *menu = new QMenu;
  QList<QDockWidget *> docks = findChildren<QDockWidget *>();
  for (int i = 0; i < docks.size(); i++)
    if (docks.at(i)->features() & QDockWidget::DockWidgetClosable)
      menu->addAction(docks.at(i)->toggleViewAction());
  ui->actionDocks->setMenu(menu);

  // Add toolbars to View menu
  menu = new QMenu;
  QList<QToolBar *> toolBars = findChildren<QToolBar *>();
  for (int i = 0; i < toolBars.size(); i++)
    menu->addAction(toolBars.at(i)->toggleViewAction());
  ui->actionToolbars->setMenu(menu);

  // Select workpiece radio
  ui->automaticCuboidRadioButton->setChecked(true);

  // Add status label to status bar
  statusLabel = new QLabel;
  statusBar()->addPermanentWidget(statusLabel);

  // Setup console stream
  consoleStream = new LineBufferStream<ConsoleWriter>(*ui->console);
  Logger::instance().setScreenStream(*consoleStream);
}


QtWin::~QtWin() {
  saveAllState();
  Logger::instance().setScreenStream(cout);
}


void QtWin::init() {
  // Start animation timer
  animationTimer.setSingleShot(false);
  connect(&animationTimer, SIGNAL(timeout()), this, SLOT(animate()));
  animationTimer.start(50);

  // Simulation and Tool View tabs are not closeable
  ui->fileTabManager->setTabsClosable(true);
  QTabBar *tabBar = ui->fileTabManager->findChild<QTabBar *>();
  for (int i = 0; i < tabBar->count(); i++) {
    tabBar->setTabButton(i, QTabBar::RightSide, 0);
    tabBar->setTabButton(i, QTabBar::LeftSide, 0);
  }

  // Hide console by default
  hideConsole();

  // Init GUI
  ui->fileTabManager->setCurrentIndex(0);
  updateActions();

  // Init Layout
  saveFullLayout();
  defaultLayout();
  setDefaultGeometry();
  restoreAllState();

  // Update fullscreen menu item
  if (windowState() & Qt::WindowFullScreen)
    ui->actionFullscreen->setChecked(true);

  connectionManager->init();

  // Observe values
  valueSet["play_speed"]->add(this, &QtWin::updatePlaySpeed);
  valueSet["view_flags"]->add(this, &QtWin::updateViewFlags);
  valueSet["play_direction"]->add(this, &QtWin::updatePlayDirection);
  valueSet["time_ratio"]->add(this, &QtWin::updateTimeRatio);

  valueSet["x"]->add(this, &QtWin::updateX);
  valueSet["y"]->add(this, &QtWin::updateY);
  valueSet["z"]->add(this, &QtWin::updateZ);

  valueSet["current_time"]->add(this, &QtWin::updateCurrentTime);
  valueSet["current_distance"]->add(this, &QtWin::updateCurrentDistance);
  valueSet["remaining_time"]->add(this, &QtWin::updateRemainingTime);
  valueSet["remaining_distance"]->add(this, &QtWin::updateRemainingDistance);
  valueSet["total_time"]->add(this, &QtWin::updateTotalTime);
  valueSet["total_distance"]->add(this, &QtWin::updateTotalDistance);
  valueSet["percent_time"]->add(this, &QtWin::updatePercentTime);
  valueSet["percent_distance"]->add(this, &QtWin::updatePercentDistance);

  valueSet["tool"]->add(this, &QtWin::updateTool);
  valueSet["feed"]->add(this, &QtWin::updateFeed);
  valueSet["speed"]->add(this, &QtWin::updateSpeed);
  valueSet["direction"]->add(this, &QtWin::updateDirection);
  valueSet["program_line"]->add(this, &QtWin::updateProgramLine);

  valueSet.updated();
}


void QtWin::setUnitLabel(QLabel *label, real value, int precision,
                         bool withUnit) {
  if (std::numeric_limits<real>::max() == abs(value) || Math::isinf(value) ||
      Math::isnan(value)) {
    label->setText("nan");
    return;
  }

  real scale = isMetric() ? 1.0 : 1.0 / 25.4;
  label->setText(QString().sprintf("%.*f%s", precision, value * scale,
                                   withUnit ? (isMetric() ? "mm" : "in") : ""));
}


void QtWin::loadDefaultExample() {
  if (defaultExample.empty()) newProject();
  else {
    openProject(defaultExample);
    snapView('t');
  }
}


void QtWin::loadExamples() {
  try {
    const char *paths[] = {
      "/usr/share/doc/camotics/examples",
      "../SharedSupport/examples",
      "examples",
      0
    };

    string root = ".";
    string appPath =
      QCoreApplication::applicationFilePath().toUtf8().data();
    if (appPath.empty()) LOG_WARNING("Couldn't get application path");
    else root = SystemUtilities::dirname(appPath);

    for (const char **p = paths; *p; p++) {
      string path = *p;
      if (path[0] != '/') path = root + "/" + path;

      if (SystemUtilities::isDirectory(path)) {
        typedef map<string, string> examples_t;
        examples_t examples;
        DirectoryWalker walker(path, ".*\\.xml", 2);

        while (walker.hasNext()) {
          string filename = walker.next();
          string dirname = SystemUtilities::dirname(filename);
          string basename = SystemUtilities::basename(filename);
          string name = basename.substr(0, basename.size() - 4);

          if (String::endsWith(dirname, "/" + name)) {
            name = String::trim(name);
            name = String::transcode(name, "_-", "  ");
            name = String::capitalize(name);

            examples[name] = filename;
            if (name == "Camotics") defaultExample = filename;
          }
        }

        if (examples.empty()) continue;

        QMenu *menu = new QMenu;

        ui->actionExamples->setMenu(menu);
        ui->actionExamples->setEnabled(true);

        examples_t::iterator it;
        for (it = examples.begin(); it != examples.end(); it++) {
          string name = it->first;
          string path = it->second;

          QAction *action = menu->addAction(QString::fromUtf8(name.c_str()));
          action->setToolTip(QString::fromUtf8(path.c_str()));
          connect(action, SIGNAL(triggered()), this,
                  SLOT(on_actionExamples_triggered()));
        }

        break;
      }
    }

  } CATCH_ERROR;
}


void QtWin::saveAllState() {
  Settings settings;
  settings.set("MainWindow/State", saveState());
  settings.set("MainWindow/Geometry", saveGeometry());
  settings.set("Console/Splitter", ui->splitter->saveState());
}


void QtWin::restoreAllState() {
  Settings settings;
  restoreState(settings.get("MainWindow/State").toByteArray());
  restoreGeometry(settings.get("MainWindow/Geometry").toByteArray());
  ui->splitter->restoreState(settings.get("Console/Splitter").toByteArray());
}


void QtWin::setDefaultGeometry() {
  int width = 1200;
  int height = 800;

  QDesktopWidget dw;
  QRect screen = dw.availableGeometry(this);
  QRect geom = geometry();
  QRect frame = frameGeometry();

  if (screen.width() < width) width = screen.width();
  if (screen.height() < height) height = screen.height();

  width -= frame.width() - geom.width();
  height -= frame.height() - geom.height();

  int x = (screen.width() - width) / 2;
  int y = (screen.height() - height) / 2;

  setGeometry(QRect(x, y, width, height));
}


void QtWin::saveFullLayout() {
  fullLayoutState = saveState();
}


void QtWin::fullLayout() {
  restoreState(fullLayoutState);
  showConsole();
}


void QtWin::defaultLayout() {
  restoreState(fullLayoutState);

  // Hide all closable docks
  QList<QDockWidget *> docks = findChildren<QDockWidget *>();
  for (int i = 0; i < docks.size(); i++)
    if (docks.at(i)->features() & QDockWidget::DockWidgetClosable)
      docks.at(i)->setVisible(false);

  hideConsole();
}


void QtWin::minimalLayout() {
  defaultLayout();

  // Hide all toolbars
  QList<QToolBar *> toolBars = findChildren<QToolBar *>();
  for (int i = 0; i < toolBars.size(); i++)
    toolBars.at(i)->setVisible(false);

  hideConsole();
}


void QtWin::snapView(char v) {
  view->resetView(v);
  redraw();
}


void QtWin::glViewMousePressEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton)
    view->startRotation(event->x(), event->y());

  else if (event->buttons() & (Qt::RightButton | Qt::MidButton))
    view->startTranslation(event->x(), event->y());
}


void QtWin::glViewMouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    view->updateRotation(event->x(), event->y());
    redraw(true);

  } else if (event->buttons() & (Qt::RightButton | Qt::MidButton)) {
    view->updateTranslation(event->x(), event->y());
    redraw(true);
  }
}


void QtWin::glViewWheelEvent(QWheelEvent *event) {
  if (event->delta() < 0) view->zoomIn();
  else view->zoomOut();

  redraw(true);
}


void QtWin::initializeGL() {
  view->glInit();
  viewer->init();
}


void QtWin::resizeGL(int w, int h) {
  LOG_DEBUG(5, "resizeGL(" << w << ", " << h << ")");
  view->resize(w, h);
}


void QtWin::paintGL() {
  LOG_DEBUG(5, "paintGL()");

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  viewer->draw(*view);
}


void QtWin::showMessage(const string &msg, double timeout) {
  ui->statusbar->showMessage(QString::fromUtf8(msg.c_str()), timeout * 1000);
}


void QtWin::message(const string &msg) {
  QMessageBox::information
    (this, "CAMotics", QString::fromUtf8(msg.c_str()), QMessageBox::Ok);
}


void QtWin::warning(const string &msg) {
  QMessageBox::warning
    (this, "CAMotics", QString::fromUtf8(msg.c_str()), QMessageBox::Ok);
}


void QtWin::loadToolPath(const SmartPointer<ToolPath> &toolPath,
                         bool simulate) {
  this->toolPath = toolPath;

  // Update changed Project settings
  project->updateAutomaticWorkpiece(*toolPath);
  project->updateResolution();

  // Setup view
  view->setFlag(View::PATH_VBOS_FLAG,
                Settings().get("Settings/VBO/Path", true).toBool());
  view->setToolPath(toolPath);
  view->setWorkpiece(project->getWorkpieceBounds());

  // Update UI
  loadWorkpiece();
  updateBounds();

  redraw();

  if (!simulate) {
    setStatusActive(false);
    return;
  }

  // Auto play
  if (autoPlay) {
    autoPlay = false;
    view->path->setByRatio(0);
    view->setFlag(View::PLAY_FLAG);
    view->setReverse(false);
  }

  // Simulation
  project->path = toolPath;
  project->time = view->getTime();
  project->threads = options["threads"].toInteger();
  project->workpiece = project->getWorkpieceBounds();

  // Load surface
  surface.release();
  view->setSurface(0);
  view->setMoveLookup(0);
  taskMan.addTask(new SurfaceTask(*project));
}


void QtWin::toolPathComplete(ToolPathTask &task) {
  if (task.getErrorCount()) {
    const char *msg = "Errors were encountered during tool path generation.  "
      "See the console output for more details";

    QMessageBox::critical(this, "Tool path errors", msg, QMessageBox::Ok);

    showConsole();
  }

  gcode = task.getGCode();
  loadToolPath(task.getPath(), !task.getErrorCount());
}


void QtWin::surfaceComplete(SurfaceTask &task) {
  simRun = task.getSimRun();
  surface = task.getSurface();
  if (surface.isNull()) simRun.release();

  view->setSurface(surface);
  view->setMoveLookup(simRun->getMoveLookup());
  view->setFlag(View::SURFACE_VBOS_FLAG,
                Settings().get("Settings/VBO/Surface", true).toBool());

  redraw();

  setStatusActive(false);
}


void QtWin::reduceComplete(ReduceTask &task) {
  surface = task.getSurface();
  view->setSurface(surface);
  redraw();

  setStatusActive(false);
}


void QtWin::optimizeComplete(Opt &opt) {
  loadToolPath(opt.getPath(), true);
}


void QtWin::quit() {
  stop();
  app.requestExit();
  taskMan.join();
  QCoreApplication::exit();
}


void QtWin::stop() {
  taskMan.interrupt();
  setStatusActive(false);
}


void QtWin::reload(bool now) {
  if (!now) {
    simDirty = true;
    return;
  }

  simDirty = false;

  // Reset console
  ui->console->document()->clear();
  ui->console->setTextColor(QColor("#d9d9d9"));

  try {
    // Queue tool path task
    taskMan.addTask(new ToolPathTask(*project));
    setStatusActive(true);
  } CATCH_ERROR;
}


void QtWin::reduce() {
  if (surface.isNull()) return;

  try {
    // Queue reduce task
    taskMan.addTask(new ReduceTask(*surface));
    setStatusActive(true);
  } CATCH_ERROR;
}


void QtWin::optimize() {
  if (toolPath.isNull()) return;

  try {
    // Queue optimize task
    taskMan.addTask(new Opt(*toolPath));
    setStatusActive(true);
  } CATCH_ERROR;
}


void QtWin::redraw(bool now) {
  if (now && 0.05 < Timer::now() - lastRedraw) {
    lastRedraw = Timer::now();

    updateWorkpieceBounds();
    ui->simulationView->updateGL();

    dirty = false;

  } else dirty = true;
}


void QtWin::snapshot() {
  string filename = project->getFilename();
  QPixmap pixmap =
    QPixmap::fromImage(ui->simulationView->grabFrameBuffer(true));

  QList<QByteArray> formats = QImageWriter::supportedImageFormats();
  string fileTypes = "Image files (";
  for (int i = 0; i < formats.size(); i++) {
    if (i) fileTypes += ",";
    fileTypes += "*." + String::toLower(formats.at(i).data());
  }
  fileTypes += ")";

  filename = SystemUtilities::swapExtension(filename, "png");
  filename = openFile("Save snapshot", fileTypes, filename, true);
  if (filename.empty()) return;

  if (!pixmap.save(QString::fromUtf8(filename.c_str())))
    warning("Failed to save snapshot.");
  else showMessage("Snapshot saved.");
}


void QtWin::exportData() {
  // Check what we have to export
  if (surface.isNull() && gcode.isNull() && project.isNull()) {
    warning("Nothing to export.\nRun a simulation first.");
    return;
  }

  exportDialog.enableSurface(!surface.isNull());
  exportDialog.enableGCode(!gcode.isNull());
  exportDialog.enableSimData(!project.isNull());

  // Run dialog
  if (exportDialog.exec() != QDialog::Accepted) return;

  // Select output file
  string title = "Export ";
  string fileTypes;
  string ext;

  if (exportDialog.surfaceSelected()) {
    title += "Surface";
    fileTypes = "STL Files (*.stl)";
    ext = "stl";

  } else if (exportDialog.gcodeSelected()) {
    title += "GCode";
    fileTypes = "GCode Files (*.gcode, *.nc, *.ngc *.tap)";
    ext = "gcode";

  } else {
    title += "Simulation Data";
    fileTypes = "JSON Files (*.json)";
    ext = "json";
  }

  fileTypes += ";;All Files (*.*)";

  // Open output file
  string filename = SystemUtilities::swapExtension(project->getFilename(), ext);
  filename = openFile(title, fileTypes, filename, true);

  if (filename.empty()) return;
  SmartPointer<iostream> stream = SystemUtilities::open(filename, ios::out);

  // Export
  if (exportDialog.surfaceSelected()) {
    string hash = project.isNull() ? "" : project->computeHash();
    bool binary = exportDialog.binarySTLSelected();
    surface->writeSTL(*stream, binary, "CAMotics Surface", hash);

  } else if (exportDialog.gcodeSelected()) {
    stream->write(&gcode->front(), gcode->size());

  } else {
    JSON::Writer writer(*stream, 0, exportDialog.compactJSONSelected());
    project->write(writer, true);
    writer.close();
  }
}


bool QtWin::runNewProjectDialog() {
  // Initialize dialog
  newProjectDialog.setUnits(getDefaultUnits());

  // Run dialog
  return newProjectDialog.exec() == QDialog::Accepted;
}


ToolTable QtWin::getNewToolTable() {
  if (newProjectDialog.defaultToolTableSelected())
    return loadDefaultToolTable();

  if (newProjectDialog.currentToolTableSelected())
    return project.isNull() ? ToolTable() : project->getToolTable();

  return ToolTable();
}


ToolUnits QtWin::getNewUnits() {return newProjectDialog.getUnits();}


string QtWin::openFile(const string &title, const string &filters,
                       const string &_filename, bool save) {
  string filename = _filename;
  if (filename.empty() && !project.isNull())
    filename = SystemUtilities::dirname(project->getFilename());

  return fileDialog.open(title, filters, filename, save);
}


void QtWin::loadProject() {
  toolsChanged();
  updateFiles();
  project->markClean();
}


void QtWin::resetProject() {
  view->clear();

  // Close editor tabs
  ui->fileTabManager->closeAll(false, true);
  ui->fileTabManager->setCurrentIndex(0);
}


void QtWin::newProject() {
  if (!checkSave()) return;

  LOG_INFO(1, "New project");

  if (!runNewProjectDialog()) return;

  // Save tool table before resetting project
  ToolTable toolTable = getNewToolTable();
  ToolUnits units = getNewUnits();

  // Create new project
  resetProject();
  project = new Project(options);
  project->setUnits(units);
  project->getToolTable() = toolTable;

  reload();
  loadProject();
}


static bool is_xml(const std::string &filename) {
  try {
    if (!SystemUtilities::exists(filename))
      return SystemUtilities::extension(filename) == "xml";

    SmartPointer<iostream> stream = SystemUtilities::open(filename, ios::in);

    while (true) {
      int c = stream->peek();
      if (c == '<') return true;
      else if (isspace(c)) stream->get(); // Next
      else return false; // Not XML
    }

  } CATCH_WARNING;

  return false;
}


void QtWin::openProject(const string &_filename) {
  if (!checkSave()) return;

  string filename = _filename;
  QSettings settings;
  QString lastDir = settings.value("Projects/lastDir", QDir::homePath()).toString();
  if (filename.empty()) {
    filename = QFileDialog::getOpenFileName(
          this,
          tr("Open File"),
          lastDir,
          tr("Supported Files (*.xml *.nc *.ngc "
             "*.gcode *.tap *.tpl);;All Files (*.*)")).toStdString();
    if (filename.empty()) return;
    settings.setValue("Projects/lastDir", QString::fromStdString(filename));
  }

  int size = settings.beginReadArray("recentProjects");
  QStringList recents;
  for (int i = 0; i < size; i++) {
    settings.setArrayIndex(i);
    QString recent = settings.value("fileName").toString();
    // skip the currently opened project from the recents
    // if it was already present in the recents projects list
    if (recent != QString::fromStdString(filename))
      recents.append(recent);
  }
  settings.endArray();

  // reload the actions in the recent menu too to move the opened file to the last position
  foreach (QAction *action, ui->menuRecent_projects->actions()) {
    ui->menuRecent_projects->removeAction(action);
    delete action;
  }

  // there is no way to remove an array item from QSettings
  // rewrite the whole array if the order changed
  recents.append(QString::fromStdString(filename));

  settings.beginWriteArray("recentProjects");
  int i = 0;
  int skipIndex = (recents.size() - maxRecentsSize);
  foreach (QString recent, recents.mid(skipIndex)) {
    QAction *action = ui->menuRecent_projects->addAction(recent, &recentProjectsMapper, SLOT(map()));
    recentProjectsMapper.setMapping(action, recent);

    settings.setArrayIndex(i);
    settings.setValue("fileName", recent);
    i++;
  }
  settings.endArray();

  showMessage("Opening " + filename);
  LOG_INFO(1, "Opening " << filename);

  try {
    // Check if the file appears to be XML
    bool xml = is_xml(filename);

    if (!xml) {
      // Check if .xml file exists
      string xmlPath = SystemUtilities::splitExt(filename)[0] + ".xml";
      if (SystemUtilities::exists(xmlPath) && is_xml(xmlPath)) {
        int response =
          QMessageBox::question
          (this, "Project File Exists", "An CAMotics project file for the "
           "selected file exists.  It may contain important project settings.  "
           "Would you like to open it instead?",
           QMessageBox::Cancel  | QMessageBox::No | QMessageBox::Yes,
           QMessageBox::Yes);

        if (response == QMessageBox::Cancel) return;
        if (response == QMessageBox::Yes) {
          xml = true;
          filename = xmlPath;
        }
      }
    }

    resetProject();

    if (xml) project = new Project(options, filename);
    else {
      // Assume TPL or G-Code and create a new project with the file

      if (!runNewProjectDialog()) return;

      // Save tool table before resetting project
      ToolTable toolTable = getNewToolTable();
      ToolUnits units = getNewUnits();

      project = new Project(options);
      project->addFile(filename);
      project->setUnits(units);
      project->getToolTable() = toolTable;
    }
  } CATCH_ERROR;

  view->path->setByRatio(1);
  reload();
  loadProject();
}


bool QtWin::saveProject(bool saveAs) {
  string filename = project->getFilename();

  if (saveAs || filename.empty()) {
    if (!filename.empty())
      filename = SystemUtilities::swapExtension(filename, "xml");

    filename = openFile("Save Project", "Projects (*.xml)", filename, true);
    if (filename.empty()) return false;

    string ext = SystemUtilities::extension(filename);
    if (ext.empty()) filename += ".xml";
    else if (ext != "xml") {
      warning("Project file must have .xml extension, not saved!");
      return false;
    }
  }

  try {
    project->save(filename);
    ui->fileTabManager->saveAll();
    showMessage("Saved " + filename);
    return true;
  } CATCH_ERROR;

  warning("Could not save project to " + filename);

  return false;
}


void QtWin::revertProject() {
  string filename = project->getFilename();

  if (filename.empty()) {
    warning("Cannot revert project.");
    return;
  }

  project->markClean();
  openProject(filename);
  ui->fileTabManager->revertAll();
}


bool QtWin::isMetric() const {
  return project.isNull() || project->isMetric();
}


ToolUnits QtWin::getDefaultUnits() const {
  return (ToolUnits::enum_t)QSettings().value("Settings/Units").toInt();
}


void QtWin::updateFiles() {
  QStringList list;

  for (unsigned i = 0; i < project->getFileCount(); i++)
    list.append(QString::fromUtf8
                (project->getFile(i)->getRelativePath().c_str()));

  ui->filesListView->setModel(new QStringListModel(list));
}


void QtWin::newFile(bool tpl) {
  string filename = project->getFilename();
  filename = SystemUtilities::swapExtension(filename, tpl ? "tpl" : "ngc");

  filename = openFile(tpl ? "New TPL file" : "New GCode file",
                      tpl ? "TPL (*.tpl);;All files (*.*)" :
                      "GCode (*.nc *.ngc *.gcode *.tap);;All files", filename,
                      false);
  if (filename.empty()) return;

  string ext = SystemUtilities::extension(filename);
  if (ext.empty()) filename += tpl ? ".tpl" : ".gcode";

  else if (tpl && ext != "tpl") {
    warning("TPL file must have .tpl extension");
    return;

  } else if (!tpl && (ext == "xml" || ext == "tpl")) {
    warning("GCode file cannot have .tpl or .xml extension");
    return;
  }

  project->addFile(filename);
  updateFiles();
}


void QtWin::addFile() {
  string filename =
    openFile("Add file", "Supported Files (*.nc *.ngc "
             "*.gcode *.tap *.tpl);;All Files (*.*)", "", false);
  if (filename.empty()) return;

  project->addFile(filename);
  updateFiles();
}


void QtWin::editFile(unsigned index) {
  SmartPointer<NCFile> file = project->getFile(index);
  if (!file.isNull()) ui->fileTabManager->open(file);
}


void QtWin::removeFile(unsigned index) {
  project->removeFile(index);
  updateFiles();
}


bool QtWin::checkSave(bool canCancel) {
  if (!ui->fileTabManager->checkSaveAll()) return false;
  if (project.isNull() || !project->isDirty()) return true;

  int response =
    QMessageBox::question(this, "Project Modified", "The current project has "
                          "been modifed.  Would you like to save it?",
                          (canCancel ?
                           QMessageBox::Cancel : QMessageBox::NoButton) |
                          QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);

  if (response == QMessageBox::Yes) return saveProject();
  else if (response != QMessageBox::No) return false;
  return true;
}


void QtWin::activateFile(const string &filename, int line, int col) {
  SmartPointer<NCFile> file = project->findFile(filename);
  if (!file.isNull()) ui->fileTabManager->open(file, line, col);
}


void QtWin::updateActions() {
  unsigned tab = ui->fileTabManager->currentIndex();
  bool fileTab = ui->fileTabManager->isFileTab(tab);

  if (!fileTab) {
    ui->actionSaveFile->setEnabled(false);
    ui->actionSaveFileAs->setEnabled(false);
    ui->actionRevertFile->setEnabled(false);

    ui->actionSaveFile->setText("Save File");
    ui->actionSaveFileAs->setText("Save File As");
    ui->actionRevertFile->setText("Revert File");

  } else {
    SmartPointer<NCFile> file = ui->fileTabManager->getFile(tab);
    string basename = SystemUtilities::basename(file->getAbsolutePath());
    QString title = QString::fromUtf8(basename.c_str());

    bool modified = ui->fileTabManager->isModified(tab);
    bool exists = file->exists();

    ui->actionSaveFile->setEnabled(modified);
    ui->actionSaveFileAs->setEnabled(true);
    ui->actionRevertFile->setEnabled(modified && exists);

    ui->actionSaveFile->setText(QString("Save \"%1\"").arg(title));
    ui->actionSaveFileAs->setText(QString("Save \"%1\" As").arg(title));
    ui->actionRevertFile->setText(QString("Revert \"%1\"").arg(title));
  }

  ui->actionUndo->setEnabled(fileTab);
  ui->actionRedo->setEnabled(fileTab);
  ui->actionCut->setEnabled(fileTab);
  ui->actionCopy->setEnabled(fileTab);
  ui->actionPaste->setEnabled(fileTab);
  ui->actionSelectAll->setEnabled(fileTab);
}


void QtWin::updateUnits() {
  loadWorkpiece();
  updateBounds();
  valueSet.updated();
}


void QtWin::updateToolTables() {
  ToolTable tools;
  if (!project.isNull()) tools = project->getToolTable();

  QStringList list;
  for (ToolTable::iterator it = tools.begin(); it != tools.end(); it++)
    list.append
      (QString().sprintf("%d: %s", it->first, it->second.getText().c_str()));

  ui->toolTableListView->setModel(new QStringListModel(list));
}


void QtWin::toolsChanged() {
  project->markDirty();
  updateToolTables();
}


void QtWin::editTool(unsigned number) {
  if (project.isNull()) return;

  ToolTable &tools = project->getToolTable();

  if (tools.has(number)) toolDialog.setTool(tools.get(number));
  else toolDialog.getTool().setNumber(number);

  if (toolDialog.edit() != QDialog::Accepted) return;

  Tool &tool = toolDialog.getTool();

  if (tool.getNumber() != number) {
    if (tools.has(tool.getNumber())) {
      int response = QMessageBox::question
        (this, "Overwrite Tool?",
         QString().sprintf("Tool %d already exists.  Do you want to overwrite "
                           "it?", tool.getNumber()),
         QMessageBox::No | QMessageBox::Yes, QMessageBox::No);

      if (response == QMessageBox::No) return;
    }

    tools.erase(number);
  }

  tools.set(tool);
  toolsChanged();
}


void QtWin::addTool() {
  if (project.isNull()) return;
  ToolTable &tools = project->getToolTable();

  for (unsigned i = 1; i < 1000; i++)
    if (!tools.has(i)) {
      editTool(i);
      return;
    }

  THROW("Too many tools");
}


void QtWin::removeTool(unsigned number) {
  project->getToolTable().erase(number);
  toolsChanged();
}


void QtWin::exportToolTable() {
  if (project.isNull()) return;
  const ToolTable &tools = project->getToolTable();

  string filename =
    SystemUtilities::dirname(project->getFilename()) + "/tools.json";

  filename =
    openFile("Export tool table", "Tool table files (*.json)", filename, true);

  if (filename.empty()) return;

  *SystemUtilities::oopen(filename) << tools;
}


void QtWin::importToolTable() {
  if (project.isNull()) return;

  string filename =
    openFile("Import tool table", "Tool table files (*.json)", "", false);

  if (filename.empty()) return;

  ToolTable tools;
  *SystemUtilities::iopen(filename) >> tools;

  if (tools.empty()) {
    warning("'" + filename + "' empty or not a tool table");
    return;
  }

  project->getToolTable() = tools;
  toolsChanged();
}


void QtWin::saveDefaultToolTable(const ToolTable &tools) {
  ostringstream str;
  str << tools << flush;

  QSettings settings;
  settings.setValue("ToolTable/Default", QString::fromUtf8(str.str().c_str()));

  showMessage("Default tool table saved");
}


ToolTable QtWin::loadDefaultToolTable() {
  QSettings settings;
  QByteArray data = settings.value("ToolTable/Default").toString().toUtf8();

  ToolTable tools;

  if (!data.isEmpty()) {
    istringstream str(data.data());
    str >> tools;
  }

  return tools;
}


void QtWin::updateWorkpiece() {
  view->setWorkpiece(project->getWorkpieceBounds());
  redraw();
}


void QtWin::loadWorkpiece() {
  if (project->getAutomaticWorkpiece()) on_automaticCuboidRadioButton_clicked();
  else on_manualCuboidRadioButton_clicked();

  LOCK_UI_UPDATES;
  ui->marginDoubleSpinBox->setValue(project->getWorkpieceMargin());

  // Bounds
  real scale = isMetric() ? 1 : 1 / 25.4;
  Rectangle3R bounds = project->getWorkpieceBounds();
  ui->xDimDoubleSpinBox->setValue(bounds.getDimensions().x() * scale);
  ui->yDimDoubleSpinBox->setValue(bounds.getDimensions().y() * scale);
  ui->zDimDoubleSpinBox->setValue(bounds.getDimensions().z() * scale);
  ui->xOffsetDoubleSpinBox->setValue(bounds.getMin().x() * scale);
  ui->yOffsetDoubleSpinBox->setValue(bounds.getMin().y() * scale);
  ui->zOffsetDoubleSpinBox->setValue(bounds.getMin().z() * scale);

  // Units
  const char *suffix = isMetric() ? "mm" : "in";
  ui->xDimDoubleSpinBox->setSuffix(suffix);
  ui->yDimDoubleSpinBox->setSuffix(suffix);
  ui->zDimDoubleSpinBox->setSuffix(suffix);
  ui->xOffsetDoubleSpinBox->setSuffix(suffix);
  ui->yOffsetDoubleSpinBox->setSuffix(suffix);
  ui->zOffsetDoubleSpinBox->setSuffix(suffix);

  // Update Workpiece steps
  real step = isMetric() ? 1 : 0.125;
  ui->xDimDoubleSpinBox->setSingleStep(step);
  ui->yDimDoubleSpinBox->setSingleStep(step);
  ui->zDimDoubleSpinBox->setSingleStep(step);
  ui->xOffsetDoubleSpinBox->setSingleStep(step);
  ui->yOffsetDoubleSpinBox->setSingleStep(step);
  ui->zOffsetDoubleSpinBox->setSingleStep(step);

  updateWorkpiece();
}


void QtWin::setWorkpieceDim(unsigned dim, real value) {
  real scale = isMetric() ? 1 : 25.4;
  Rectangle3R bounds = project->getWorkpieceBounds();
  bounds.rmax[dim] = bounds.rmin[dim] + value * scale;
  project->setWorkpieceBounds(bounds);

  updateWorkpiece();
  redraw(true);
}


void QtWin::setWorkpieceOffset(unsigned dim, real value) {
  real scale = isMetric() ? 1 : 25.4;
  Rectangle3R bounds = project->getWorkpieceBounds();
  bounds.rmax[dim] = bounds.getDimension(dim) + value * scale;
  bounds.rmin[dim] = value * scale;
  project->setWorkpieceBounds(bounds);

  updateWorkpiece();
  redraw(true);
}


void QtWin::updateBounds() {
  updateToolPathBounds();
  updateWorkpieceBounds();
}


void QtWin::updateToolPathBounds() {
  Rectangle3R bounds = *toolPath;
  Vector3R bMin = bounds.getMin();
  Vector3R bMax = bounds.getMax();
  Vector3R bDim = bounds.getDimensions();

  setUnitLabel(ui->toolPathBoundsXMinLabel, bMin.x());
  setUnitLabel(ui->toolPathBoundsXMaxLabel, bMax.x());
  setUnitLabel(ui->toolPathBoundsXDimLabel, bDim.x());

  setUnitLabel(ui->toolPathBoundsYMinLabel, bMin.y());
  setUnitLabel(ui->toolPathBoundsYMaxLabel, bMax.y());
  setUnitLabel(ui->toolPathBoundsYDimLabel, bDim.y());

  setUnitLabel(ui->toolPathBoundsZMinLabel, bMin.z());
  setUnitLabel(ui->toolPathBoundsZMaxLabel, bMax.z());
  setUnitLabel(ui->toolPathBoundsZDimLabel, bDim.z());
}


void QtWin::updateWorkpieceBounds() {
  if (project.isNull()) return;

  Rectangle3R bounds = project->getWorkpieceBounds();
  Vector3R bMin = bounds.getMin();
  Vector3R bMax = bounds.getMax();
  Vector3R bDim = bounds.getDimensions();

  setUnitLabel(ui->workpieceBoundsXMinLabel, bMin.x());
  setUnitLabel(ui->workpieceBoundsXMaxLabel, bMax.x());
  setUnitLabel(ui->workpieceBoundsXDimLabel, bDim.x());

  setUnitLabel(ui->workpieceBoundsYMinLabel, bMin.y());
  setUnitLabel(ui->workpieceBoundsYMaxLabel, bMax.y());
  setUnitLabel(ui->workpieceBoundsYDimLabel, bDim.y());

  setUnitLabel(ui->workpieceBoundsZMinLabel, bMin.z());
  setUnitLabel(ui->workpieceBoundsZMaxLabel, bMax.z());
  setUnitLabel(ui->workpieceBoundsZDimLabel, bDim.z());
}


void QtWin::setStatusActive(bool active) {
  if (active == lastStatusActive) return;
  lastStatusActive = active;

  if (active) {
    statusLabel->clear();
    QMovie *movie = new QMovie(":/icons/running.gif");
    statusLabel->setMovie(movie);
    movie->start();

    statusLabel->setToolTip("Simulation is running");
    ui->actionStop->setEnabled(true);

  } else {
    QMovie *movie = statusLabel->movie();
    if (movie) {
      statusLabel->clear();
      delete movie;
    }

    statusLabel->setPixmap(QPixmap(":/icons/idle.png"));
    statusLabel->setToolTip("Simulation has ended");
    ui->actionStop->setEnabled(false);
  }
}


void QtWin::showConsole() {
  ui->splitter->setSizes(QList<int>() << 5 << 1);
}


void QtWin::hideConsole() {
  ui->splitter->setSizes(QList<int>() << 1 << 0);
}


void QtWin::updatePlaySpeed(const string &name, unsigned value) {
  showMessage(String::printf("Playback speed %dx", view->getSpeed()));
}


void QtWin::updateViewFlags(const std::string &name, unsigned flags) {
  ui->actionPlay->setIcon(flags & View::PLAY_FLAG ? pauseIcon : playIcon);
  ui->actionPlay->setText(flags & View::PLAY_FLAG ? "Pause" : "Play");
}


void QtWin::updatePlayDirection(const string &name, bool reverse) {
  ui->actionDirection->setIcon(reverse ? backwardIcon : forwardIcon);
}


void QtWin::updateTimeRatio(const string &name, double ratio) {
  ui->positionSlider->setValue(10000 * ratio);
}


void QtWin::updateX(const string &name, real value) {
  setUnitLabel(ui->xLabel, value, isMetric() ? 4 : 5, true);
}


void QtWin::updateY(const string &name, real value) {
  setUnitLabel(ui->yLabel, value, isMetric() ? 4 : 5, true);
}


void QtWin::updateZ(const string &name, real value) {
  setUnitLabel(ui->zLabel, value, isMetric() ? 4 : 5, true);
}


void QtWin::updateCurrentTime(const string &name, double value) {
  QString text =
    QString::fromUtf8(TimeInterval(value, true).toString().c_str());
  ui->currentTimeLabel->setText(text);
}


void QtWin::updateCurrentDistance(const string &name, double value) {
  setUnitLabel(ui->currentDistanceLabel, value);
}


void QtWin::updateRemainingTime(const string &name, double value) {
  QString text =
    QString::fromUtf8(TimeInterval(value, true).toString().c_str());
  ui->remainingTimeLabel->setText(text);
}


void QtWin::updateRemainingDistance(const string &name, double value) {
  setUnitLabel(ui->remainingDistanceLabel, value);
}


void QtWin::updateTotalTime(const string &name, double value) {
  QString text =
    QString::fromUtf8(TimeInterval(value, true).toString().c_str());
  ui->totalTimeLabel->setText(text);
}


void QtWin::updateTotalDistance(const string &name, double value) {
  setUnitLabel(ui->totalDistanceLabel, value);
}


void QtWin::updatePercentTime(const string &name, double value) {
  ui->percentTimeLabel->setText(QString().sprintf("%.2f%%", value));
}


void QtWin::updatePercentDistance(const string &name, double value) {
  ui->percentDistanceLabel->setText(QString().sprintf("%.2f%%", value));
}


void QtWin::updateTool(const string &name, unsigned value) {
  ui->toolLabel->setText(QString().sprintf("%d", value));
}


void QtWin::updateFeed(const string &name, double value) {
  ui->feedLabel->setText(QString().sprintf("%.2f", value));
}


void QtWin::updateSpeed(const string &name, double value) {
  if (std::numeric_limits<real>::max() == abs(value) || Math::isinf(value) ||
      Math::isnan(value)) ui->speedLabel->setText("nan");
  else ui->speedLabel->setText(QString().sprintf("%.2f RPM", value));
}


void QtWin::updateDirection(const string &name, const char *value) {
  ui->directionLabel->setText(QString::fromUtf8(value));
}


void QtWin::updateProgramLine(const string &name, unsigned value) {
  ui->programLineLabel->setText(QString().sprintf("%d", value));
}

void QtWin::taskCompleted() {
  QCoreApplication::postEvent
    (this, new QEvent((QEvent::Type)taskCompleteEvent));
}


bool QtWin::event(QEvent *event) {
  if (event->type() != taskCompleteEvent) return QMainWindow::event(event);

  while (taskMan.hasMore()) {
    SmartPointer<Task> task = taskMan.remove();

    if (task.isInstance<ToolPathTask>())
      toolPathComplete(*task.cast<ToolPathTask>());
    else if (task.isInstance<SurfaceTask>())
      surfaceComplete(*task.cast<SurfaceTask>());
    else if (task.isInstance<ReduceTask>())
      reduceComplete(*task.cast<ReduceTask>());
    else if (task.isInstance<Opt>())
      optimizeComplete(*task.cast<Opt>());
  }

  return true;
}


void QtWin::closeEvent(QCloseEvent *event) {
  if (checkSave()) {
    quit();
    event->accept();
  } else event->ignore();
}


void QtWin::resizeEvent(QResizeEvent *event) {
  updateToolTables();
  QMainWindow::resizeEvent(event);
}


void QtWin::animate() {
  try {
    dirty = connectionManager->update() || dirty;
    dirty = view->update() || dirty;

    // Auto close after auto play
    if (!autoPlay && autoClose && !view->isFlagSet(View::PLAY_FLAG))
      app.requestExit();

    if (dirty) redraw(true);
    if (simDirty) reload(true);

    // Update progress
    if (!view->isFlagSet(View::PLAY_FLAG)) {
      double progress = taskMan.getProgress();
      string status = taskMan.getStatus();
      if (lastProgress != progress || lastStatus != status) {
        lastProgress = progress;
        lastStatus = status;

        if (progress) {
          double eta = taskMan.getETA();
          ui->progressBar->setValue(10000 * progress);
          string s = String::printf("%.2f%% ", progress * 100);
          if (eta) s += TimeInterval(eta).toString();
          ui->progressBar->setFormat(QString::fromUtf8(s.c_str()));

          showMessage("Progress: " + s);

        } else {
          ui->progressBar->setValue(0);
          ui->progressBar->setFormat(QString::fromUtf8(status.c_str()));
        }
      }
    }

    // Copy log
    ui->console->writeToConsole();

  } CBANG_CATCH_ERROR;

  bool modified = !project.isNull() && project->isDirty();
  if (isWindowModified() != modified) setWindowModified(modified);

  if (app.shouldQuit()) {
    checkSave(false);
    quit();
  }
}

void QtWin::openRecentProjectsSlot(const QString path) {
  openProject(path.toStdString());
}


void QtWin::on_fileTabManager_currentChanged(int index) {
  redraw();
  updateActions();
}


void QtWin::on_positionSlider_valueChanged(int position) {
  if (Math::isnan(view->path->getTimeRatio())) return;

  double ratio = position / 10000.0;

  view->path->setByRatio(ratio);
  redraw();

  if (sliderMoving) return;
  if (view->isFlagSet(View::PLAY_FLAG) && lastStatusActive) return;
  if (simRun.isNull()) {
    reload();
    return;
  }

  simRun->setEndTime(ratio * view->path->getTotalTime());

  setStatusActive(true);
  taskMan.addTask(new SurfaceTask(simRun));
}


void QtWin::on_positionSlider_sliderPressed() {
  sliderMoving = true;
  view->setFlag(View::TRANSLUCENT_SURFACE_FLAG);
}


void QtWin::on_positionSlider_sliderReleased() {
  sliderMoving = false;
  view->clearFlag(View::TRANSLUCENT_SURFACE_FLAG);
  on_positionSlider_valueChanged(ui->positionSlider->value());
}


void QtWin::on_filesListView_activated(const QModelIndex &index) {
  if (index.isValid()) editFile(index.row());
}


void QtWin::on_filesListView_customContextMenuRequested(QPoint point) {
  QModelIndex index = ui->filesListView->indexAt(point);
  bool valid = index.isValid();

  ui->actionEditFile->setEnabled(valid);
  ui->actionRemoveFile->setEnabled(valid);

  QMenu menu;

  menu.addAction(ui->actionAddFile);
  menu.addAction(ui->actionEditFile);
  menu.addAction(ui->actionReloadFile);
  menu.addAction(ui->actionRemoveFile);

  menu.exec(ui->filesListView->mapToGlobal(point));
}


void QtWin::on_toolTableListView_activated(const QModelIndex &index) {
  if (index.isValid())
    editTool(project->getToolTable().at(index.row()).getNumber());
}


void QtWin::on_toolTableListView_customContextMenuRequested(QPoint point) {
  QModelIndex index = ui->toolTableListView->indexAt(point);
  bool valid = index.isValid();

  ui->actionEditTool->setEnabled(valid);
  ui->actionRemoveTool->setEnabled(valid);

  QMenu menu;

  menu.addAction(ui->actionAddTool);
  menu.addAction(ui->actionEditTool);
  menu.addAction(ui->actionRemoveTool);
  menu.addSeparator();
  menu.addAction(ui->actionImportToolTable);
  menu.addAction(ui->actionExportToolTable);
  menu.addSeparator();
  menu.addAction(ui->actionLoadDefaultToolTable);
  menu.addAction(ui->actionSaveDefaultToolTable);

  menu.exec(ui->toolTableListView->mapToGlobal(point));
}


void QtWin::on_automaticCuboidRadioButton_clicked() {
  ui->automaticCuboidRadioButton->setChecked(true);
  ui->automaticCuboidFrame->setEnabled(true);
  ui->manualCuboidFrame->setEnabled(false);

  PROTECT_UI_UPDATE;

  if (project->getAutomaticWorkpiece()) return;

  project->setAutomaticWorkpiece(true);
  project->updateAutomaticWorkpiece(*toolPath);
  loadWorkpiece();

  redraw(true);
}


void QtWin::on_marginDoubleSpinBox_valueChanged(double value) {
  PROTECT_UI_UPDATE;

  project->setWorkpieceMargin(value);
  if (!toolPath.isNull()) project->updateAutomaticWorkpiece(*toolPath);
  loadWorkpiece();

  redraw(true);
}


void QtWin::on_manualCuboidRadioButton_clicked() {
  ui->manualCuboidRadioButton->setChecked(true);
  ui->automaticCuboidFrame->setEnabled(false);
  ui->manualCuboidFrame->setEnabled(true);

  PROTECT_UI_UPDATE;

  if (!project->getAutomaticWorkpiece()) return;

  project->setAutomaticWorkpiece(false);
  loadWorkpiece();

  redraw(true);
}


void QtWin::on_xDimDoubleSpinBox_valueChanged(double value) {
  PROTECT_UI_UPDATE;
  setWorkpieceDim(0, value);
}


void QtWin::on_yDimDoubleSpinBox_valueChanged(double value) {
  PROTECT_UI_UPDATE;
  setWorkpieceDim(1, value);
}


void QtWin::on_zDimDoubleSpinBox_valueChanged(double value) {
  PROTECT_UI_UPDATE;
  setWorkpieceDim(2, value);
}


void QtWin::on_xOffsetDoubleSpinBox_valueChanged(double value) {
  PROTECT_UI_UPDATE;
  setWorkpieceOffset(0, value);
}


void QtWin::on_yOffsetDoubleSpinBox_valueChanged(double value) {
  PROTECT_UI_UPDATE;
  setWorkpieceOffset(1, value);
}


void QtWin::on_zOffsetDoubleSpinBox_valueChanged(double value) {
  PROTECT_UI_UPDATE;
  setWorkpieceOffset(2, value);
}


void QtWin::on_actionQuit_triggered() {
  if (checkSave()) quit();
}


void QtWin::on_actionNew_triggered() {
  newProject();
}


void QtWin::on_actionOpen_triggered() {
  openProject();
}


void QtWin::on_actionStop_triggered() {
  stop();
}


void QtWin::on_actionRun_triggered() {
  ui->fileTabManager->checkSaveAll();
  reload(true);
}


void QtWin::on_actionReduce_triggered() {
  reduce();
}


void QtWin::on_actionOptimize_triggered() {
  optimize();
}


void QtWin::on_actionSlower_triggered() {
  view->decSpeed();
}


void QtWin::on_actionPlay_triggered() {
  view->toggleFlag(View::PLAY_FLAG);
}


void QtWin::on_actionFaster_triggered() {
  view->incSpeed();
}


void QtWin::on_actionDirection_triggered() {
  view->changeDirection();
}


void QtWin::on_actionExamples_triggered() {
  QAction *action = (QAction *)QObject::sender();
  if (action->toolTip().isEmpty()) return;
  openProject(action->toolTip().toUtf8().data());
}


void QtWin::on_actionSave_triggered() {
  saveProject();
}


void QtWin::on_actionSaveAs_triggered() {
  saveProject(true);
}


void QtWin::on_actionSaveFile_triggered() {
  ui->fileTabManager->save();
}


void QtWin::on_actionSaveFileAs_triggered() {
  ui->fileTabManager->saveAs();
  updateFiles();
}


void QtWin::on_actionRevertFile_triggered() {
  ui->fileTabManager->revert();
}


void QtWin::on_actionSaveDefaultToolTable_triggered() {
  if (!project.isNull()) saveDefaultToolTable(project->getToolTable());
}


void QtWin::on_actionLoadDefaultToolTable_triggered() {
  if (!project.isNull()) project->getToolTable() = loadDefaultToolTable();
}


void QtWin::on_actionSettings_triggered() {
  if (!project.isNull()) settingsDialog.exec(*project, *view);
  updateUnits();
}


void QtWin::on_actionFullscreen_triggered(bool checked) {
  if (checked) showFullScreen();
  else showNormal();
}


void QtWin::on_actionDefaultLayout_triggered() {
  defaultLayout();
}


void QtWin::on_actionFullLayout_triggered() {
  fullLayout();
}


void QtWin::on_actionMinimalLayout_triggered() {
  minimalLayout();
}


void QtWin::on_actionAbout_triggered() {
  aboutDialog.exec();
}


void QtWin::on_actionDonate_triggered() {
  donateDialog.exec();
}


void QtWin::on_actionHelp_triggered() {
  QMessageBox msg(this);
  msg.setWindowTitle("CAMotics Help");
  msg.setTextFormat(Qt::RichText);
  msg.setText("Help can be find in the online User's Manual at "
              "<a href='http://camotics.org/manual.html'"
              ">http://camotics.org/manual.html</a>.");
  msg.exec();
}


void QtWin::on_actionCutSurface_triggered() {
  PROTECT_UI_UPDATE;

  view->setFlag(View::WIRE_FLAG, false);
  view->setFlag(View::SHOW_SURFACE_FLAG, true);
  view->setFlag(View::SHOW_WORKPIECE_FLAG, false);

  ui->actionCutSurface->setChecked(true);
  ui->actionWorkpieceSurface->setChecked(false);
  ui->actionWireSurface->setChecked(false);
  ui->actionHideSurface->setChecked(false);

  redraw();
}


void QtWin::on_actionWorkpieceSurface_triggered() {
  PROTECT_UI_UPDATE;

  view->setFlag(View::WIRE_FLAG, false);
  view->setFlag(View::SHOW_SURFACE_FLAG, false);
  view->setFlag(View::SHOW_WORKPIECE_FLAG, true);

  ui->actionCutSurface->setChecked(false);
  ui->actionWorkpieceSurface->setChecked(true);
  ui->actionWireSurface->setChecked(false);
  ui->actionHideSurface->setChecked(false);

  redraw();
}


void QtWin::on_actionWireSurface_triggered() {
  PROTECT_UI_UPDATE;

  view->setFlag(View::WIRE_FLAG, true);
  view->setFlag(View::SHOW_SURFACE_FLAG, true);
  view->setFlag(View::SHOW_WORKPIECE_FLAG, false);

  ui->actionCutSurface->setChecked(false);
  ui->actionWorkpieceSurface->setChecked(false);
  ui->actionWireSurface->setChecked(true);
  ui->actionHideSurface->setChecked(false);

  redraw();
}


void QtWin::on_actionHideSurface_triggered() {
  PROTECT_UI_UPDATE;

  view->setFlag(View::SHOW_SURFACE_FLAG, false);
  view->setFlag(View::SHOW_WORKPIECE_FLAG, false);

  ui->actionCutSurface->setChecked(false);
  ui->actionWorkpieceSurface->setChecked(false);
  ui->actionWireSurface->setChecked(false);
  ui->actionHideSurface->setChecked(true);

  redraw();
}


void QtWin::on_actionAxes_triggered(bool checked) {
  view->setShowAxes(checked);
  redraw();
}


void QtWin::on_actionTool_triggered(bool checked) {
  view->setFlag(View::SHOW_TOOL_FLAG, checked);
  redraw();
}


void QtWin::on_actionWorkpieceBounds_triggered(bool checked) {
  view->setFlag(View::SHOW_WORKPIECE_BOUNDS_FLAG, checked);
  redraw();
}


void QtWin::on_actionToolPath_triggered(bool checked) {
  view->setFlag(View::SHOW_PATH_FLAG, checked);
  redraw();
}


void QtWin::on_actionAddFile_triggered() {
  if (newDialog.exec() != QDialog::Accepted) return;
  newFile(newDialog.tplSelected());
}

void QtWin::on_actionReloadFile_triggered()
{
  openProject(project->getFile(ui->filesListView->currentIndex().row())->getRelativePath());
}

void QtWin::on_actionEditFile_triggered() {
  editFile(ui->filesListView->currentIndex().row());
}


void QtWin::on_actionRemoveFile_triggered() {
  removeFile(ui->filesListView->currentIndex().row());
}


void QtWin::on_actionAddTool_triggered() {
  addTool();
}


void QtWin::on_actionEditTool_triggered() {
  int row = ui->toolTableListView->currentIndex().row();
  editTool(project->getToolTable().at(row).getNumber());
}


void QtWin::on_actionRemoveTool_triggered() {
  int row = ui->toolTableListView->currentIndex().row();
  removeTool(project->getToolTable().at(row).getNumber());
}


void QtWin::on_actionHideConsole_triggered() {
  QList<int> sizes = ui->splitter->sizes();
  sizes[0] = 1;
  sizes[1] = 0;
  ui->splitter->setSizes(sizes);
}


void QtWin::on_actionShowConsole_triggered() {
  QList<int> sizes = ui->splitter->sizes();
  sizes[0] = 1;
  sizes[1] = 1;
  ui->splitter->setSizes(sizes);
}


void QtWin::on_hideConsolePushButton_clicked() {
  on_actionHideConsole_triggered();
}


void QtWin::on_clearConsolePushButton_clicked() {
  ui->console->clear();
}

void QtWin::loadRecentProjects() {
  QSettings settings;
  int size = settings.beginReadArray("recentProjects");
  bool hasRemoved = false;
  QStringList recents;
  for (int i = 0; i < size; i++) {
    settings.setArrayIndex(i);
    QString recent = settings.value("fileName").toString();
    QFileInfo fi(recent);
    if (fi.exists(recent) && fi.isReadable()) {
      QAction *action = ui->menuRecent_projects->addAction(recent, &recentProjectsMapper, SLOT(map()));
      recentProjectsMapper.setMapping(action, recent);
      recents.append(recent);
    } else {
      // file removed/inaccessible -> set the flag
      hasRemoved = true;
    }
  }
  settings.endArray();

  // if any of the recent projects are inaccessible rewrite the whole list
  if (hasRemoved) {
    settings.beginWriteArray("recentProjects", recents.size());
    int i = 0;
    foreach (QString recent, recents) {
      settings.setArrayIndex(i);
      settings.setValue("fileName", recent);
      i++;
    }
    settings.endArray();
  }
  settings.sync();
}
