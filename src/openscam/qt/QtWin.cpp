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

#include "QtWin.h"

#include "ui_openscam.h"

#include "ProjectModel.h"
#include "FileTabManager.h"

#include <openscam/Geom.h>
#include <openscam/view/Viewer.h>
#include <openscam/cutsim/CutSim.h>
#include <openscam/cutsim/Project.h>
#include <openscam/cutsim/CutWorkpiece.h>
#include <openscam/remote/ConnectionManager.h>
#include <openscam/stl/STL.h>
#include <openscam/render/Renderer.h>

#include <cbang/Application.h>
#include <cbang/os/SystemUtilities.h>
#include <cbang/os/DirectoryWalker.h>
#include <cbang/log/Logger.h>
#include <cbang/util/SmartInc.h>
#include <cbang/util/DefaultCatch.h>
#include <cbang/time/TimeInterval.h>

#include <QSettings>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFileDialog>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QImageWriter>
#include <QMovie>
#include <QDesktopWidget>

#include <vector>

using namespace std;
using namespace cb;
using namespace OpenSCAM;

#define LOCK_UI_UPDATES SmartInc<unsigned> inc(inUIUpdate)
#define PROTECT_UI_UPDATE if (inUIUpdate) return; LOCK_UI_UPDATES


QtWin::QtWin(Application &app) :
  QMainWindow(0), toolPathCompleteEvent(0), surfaceCompleteEvent(0),
  ui(new Ui::OpenSCAMWindow), fileDialog(*this), app(app),
  options(app.getOptions()), cutSim(new CutSim(options)),
  connectionManager(new ConnectionManager(options)),
  view(new View(valueSet)), viewer(new Viewer), toolView(new ToolView),
  dirty(false), simDirty(false), inUIUpdate(false), lastProgress(0),
  lastStatusActive(false), smooth(true), autoPlay(false),
  currentUIView(NULL_VIEW) {

  ui->setupUi(this);
  ui->simulationView->init(SIMULATION_VIEW, this);

  // FileTabManager
  fileTabManager = new FileTabManager(*this, *ui->tabWidget, 2);

  connect(ui->actionUndo, SIGNAL(triggered()),
          fileTabManager.get(), SLOT(on_actionUndo_triggered()));
  connect(ui->actionRedo, SIGNAL(triggered()),
          fileTabManager.get(), SLOT(on_actionRedo_triggered()));
  connect(ui->actionCut, SIGNAL(triggered()),
          fileTabManager.get(), SLOT(on_actionCut_triggered()));
  connect(ui->actionCopy, SIGNAL(triggered()),
          fileTabManager.get(), SLOT(on_actionCopy_triggered()));
  connect(ui->actionPaste, SIGNAL(triggered()),
          fileTabManager.get(), SLOT(on_actionPaste_triggered()));
  connect(ui->actionSelectAll, SIGNAL(triggered()),
          fileTabManager.get(), SLOT(on_actionSelectAll_triggered()));

  // Register user events
  toolPathCompleteEvent = QEvent::registerEventType();
  surfaceCompleteEvent = QEvent::registerEventType();

  // Disable view bounds
  view->setShowBounds(false);

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

  // Setup console color
  ui->console->setTextColor(QColor("#d9d9d9"));

  // Setup console stream
  consoleStream = new LineBufferStream<LineBuffer>(lineBuffer);
  Logger::instance().setScreenStream(*consoleStream);
}


QtWin::~QtWin() {
  saveAllState();
}


void QtWin::init() {
  // Start animation timer
  animationTimer.setSingleShot(false);
  connect(&animationTimer, SIGNAL(timeout()), this, SLOT(animate()));
  animationTimer.start(100);

  // Simulation and Tool View tabs are not closeable
  ui->tabWidget->setTabsClosable(true);
  QTabBar *tabBar = ui->tabWidget->findChild<QTabBar *>();
  for (int i = 0; i < tabBar->count(); i++) {
    tabBar->setTabButton(i, QTabBar::RightSide, 0);
    tabBar->setTabButton(i, QTabBar::LeftSide, 0);
  }

  // Hide console by default
  hideConsole();

  // Init GUI
  setUIView(SIMULATION_VIEW);
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


void QtWin::setUnitLabel(QLabel *label, real value, int precision) {
  if (std::numeric_limits<real>::max() == abs(value) || Math::isinf(value) ||
      Math::isnan(value)) {
    label->setText("nan");
    return;
  }

  ToolUnits units = ToolUnits::UNITS_MM;
  if (!project.isNull()) units = project->getUnits();
  real scale = units == ToolUnits::UNITS_MM ? 1.0 : 1.0 / 25.4;
  label->setText(QString().sprintf("%.*f", precision, value * scale));
}


void QtWin::loadExamples() {
  try {
    const char *paths[] = {
      "/usr/share/doc/openscam/examples",
      "../SharedSupport/examples",
      "examples",
      0
    };

    string root = ".";
    string appPath =
      QCoreApplication::applicationFilePath().toAscii().data();
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

          QAction *action = menu->addAction(name.c_str());
          action->setToolTip(path.c_str());
          connect(action, SIGNAL(triggered()), this,
                  SLOT(on_actionExamples_triggered()));
        }

        break;
      }
    }

  } CATCH_ERROR;
}


