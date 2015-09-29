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

#ifndef CAMOTICS_QT_WIN_H
#define CAMOTICS_QT_WIN_H

#include "NewDialog.h"
#include "ExportDialog.h"
#include "AboutDialog.h"
#include "DonateDialog.h"
#include "FindDialog.h"
#include "FileDialog.h"

#include <camotics/Real.h>
#include <camotics/ConcurrentTaskManager.h>
#include <camotics/view/View.h>
#include <camotics/view/ToolView.h>
#include <camotics/value/ValueSet.h>

#include <cbang/SmartPointer.h>
#include <cbang/Application.h>
#ifndef Q_MOC_RUN
#include <cbang/iostream/LineBufferDevice.h>
#endif

#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QtGui>
#else
#include <QtWidgets>
#endif

namespace cb {class Application;}
namespace Ui {class CAMoticsWindow;}
class QMdiSubWindow;


namespace CAMotics {
  class ConnectionManager;
  class Viewer;
  class Project;
  class Simulation;
  class ProjectModel;
  class ToolPath;
  class CutWorkpiece;
  class ConsoleWriter;
  class ToolPathTask;
  class SurfaceTask;
  class ReduceTask;
  class Opt;


  class QtWin : public QMainWindow, public TaskObserver {
    Q_OBJECT;

    cb::SmartPointer<Ui::CAMoticsWindow> ui;
    NewDialog newDialog;
    ExportDialog exportDialog;
    AboutDialog aboutDialog;
    DonateDialog donateDialog;
    FindDialog findDialog;
    FindDialog findAndReplaceDialog;
    FileDialog fileDialog;
    QTimer animationTimer;
    QGraphicsScene toolScene;
    QByteArray fullLayoutState;
    cb::SmartPointer<ProjectModel> projectModel;
    ConcurrentTaskManager taskMan;
    int taskCompleteEvent;

    QIcon playIcon;
    QIcon pauseIcon;
    QIcon forwardIcon;
    QIcon backwardIcon;

    QLabel *statusLabel;

    cb::Application &app;
    cb::Options &options;

    ValueSet valueSet;
    cb::SmartPointer<Project> project;
    cb::SmartPointer<Simulation> sim;
    cb::SmartPointer<ConnectionManager> connectionManager;
    cb::SmartPointer<View> view;
    cb::SmartPointer<Viewer> viewer;
    cb::SmartPointer<ToolView> toolView;
    cb::SmartPointer<ToolPath> toolPath;
    cb::SmartPointer<std::vector<char> > gcode;
    cb::SmartPointer<Surface> surface;

    double lastRedraw;
    bool dirty;
    bool simDirty;
    unsigned inUIUpdate;
    double lastProgress;
    std::string lastStatus;
    bool lastStatusActive;
    Tool *currentTool;
    bool autoPlay;
    bool autoClose;
    std::string defaultExample;

    typedef enum {
      NULL_VIEW,
      SIMULATION_VIEW,
      TOOL_VIEW,
      FILE_VIEW,
    } ui_view_t;
    ui_view_t currentUIView;

    cb::SmartPointer<cb::LineBufferStream<ConsoleWriter> > consoleStream;

  public:
    QtWin(cb::Application &app);
    ~QtWin();

    const cb::SmartPointer<ConnectionManager> &getConnectionManager() const
    {return connectionManager;}
    const cb::SmartPointer<View> &getView() const {return view;}

    void setAutoPlay(bool x = true) {autoPlay = x;}
    void setAutoClose(bool x = true) {autoClose = x;}

    void init();
    void setUnitLabel(QLabel *label, real value, int precision = 2);

    void loadDefaultExample();
    void loadExamples();

    void saveAllState();
    void restoreAllState();
    void setDefaultGeometry();
    void saveFullLayout();
    void fullLayout();
    void defaultLayout();
    void minimalLayout();

    cb::SmartPointer<ViewPort> getCurrentViewPort() const;
    void snapView(char v);

    void glViewMousePressEvent(unsigned id, QMouseEvent *event);
    void glViewMouseMoveEvent(unsigned id, QMouseEvent *event);
    void glViewWheelEvent(unsigned id, QWheelEvent *event);

    void initializeGL(unsigned id);
    void resizeGL(unsigned id, int w, int h);
    void paintGL(unsigned id);

    void showMessage(const std::string &msg, double timeout = 30);
    void message(const std::string &msg);
    void warning(const std::string &msg);

    void loadToolPath(const cb::SmartPointer<ToolPath> &toolPath,
                      bool simulate);

    void toolPathComplete(ToolPathTask &task);
    void surfaceComplete(SurfaceTask &task);
    void reduceComplete(ReduceTask &task);
    void optimizeComplete(Opt &task);

    void quit();
    void stop();
    void reload(bool now = false);
    void reduce();
    void optimize();
    void redraw(bool now = false);
    void snapshot();
    void exportData();

    std::string openFile(const std::string &title,
                         const std::string &filters,
                         const std::string &filename, bool save);
    const cb::SmartPointer<Project> &getProject() const {return project;}
    void loadProject();
    void resetProject();
    void newProject();
    void openProject(const std::string &filename = std::string());
    bool saveProject(bool saveas = false);
    void revertProject();
    void newFile(bool tpl);
    void addFile();
    void removeFile();
    bool checkSave(bool canCancel = true);
    void activateFile(const std::string &filename, int line = -1, int col = -1);

    void updateActions();
    void updateUnits();