void QtWin::saveAllState() {
  QSettings settings;
  settings.setValue("MainWindow/State", saveState());
  settings.setValue("MainWindow/Geometry", saveGeometry());
  settings.setValue("Console/Splitter", ui->splitter->saveState());
}


void QtWin::restoreAllState() {
  QSettings settings;
  restoreState(settings.value("MainWindow/State").toByteArray());
  restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
  ui->splitter->restoreState(settings.value("Console/Splitter").toByteArray());
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


SmartPointer<ViewPort> QtWin::getCurrentViewPort() const {
  switch (currentUIView) {
  case SIMULATION_VIEW: return view;
  default: return 0;
  }
}


void QtWin::snapView(char v) {
  SmartPointer<ViewPort> viewPort = getCurrentViewPort();
  if (viewPort.isNull()) return;

  viewPort->resetView(v);

  redraw();
}


void QtWin::glViewMousePressEvent(unsigned id, QMouseEvent *event) {
  SmartPointer<ViewPort> viewPort = getCurrentViewPort();
  if (viewPort.isNull()) return;

  if (event->buttons() & Qt::LeftButton)
    viewPort->startRotation(event->x(), event->y());

  else if (event->buttons() & (Qt::RightButton | Qt::MidButton))
    viewPort->startTranslation(event->x(), event->y());
}


void QtWin::glViewMouseMoveEvent(unsigned id, QMouseEvent *event) {
  SmartPointer<ViewPort> viewPort = getCurrentViewPort();
  if (viewPort.isNull()) return;

  if (event->buttons() & Qt::LeftButton) {
    viewPort->updateRotation(event->x(), event->y());
    redraw(true);

  } else if (event->buttons() & (Qt::RightButton | Qt::MidButton)) {
    viewPort->updateTranslation(event->x(), event->y());
    redraw(true);
  }
}


void QtWin::glViewWheelEvent(unsigned id, QWheelEvent *event) {
  SmartPointer<ViewPort> viewPort = getCurrentViewPort();
  if (viewPort.isNull()) return;

  if (event->delta() < 0) viewPort->zoomIn();
  else viewPort->zoomOut();

  redraw(true);
}


void QtWin::initializeGL(unsigned id) {
  LOG_DEBUG(5, "initializeGL(" << id << ")");

  GLenum err = glewInit();
  if (err != GLEW_OK) THROWS("Initializing GLEW: " << glewGetErrorString(err));

  switch (id) {
  case SIMULATION_VIEW:
    view->glInit();
    viewer->init();
    break;
  }
}


void QtWin::resizeGL(unsigned id, int w, int h) {
  LOG_DEBUG(5, "resizeGL(" << id << ", " << w << ", " << h << ")");

  switch (id) {
  case SIMULATION_VIEW: view->resize(w, h); break;
  }
}


void QtWin::paintGL(unsigned id) {
  LOG_DEBUG(5, "paintGL(" << id << ")");

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  switch (id) {
  case SIMULATION_VIEW: viewer->draw(*view); break;
  }
}


void QtWin::showMessage(const string &msg, double timeout) {
  ui->statusbar->showMessage(QString::fromAscii(msg.c_str()), timeout * 1000);
}


void QtWin::message(const string &msg) {
  QMessageBox::information(this, "OpenSCAM", msg.c_str(), QMessageBox::Ok);
}


void QtWin::warning(const string &msg) {
  QMessageBox::warning(this, "OpenSCAM", msg.c_str(), QMessageBox::Ok);
}


void QtWin::toolPathComplete() {
  toolPath = toolPathThread->getPath();

  // Update changed Project settings
  project->updateAutomaticWorkpiece(*toolPath);
  project->updateResolution();

  // Setup view
  view->setToolPath(toolPath);
  view->setWorkpiece(project->getWorkpieceBounds());

  // Load resolution settings
  LOCK_UI_UPDATES;
  ui->resolutionDoubleSpinBox->setValue(project->getResolution());
  ui->resolutionComboBox->setCurrentIndex(project->getResolutionMode());

  // Units
  ui->unitsComboBox->setCurrentIndex(project->getUnits());

  // Update UI
  loadWorkpiece();
  updateBounds();

  redraw();

  // Start surface thread
  surfaceThread = new SurfaceThread(surfaceCompleteEvent, this, cutSim,
                                    toolPath, project->getWorkpieceBounds(),
                                    project->getResolution(), view->getTime(),
                                    smooth);
  surfaceThread->start();

  // Auto play
  if (autoPlay) {
    autoPlay = false;
    view->path->setByRatio(0);
    view->setFlag(View::PLAY_FLAG);
    view->reverse = false;
  }
}


void QtWin::surfaceComplete() {
  if (!surfaceThread->getSurface().isNull()) {
    surface = surfaceThread->getSurface();
    view->setSurface(surface);
    redraw();
  }

  setStatusActive(false);
}


void QtWin::stop() {
  if (!toolPathThread.isNull()) toolPathThread->join();
  if (!surfaceThread.isNull()) surfaceThread->join();
  setStatusActive(false);
}


void QtWin::reload(bool now) {
  if (!now) {
    simDirty = true;
    return;
  }
  simDirty = false;

  try {
    stop();

    // Start tool path thread
    toolPathThread =
      new ToolPathThread(toolPathCompleteEvent, this, cutSim, project);
    toolPathThread->start();

    setStatusActive(true);

  } CBANG_CATCH_ERROR;
}


void QtWin::redraw(bool now) {
  if (now) {
    switch (currentUIView) {
    case SIMULATION_VIEW:
      updateWorkpieceBounds();
      ui->simulationView->updateGL();
      break;
    case TOOL_VIEW: updateToolUI(); break;
    default: break;
    }

    dirty = false;

  } else dirty = true;
}


void QtWin::snapshot() {
  string filename;
  QPixmap pixmap;

  switch (currentUIView) {
  case SIMULATION_VIEW:
    filename = SystemUtilities::basename(project->getFilename());
    filename = SystemUtilities::splitExt(filename)[0];
    pixmap = QPixmap::fromImage(ui->simulationView->grabFrameBuffer(true));
    break;

  case TOOL_VIEW:
    if (currentTool.isNull()) return;
    filename = String::printf("tool%d", currentTool->getNumber());
    pixmap = QPixmap::grabWidget(ui->toolView);
    break;

  default: return;
  }

  QList<QByteArray> formats = QImageWriter::supportedImageFormats();
  string fileTypes = "Image files (";
  for (int i = 0; i < formats.size(); i++) {
    if (i) fileTypes += ",";
    fileTypes += "*." + String::toLower(formats.at(i).data());
  }
  fileTypes += ")";

  filename += ".png";
  filename = openFile("Save snapshot", fileTypes, filename, true);
  if (filename.empty()) return;

  if (!pixmap.save(filename.c_str())) warning("Failed to save snapshot.");
  else showMessage("Snapshot saved.");
}


void QtWin::exportData() {
  // Check what we have to export

  if (surface.isNull() && toolPath.isNull()) {
    warning("Nothing to export.\nAdd a tool path or run a simulation.");
    return;
  }

  exportDialog.enableSurface(!surface.isNull());
  exportDialog.enableToolPath(!toolPath.isNull());

  // Run dialog
  if (exportDialog.exec() != QDialog::Accepted) return;

  // Select output file
  bool exportSurface  = exportDialog.surfaceSelected() && !surface.isNull();

  string title = string("Export ") + (exportSurface ? "Surface" : "Tool Path");
  string fileTypes =
    SSTR((exportSurface ? "STL" : "JSON") << " Files ("
         << (exportSurface ? "*.stl" : "*.json") << ");;All Files (*.*)");

  string filename = SystemUtilities::basename(project->getFilename());
  vector<string> parts = SystemUtilities::splitExt(filename);
  filename = parts[0] + (exportSurface ? ".stl" : ".json");

  filename = openFile(title, fileTypes, filename, true);

  if (filename.empty()) return;
  SmartPointer<iostream> stream = SystemUtilities::open(filename, ios::out);

  // Export
  if (exportSurface) {
    STL stl("OpenSCAM Surface");
    surface->exportSTL(stl);
    stl.setBinary(exportDialog.binarySTLSelected());
    stl.write(*stream);

  } else {
    JSON::Writer writer(*stream, 0, exportDialog.compactJSONSelected());
    toolPath->exportJSON(writer);
    writer.close();
  }
}


string QtWin::openFile(const string &title, const string &filters,
                       const string &filename, bool save) {
  return fileDialog.open(title, filters, filename, save);
}


void QtWin::loadProject() {
  if (projectModel.isNull()) {
    projectModel = new ProjectModel(project, this);
    ui->projectTreeView->setModel(projectModel.get());

  } else projectModel->setProject(project);

  ui->projectTreeView->expandAll();
  project->markClean();
}


void QtWin::resetProject() {
  view->setToolPath(0);
  view->setWorkpiece(Rectangle3R());
  view->setSurface(0);
  currentTool.release();
  view->resetView();

  // Close editor tabs
  fileTabManager->closeAll(false, true);
  ui->tabWidget->setCurrentIndex(0);
}


void QtWin::newProject() {
  if (!checkSave()) return;

  resetProject();
  project = new Project(options);

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

  if (filename.empty()) {
    filename = openFile("Open File", "Supported Files (*.xml *.nc *.ngc "
                        "*.gcode *.tpl);;All Files (*.*)", "", false);
    if (filename.empty()) return;
  }

  showMessage("Opening " + filename);

  try {
    // Check if the file appears to be XML
    bool xml = is_xml(filename);

    if (!xml) {
      // Check if .xml file exists
      string xmlPath = SystemUtilities::splitExt(filename)[0] + ".xml";
      if (SystemUtilities::exists(xmlPath) && is_xml(xmlPath)) {
        int response =
          QMessageBox::question
          (this, "Project File Exists", "An OpenSCAM project file for the "
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
      project = new Project(options);
      project->addFile(filename);
    }
  } CATCH_ERROR;

  reload();
  view->path->setByRatio(1);
  loadProject();
}


bool QtWin::saveProject(bool saveAs) {
  string filename = project->getFilename();

  if (saveAs || filename.empty()) {
    if (!filename.empty()) {
      string ext = SystemUtilities::extension(filename);

      if (ext.empty()) filename += ".xml";
      else if (ext != "xml")
        filename = filename.substr(0, filename.length() - ext.length()) + "xml";
    }

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
    fileTabManager->saveAll();
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
  fileTabManager->revertAll();
}


void QtWin::newFile(bool tpl) {
  string filename =
    openFile(tpl ? "New TPL file" : "New GCode file",
             tpl ? "TPL (*.tpl);;All files (*.*)" :
             "GCode (*.nc *.ngc *.gcode);;All files", "", false);
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
  projectModel->invalidate();
  reload();
}


void QtWin::addFile() {
  string filename = openFile("Add file", "Supported Files (*.nc *.ngc "
                             "*.gcode *.tpl);;All Files (*.*)", "", false);
  if (filename.empty()) return;

  project->addFile(filename);
  projectModel->invalidate();
  reload();
}


void QtWin::removeFile() {
  QModelIndex index = ui->projectTreeView->currentIndex();
  if (!index.isValid() ||
      projectModel->getType(index) != ProjectModel::FILE_ITEM) return;

  project->removeFile(projectModel->getOffset(index));
  projectModel->invalidate();
  reload();
}


bool QtWin::checkSave(bool canCancel) {
  for (int tab = 2; tab < ui->tabWidget->count(); tab++)
    if (!fileTabManager->checkSave(tab)) return false;

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


void QtWin::updateActions() {
  unsigned tab = ui->tabWidget->currentIndex();
  bool fileTab = 2 <= tab;

  if (!fileTab) {
    ui->actionSaveFile->setEnabled(false);
    ui->actionSaveFileAs->setEnabled(false);
    ui->actionRevertFile->setEnabled(false);

    ui->actionSaveFile->setText("Save File");
    ui->actionSaveFileAs->setText("Save File As");
    ui->actionRevertFile->setText("Revert File");

  } else {
    SmartPointer<NCFile> file = fileTabManager->getFile(tab);
    string basename = SystemUtilities::basename(file->getAbsolutePath());
    QString title = QString(basename.c_str());

    bool modified = fileTabManager->isModified(tab);
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


void QtWin::loadTool(unsigned number) {
  if (project.isNull()) return;

  if (!number) {
    // Find a tool which is not the current tool
    ToolTable &tools = *project->getToolTable();
    for (ToolTable::iterator it = tools.begin(); it != tools.end(); it++)
      if (it->first &&
          (currentTool.isNull() || it->first != currentTool->getNumber())) {
        number = it->first;
        break;
      }

    if (!number) {
      currentTool.release();
      return;
    }
  }

  currentTool = project->getToolTable()->get(number);

  real scale =
    currentTool->getUnits() == ToolUnits::UNITS_MM ? 1.0 : 1.0 / 25.4;

  {
    LOCK_UI_UPDATES;
    ui->toolSpinBox->setValue(number);
    ui->toolUnitsComboBox->setCurrentIndex(currentTool->getUnits());
    ui->shapeComboBox->setCurrentIndex(currentTool->getShape());
    ui->lengthDoubleSpinBox->setValue(currentTool->getLength() * scale);
    ui->diameterDoubleSpinBox->setValue(currentTool->getDiameter() * scale);
    ui->snubDiameterDoubleSpinBox->
      setValue(currentTool->getSnubDiameter() * scale);
    ui->descriptionLineEdit->
      setText(QString::fromAscii(currentTool->getDescription().c_str()));
  }

  on_shapeComboBox_currentIndexChanged(currentTool->getShape());

  updateToolUI();
}


void QtWin::addTool() {
  if (project.isNull()) return;
  ToolTable &tools = *project->getToolTable();

  for (unsigned i = 1; i < 1000; i++)
    if (!tools.has(i)) {
      tools.add(new Tool(i, 0, project->getUnits()));
      loadTool(i);
      setUIView(TOOL_VIEW);
      project->markDirty();
      projectModel->invalidate();
      return;
    }

  THROW("Too many tools");
}


void QtWin::removeTool() {
  project->getToolTable()->erase(currentTool->getNumber());
  project->markDirty();
  projectModel->invalidate();
  loadTool(0);
}


void QtWin::updateToolUI() {
  if (currentTool.isNull()) return;

  // Get widgets
  QGraphicsView &view = *ui->toolView;
  QGraphicsScene &scene = toolScene;
  view.setScene(&scene);

  // Get dimensions
  int width = view.frameSize().width();
  int height = view.frameSize().height();

  // Set dimensions
  scene.clear();
  scene.setSceneRect(0, 0, width, height);

  // Update tool view
  toolView->setTool(currentTool);
  toolView->resize(width, height);
  toolView->draw();

  // Paint image
  int stride = toolView->getStride();
  unsigned char *data = toolView->getBuffer().get();
  QImage image(data, width, height, stride, QImage::Format_ARGB32);
  QGraphicsPixmapItem *item =
    new QGraphicsPixmapItem(QPixmap::fromImage(image));

  scene.addItem(item);
  scene.update();

  // Select correct tool line
  unsigned number = currentTool->getNumber();
  ui->projectTreeView->setCurrentIndex(projectModel->getToolIndex(number));
}


void QtWin::loadWorkpiece() {
  if (project->getAutomaticWorkpiece()) on_automaticCuboidRadioButton_clicked();
  else on_manualCuboidRadioButton_clicked();

  LOCK_UI_UPDATES;
  ui->marginDoubleSpinBox->setValue(project->getWorkpieceMargin());

  ToolUnits units = project->getUnits();

  // Bounds
  real scale = units == ToolUnits::UNITS_MM ? 1 : 1 / 25.4;
  Rectangle3R bounds = project->getWorkpieceBounds();
  ui->xDimDoubleSpinBox->setValue(bounds.getDimensions().x() * scale);
  ui->yDimDoubleSpinBox->setValue(bounds.getDimensions().y() * scale);
  ui->zDimDoubleSpinBox->setValue(bounds.getDimensions().z() * scale);
  ui->xOffsetDoubleSpinBox->setValue(bounds.getMin().x() * scale);
  ui->yOffsetDoubleSpinBox->setValue(bounds.getMin().y() * scale);
  ui->zOffsetDoubleSpinBox->setValue(bounds.getMin().z() * scale);

  // Update Workpiece steps
  real step = units == ToolUnits::UNITS_MM ? 1 : 0.125;
  ui->xDimDoubleSpinBox->setSingleStep(step);
  ui->yDimDoubleSpinBox->setSingleStep(step);
  ui->zDimDoubleSpinBox->setSingleStep(step);
  ui->xOffsetDoubleSpinBox->setSingleStep(step);
  ui->yOffsetDoubleSpinBox->setSingleStep(step);
  ui->zOffsetDoubleSpinBox->setSingleStep(step);

  // Update visual
  view->setWorkpiece(bounds);
  redraw();
}


void QtWin::setWorkpieceDim(unsigned dim, real value) {
  real scale = project->getUnits() == ToolUnits::UNITS_MM ? 1 : 25.4;
  Rectangle3R bounds = project->getWorkpieceBounds();
  bounds.rmax[dim] = bounds.rmin[dim] + value * scale;
  project->setWorkpieceBounds(bounds);
  loadWorkpiece();

  redraw(true);
}


void QtWin::setWorkpieceOffset(unsigned dim, real value) {
  real scale = project->getUnits() == ToolUnits::UNITS_MM ? 1 : 25.4;
  Rectangle3R bounds = project->getWorkpieceBounds();
  bounds.rmax[dim] = bounds.getDimension(dim) + value * scale;
  bounds.rmin[dim] = value * scale;
  project->setWorkpieceBounds(bounds);
  loadWorkpiece();

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


void QtWin::appendConsole(const string &_line) {
  cout << _line << endl;

  string line = _line;
  QColor saveColor = ui->console->textColor();

  if (4 < line.size() && line[0] == 27 && line[1] == '[' &&
      line[4] == 'm') {

    int code = String::parseU8(line.substr(2, 2));
    QColor color;

    switch (code) {
    case 30: color = QColor("#000000"); break;
    case 31: color = QColor("#ff0000"); break;
    case 32: color = QColor("#00ff00"); break;
    case 33: color = QColor("#ffff00"); break;
    case 34: color = QColor("#0000ff"); break;
    case 35: color = QColor("#ff00ff"); break;
    case 36: color = QColor("#00ffff"); break;
    case 37: color = QColor("#ffffff"); break;

    case 90: color = QColor("#555555"); break;
    case 91: color = QColor("#ff5555"); break;
    case 92: color = QColor("#55ff55"); break;
    case 93: color = QColor("#ffff55"); break;
    case 94: color = QColor("#5555ff"); break;
    case 95: color = QColor("#ff55ff"); break;
    case 96: color = QColor("#55ffff"); break;
    case 97: color = QColor("#ffffff"); break;
    }

    line = line.substr(5);
    ui->console->setTextColor(color);
  }

  if (String::endsWith(line, "\033[0m"))
    line = line.substr(0, line.size() - 4);

  ui->console->append(QByteArray(line.c_str()));
  ui->console->setTextColor(saveColor);
}


void QtWin::setUIView(ui_view_t uiView) {
  if (uiView == currentUIView) return;
  currentUIView = uiView;

  // Select view widgets
  switch (uiView) {
  case SIMULATION_VIEW:
    ui->tabWidget->setCurrentIndex(0);
    ui->settingsStack->setCurrentWidget(ui->simulationProperties);
    break;

  case TOOL_VIEW:
    if (currentTool.isNull()) loadTool(0);
    ui->tabWidget->setCurrentIndex(1);
    ui->settingsStack->setCurrentWidget(ui->toolProperties);
    break;

  default: break;
  }

  redraw(true);
}


void QtWin::updatePlaySpeed(const string &name, unsigned value) {
  // TODO
  //ui->playbackSpeedLabel->setText(QString().sprintf("%dx", view->speed));
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
  setUnitLabel(ui->xLabel, value, 6);
}


void QtWin::updateY(const string &name, real value) {
  setUnitLabel(ui->yLabel, value, 6);
}


void QtWin::updateZ(const string &name, real value) {
  setUnitLabel(ui->zLabel, value, 6);
}


void QtWin::updateCurrentTime(const string &name, double value) {
  QString text =
    QString::fromAscii(TimeInterval(value, true).toString().c_str());
  ui->currentTimeLabel->setText(text);
}


void QtWin::updateCurrentDistance(const string &name, double value) {
  setUnitLabel(ui->currentDistanceLabel, value);
}


void QtWin::updateRemainingTime(const string &name, double value) {
  QString text =
    QString::fromAscii(TimeInterval(value, true).toString().c_str());
  ui->remainingTimeLabel->setText(text);
}


void QtWin::updateRemainingDistance(const string &name, double value) {
  setUnitLabel(ui->remainingDistanceLabel, value);
}


void QtWin::updateTotalTime(const string &name, double value) {
  QString text =
    QString::fromAscii(TimeInterval(value, true).toString().c_str());
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


bool QtWin::event(QEvent *event) {
  if (toolPathCompleteEvent && event->type() == toolPathCompleteEvent)
    toolPathComplete();

  else if (surfaceCompleteEvent && event->type() == surfaceCompleteEvent)
    surfaceComplete();

  else return QMainWindow::event(event);

  return true;
}


void QtWin::closeEvent(QCloseEvent *event) {
  if (checkSave()) event->accept();
  else event->ignore();
}


void QtWin::animate() {
  try {
    dirty = connectionManager->update() || dirty;
    dirty = view->update() || dirty;

    if (dirty) redraw(true);
    if (simDirty) reload(true);

    // Update progress
    double progress = cutSim->getProgress();
    string status = cutSim->getStatus();
    if (lastProgress != progress || lastStatus != status) {
      lastProgress = progress;
      lastStatus = status;

      if (progress) {
        double eta = cutSim->getETA();
        ui->progressBar->setValue(10000 * progress);
        string s = String::printf("%.2f%% ", progress * 100);
        if (eta) s += TimeInterval(eta).toString();
        ui->progressBar->setFormat(s.c_str());

        showMessage("Simulation progress: " + s);

      } else {
        ui->progressBar->setValue(0);
        ui->progressBar->setFormat(status.c_str());
      }
    }

    // Copy log
    while (lineBuffer.hasLine()) appendConsole(lineBuffer.getLine());

  } CBANG_CATCH_ERROR;

  bool modified = !project.isNull() && project->isDirty();
  if (isWindowModified() != modified) setWindowModified(modified);

  if (app.shouldQuit()) {
    stop();
    checkSave(false);
    QCoreApplication::exit();
  }
}


void QtWin::on_tabWidget_currentChanged(int index) {
  switch (index) {
  case 0: setUIView(SIMULATION_VIEW); break;
  case 1: setUIView(TOOL_VIEW); break;
  default: setUIView(FILE_VIEW); break;
  }

  updateActions();
}


void QtWin::on_tabWidget_tabCloseRequested(int index) {
  fileTabManager->close(index, true, false);
}


void QtWin::on_resolutionComboBox_currentIndexChanged(int index) {
  PROTECT_UI_UPDATE;
  project->setResolutionMode((ResolutionMode::enum_t)index);
  ui->resolutionDoubleSpinBox->setValue(project->getResolution());
}


void QtWin::on_resolutionDoubleSpinBox_valueChanged(double value) {
  PROTECT_UI_UPDATE;
  project->setResolution(value);
  project->setResolutionMode(ResolutionMode::RESOLUTION_MANUAL);
  ui->resolutionComboBox->setCurrentIndex(ResolutionMode::RESOLUTION_MANUAL);
}


void QtWin::on_unitsComboBox_currentIndexChanged(int value) {
  ToolUnits units = (ToolUnits::enum_t)value;
  if (project.isNull() || project->getUnits() == units) return;
  project->setUnits(units);
  updateUnits();
}


void QtWin::on_smoothPushButton_clicked(bool active) {
  smooth = active;
}


void QtWin::on_positionSlider_sliderMoved(int position) {
  view->path->setByRatio((double)position / 10000);
  redraw();
}


void QtWin::on_projectTreeView_activated(const QModelIndex &index) {
  switch (projectModel->getType(index)) {
  case ProjectModel::FILE_ITEM:
    fileTabManager->open(project->getFile(projectModel->getOffset(index)));
    break;

  case ProjectModel::PATHS_ITEM:
  case ProjectModel::PROJECT_ITEM:
  case ProjectModel::WORKPIECE_ITEM:
    setUIView(SIMULATION_VIEW);
    break;

  case ProjectModel::TOOL_ITEM: {
    setUIView(TOOL_VIEW);
    Tool &tool = projectModel->getTool(index);
    loadTool(tool.getNumber());
    break;
  }

  case ProjectModel::TOOLS_ITEM:
    setUIView(TOOL_VIEW);
    break;

  default: break;
  }

  redraw(true);
}


void QtWin::on_projectTreeView_customContextMenuRequested(QPoint point) {
  QModelIndex index = ui->projectTreeView->indexAt(point);

  ProjectModel::item_t type = ProjectModel::NULL_ITEM;
  if (index.isValid()) type = projectModel->getType(index);

  ui->actionRemoveFile->setEnabled(type == ProjectModel::FILE_ITEM);
  ui->actionRemoveTool->setEnabled(type == ProjectModel::TOOL_ITEM);

  switch (type) {
  case ProjectModel::FILE_ITEM: break;
  case ProjectModel::TOOL_ITEM: {
    Tool &tool = projectModel->getTool(projectModel->getOffset(index));
    loadTool(tool.getNumber());
    break;
  }

  default: break;
  }

  QMenu menu;
  menu.addAction(ui->actionAddFile);
  menu.addAction(ui->actionRemoveFile);
  menu.addSeparator();
  menu.addAction(ui->actionAddTool);
  menu.addAction(ui->actionRemoveTool);
  menu.exec(ui->projectTreeView->mapToGlobal(point));
}


void QtWin::on_toolSpinBox_valueChanged(int value) {
  int current = currentTool->getNumber();
  int newNum = value;

  if (newNum == current) return;

  ToolTable &tools = *project->getToolTable();

  while (newNum && newNum < current && tools.has(newNum)) newNum--;
  while (current < newNum && tools.has(newNum)) newNum++;
  if (!newNum) newNum = current;

  if (newNum != current) {
    currentTool->setNumber(newNum);
    tools.erase(current);
    tools.add(currentTool);
    project->markDirty();
  }

  if (value != newNum) ui->toolSpinBox->setValue(newNum);

  projectModel->invalidate();
  redraw(true);
}


void QtWin::on_toolUnitsComboBox_currentIndexChanged(int value) {
  ToolUnits units = (ToolUnits::enum_t)value;

  real step = units == ToolUnits::UNITS_MM ? 1 : 0.125;
  ui->lengthDoubleSpinBox->setSingleStep(step);
  ui->diameterDoubleSpinBox->setSingleStep(step);
  ui->snubDiameterDoubleSpinBox->setSingleStep(step);

  PROTECT_UI_UPDATE;

  if (units == currentTool->getUnits()) return;

  currentTool->setUnits(units);
  project->markDirty();
  loadTool(currentTool->getNumber());

  projectModel->invalidate();
  redraw(true);
}


void QtWin::on_shapeComboBox_currentIndexChanged(int value) {
  ToolShape shape = (ToolShape::enum_t)value;

  real scale = currentTool->getUnits() == ToolUnits::UNITS_MM ? 1.0 : 25.4;
  real length = currentTool->getLength();
  real radius = currentTool->getRadius();
  if (shape == ToolShape::TS_BALLNOSE && length < radius)
    ui->lengthDoubleSpinBox->setValue(radius / scale);

  ui->snubDiameterDoubleSpinBox->setVisible(shape == ToolShape::TS_SNUBNOSE);
  ui->snubDiameterLabel->setVisible(shape == ToolShape::TS_SNUBNOSE);

  PROTECT_UI_UPDATE;

  if (shape == currentTool->getShape()) return;

  currentTool->setShape(shape);
  project->markDirty();

  projectModel->invalidate();
  redraw(true);
}


void QtWin::on_lengthDoubleSpinBox_valueChanged(double value) {
  real scale = currentTool->getUnits() == ToolUnits::UNITS_MM ? 1.0 : 25.4;
  value *= scale;

  real radius = currentTool->getRadius();
  if (currentTool->getShape() == ToolShape::TS_BALLNOSE && value < radius) {
    value = radius;
    ui->lengthDoubleSpinBox->setValue(value / scale);
  }

  PROTECT_UI_UPDATE;

  if (value == currentTool->getLength()) return;

  currentTool->setLength(value);
  project->markDirty();

  redraw(true);
}


void QtWin::on_diameterDoubleSpinBox_valueChanged(double value) {
  real scale = currentTool->getUnits() == ToolUnits::UNITS_MM ? 1.0 : 25.4;
  value *= scale;

  real length = currentTool->getLength();
  if (currentTool->getShape() == ToolShape::TS_BALLNOSE && length < value / 2)
    ui->lengthDoubleSpinBox->setValue(value / 2 / scale);

  PROTECT_UI_UPDATE;

  if (value == currentTool->getDiameter()) return;

  currentTool->setDiameter(value);
  project->markDirty();

  projectModel->invalidate();
  redraw(true);
}


void QtWin::on_snubDiameterDoubleSpinBox_valueChanged(double value) {
  PROTECT_UI_UPDATE;

  real scale = currentTool->getUnits() == ToolUnits::UNITS_MM ? 1.0 : 25.4;
  value *= scale;

  if (value == currentTool->getSnubDiameter()) return;

  currentTool->setSnubDiameter(value);
  project->markDirty();

  redraw(true);
}


void QtWin::on_descriptionLineEdit_textChanged(const QString &value) {
  PROTECT_UI_UPDATE;

  string description = value.toAscii().data();

  if (description == currentTool->getDescription()) return;

  currentTool->setDescription(description);
  project->markDirty();

  projectModel->invalidate();
  redraw(true);
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
  if (checkSave()) QCoreApplication::exit();
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
  reload(true);
}


void QtWin::on_actionBegining_triggered() {
  view->path->setByRatio(0);
  view->clearFlag(View::PLAY_FLAG);
  view->reverse = false;
  redraw();
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


void QtWin::on_actionEnd_triggered() {
  view->path->setByRatio(1);
  view->clearFlag(View::PLAY_FLAG);
  view->reverse = true;
  redraw();
}


void QtWin::on_actionDirection_triggered() {
  view->changeDirection();
}


void QtWin::on_actionExamples_triggered() {
  QAction *action = (QAction *)QObject::sender();
  if (action->toolTip().isEmpty()) return;
  openProject(action->toolTip().toAscii().data());
}


void QtWin::on_actionSave_triggered() {
  saveProject();
}


void QtWin::on_actionSaveAs_triggered() {
  saveProject(true);
}


void QtWin::on_actionSaveFile_triggered() {
  fileTabManager->save(ui->tabWidget->currentIndex());
}


void QtWin::on_actionSaveFileAs_triggered() {
  fileTabManager->save(ui->tabWidget->currentIndex(), true);
  projectModel->invalidate();
}


void QtWin::on_actionRevertFile_triggered() {
  fileTabManager->revert(ui->tabWidget->currentIndex());
}


void QtWin::on_actionExport_triggered() {
  exportData();
}


void QtWin::on_actionSnapshot_triggered() {
  snapshot();
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
  msg.setWindowTitle("OpenSCAM Help");
  msg.setTextFormat(Qt::RichText);
  msg.setText("Help can be find in the online User's Manual at "
              "<a href='http://openscam.org/manual.html'"
              ">http://openscam.org/manual.html</a>.");
  msg.exec();
}


void QtWin::on_actionCutSurface_activated() {
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


void QtWin::on_actionWorkpieceSurface_activated() {
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


void QtWin::on_actionWireSurface_activated() {
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


void QtWin::on_actionHideSurface_activated() {
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


void QtWin::on_actionRemoveFile_triggered() {
  removeFile();
}


void QtWin::on_actionAddTool_triggered() {
  addTool();
}


void QtWin::on_actionRemoveTool_triggered() {
  removeTool();
}