    void loadTool(unsigned number);
    void addTool();
    void removeTool();
    void updateToolUI();

    void updateWorkpiece();
    void loadWorkpiece();
    void setWorkpieceDim(unsigned dim, real value);
    void setWorkpieceOffset(unsigned dim, real value);

    void updateBounds();
    void updateToolPathBounds();
    void updateWorkpieceBounds();

    void setStatusActive(bool active);

    void showConsole();
    void hideConsole();
    void appendConsole(const std::string &line);

    // Views
    void setUIView(ui_view_t uiView);

    // Value Observers
    void updatePlaySpeed(const std::string &name, unsigned value);
    void updateViewFlags(const std::string &name, unsigned flags);
    void updatePlayDirection(const std::string &name, bool reverse);
    void updateTimeRatio(const std::string &name, double ratio);

    void updateX(const std::string &name, real value);
    void updateY(const std::string &name, real value);
    void updateZ(const std::string &name, real value);

    void updateCurrentTime(const std::string &name, double value);
    void updateCurrentDistance(const std::string &name, double value);
    void updateRemainingTime(const std::string &name, double value);
    void updateRemainingDistance(const std::string &name, double value);
    void updateTotalTime(const std::string &name, double value);
    void updateTotalDistance(const std::string &name, double value);
    void updatePercentTime(const std::string &name, double value);
    void updatePercentDistance(const std::string &name, double value);

    void updateTool(const std::string &name, unsigned value);
    void updateFeed(const std::string &name, double value);
    void updateSpeed(const std::string &name, double value);
    void updateDirection(const std::string &name, const char *value);
    void updateProgramLine(const std::string &name, unsigned value);

  protected:
    // From TaskObserver
    void taskCompleted();

    // From QMainWindow
    bool event(QEvent *event);
    void closeEvent(QCloseEvent *event);

  protected slots:
    void animate();

    void on_fileTabManager_currentChanged(int index);

    void on_resolutionComboBox_currentIndexChanged(int index);
    void on_resolutionDoubleSpinBox_valueChanged(double value);
    void on_unitsComboBox_currentIndexChanged(int value);

    void on_positionSlider_valueChanged(int position);

    void on_projectTreeView_activated(const QModelIndex &index);
    void on_projectTreeView_customContextMenuRequested(QPoint point);

    void on_toolSpinBox_valueChanged(int value);
    void on_toolUnitsComboBox_currentIndexChanged(int value);
    void on_shapeComboBox_currentIndexChanged(int value);
    void on_lengthDoubleSpinBox_valueChanged(double value);
    void on_diameterDoubleSpinBox_valueChanged(double value);
    void on_snubDiameterDoubleSpinBox_valueChanged(double value);
    void on_descriptionLineEdit_textChanged(const QString &value);

    void on_automaticCuboidRadioButton_clicked();
    void on_marginDoubleSpinBox_valueChanged(double value);
    void on_manualCuboidRadioButton_clicked();
    void on_xDimDoubleSpinBox_valueChanged(double value);
    void on_yDimDoubleSpinBox_valueChanged(double value);
    void on_zDimDoubleSpinBox_valueChanged(double value);
    void on_xOffsetDoubleSpinBox_valueChanged(double value);
    void on_yOffsetDoubleSpinBox_valueChanged(double value);
    void on_zOffsetDoubleSpinBox_valueChanged(double value);

    void on_actionQuit_triggered();
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionExamples_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionSaveFile_triggered();
    void on_actionSaveFileAs_triggered();
    void on_actionRevertFile_triggered();

    void on_actionStop_triggered();
    void on_actionRun_triggered();
    void on_actionReduce_triggered();
    void on_actionOptimize_triggered();
    void on_actionSlower_triggered();
    void on_actionPlay_triggered();
    void on_actionFaster_triggered();
    void on_actionDirection_triggered();

    void on_actionExport_triggered();
    void on_actionSnapshot_triggered();

    void on_actionFullscreen_triggered(bool checked);
    void on_actionDefaultLayout_triggered();
    void on_actionFullLayout_triggered();
    void on_actionMinimalLayout_triggered();

    void on_actionAbout_triggered();
    void on_actionDonate_triggered();
    void on_actionHelp_triggered();

    void on_actionIsoView_triggered() {snapView('p');}
    void on_actionFrontView_triggered() {snapView('F');}
    void on_actionBackView_triggered() {snapView('B');}
    void on_actionLeftView_triggered() {snapView('l');}
    void on_actionRightView_triggered() {snapView('r');}
    void on_actionTopView_triggered() {snapView('t');}
    void on_actionBottomView_triggered() {snapView('b');}

    void on_actionCutSurface_triggered();
    void on_actionWorkpieceSurface_triggered();
    void on_actionWireSurface_triggered();
    void on_actionHideSurface_triggered();
    void on_actionTool_triggered(bool checked);
    void on_actionWorkpieceBounds_triggered(bool checked);
    void on_actionAxes_triggered(bool checked);
    void on_actionToolPath_triggered(bool checked);

    void on_actionAddFile_triggered();
    void on_actionRemoveFile_triggered();
    void on_actionAddTool_triggered();
    void on_actionRemoveTool_triggered();

    void on_actionHideConsole_triggered();
    void on_actionShowConsole_triggered();

    void on_hideConsolePushButton_clicked();
    void on_clearConsolePushButton_clicked();
  };
}

#endif // CAMOTICS_QT_WIN_H
