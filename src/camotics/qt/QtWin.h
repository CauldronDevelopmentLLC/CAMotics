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


#include "NewDialog.h"
#include "NewProjectDialog.h"
#include "ExportDialog.h"
#include "AboutDialog.h"
#include "SettingsDialog.h"
#include "DonateDialog.h"
#include "FindDialog.h"
#include "FileDialog.h"
#include "ToolDialog.h"
#include "CAMDialog.h"
#include "ConnectDialog.h"
#include "UploadDialog.h"
#include "BBCtrlAPI.h"

#include <camotics/ConcurrentTaskManager.h>
#include <camotics/view/View.h>
#include <camotics/view/Tool2DView.h>
#include <camotics/value/ValueSet.h>

#include <cbang/SmartPointer.h>
#include <cbang/Application.h>
#ifndef Q_MOC_RUN
#include <cbang/iostream/LineBufferDevice.h>
#endif

#include <QtWidgets>
#include <QSignalMapper>
#include <QApplication>
#include <QTranslator>

namespace cb {class Application;}
namespace Ui {class CAMoticsWindow;}
namespace GCode {class ToolPath;}
class QMdiSubWindow;


namespace CAMotics {
  namespace Project {class Project;}
  class Simulation;
  class SimulationRun;
  class CutWorkpiece;
  class ConsoleWriter;
  class ToolPathTask;
  class SurfaceTask;
  class ReduceTask;
  class Opt;


  class QtWin : public QMainWindow, public TaskObserver {
    Q_OBJECT;

    int taskCompleteEvent = 0; // Must be before creating UI elements below

    cb::SmartPointer<Ui::CAMoticsWindow> ui;
    NewDialog newDialog;
    NewProjectDialog newProjectDialog;
    ExportDialog exportDialog;
    AboutDialog aboutDialog;
    SettingsDialog settingsDialog;
    DonateDialog donateDialog;
    FindDialog findDialog;
    FindDialog findAndReplaceDialog;
    ToolDialog toolDialog;
    CAMDialog camDialog;
    ConnectDialog connectDialog;
    UploadDialog uploadDialog;
    FileDialog fileDialog;
    QTimer animationTimer;
    QByteArray fullLayoutState;
    ConcurrentTaskManager taskMan;

    QIcon playIcon;
    QIcon pauseIcon;
    QIcon forwardIcon;
    QIcon backwardIcon;

    QLabel *statusLabel;
    QString currentLang;

    cb::Application &app;
    cb::Options &options;

    QApplication &qtApp;
    QTranslator qtTran;
    QTranslator tran;

    ValueSet valueSet;
    cb::SmartPointer<Project::Project> project;
    cb::SmartPointer<SimulationRun> simRun;
    cb::SmartPointer<View> view;
    cb::SmartPointer<GCode::ToolPath> toolPath;
    std::string gcode;
    cb::SmartPointer<Surface> surface;

    QSignalMapper recentProjectsMapper;
    cb::SmartPointer<BBCtrlAPI> bbCtrlAPI;

    double lastRedraw     = 0;
    bool dirty            = false;
    bool simDirty         = false;
    unsigned inUIUpdate   = 0;
    double lastProgress   = 0;
    std::string lastStatus;
    bool lastStatusActive = false;
    bool autoPlay         = false;
    bool autoClose        = false;
    std::string defaultExample;
    bool sliderMoving     = false;
    bool positionChanged  = false;

    cb::SmartPointer<cb::LineBufferStream<ConsoleWriter> > consoleStream;

  public:
    QtWin(cb::Application &app, QApplication &qtApp);
    ~QtWin();

    View &getView() const {return *view;}

    void setAutoPlay(bool x = true) {autoPlay = x;}
    void setAutoClose(bool x = true) {autoClose = x;}

    void init();
    void show();

    void setUnitLabel(QLabel *label, double value, int precision = 2,
                      bool withUnit = false);

    void loadLanguageMenu();
    void switchTranslator(QTranslator &translator, const QString &name);
    void loadLanguage(const QString &lang);

    void loadMachine(const std::string &machine);
    void loadMachines();
    void loadDefaultExample();
    void loadExamples();
    void loadRecentProjects();

    void updateRecentProjects(const std::string &filename);

    void saveAllState();
    void restoreAllState();
    void setDefaultGeometry();
    void saveFullLayout();
    void fullLayout();
    void defaultLayout();
    void minimalLayout();

    void snapView(char v);

    void showMessage(const QString &msg, bool log = true);
    void showMessage(const char *msg, bool log = true);
    void message(const QString &msg);
    void warning(const QString &msg);

    void loadToolPath(const cb::SmartPointer<GCode::ToolPath> &toolPath,
                      bool simulate);
    void uploadGCode();

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

    bool runNewProjectDialog();
    GCode::ToolTable getNewToolTable();
    GCode::Units getNewUnits();

    bool runCAMDialog(const std::string &filename);

    QString openFile(const QString &title, const QString &filters,
                     const QString &filename, bool save,
                     bool anyFile = false);
    const cb::SmartPointer<Project::Project> &getProject() const
    {return project;}
    void loadProject();
    void newProject();
    void openProject(const std::string &filename = std::string());
    bool saveProject(bool saveas = false);
    void revertProject();
    bool isMetric() const;
    GCode::Units getDefaultUnits() const;

    void updateFiles();
    void newFile(bool tpl);
    void addFile();
    void editFile(unsigned index);
    void removeFile(unsigned index);
    bool checkSave(bool canCancel = true);
    void activateFile(const std::string &filename, int line = -1, int col = -1);

    void updateActions();
    void updateUnits();
    void updateToolTables();

    void toolsChanged();
    void editTool(unsigned number);
    void addTool();
    void removeTool(unsigned number);
    void exportToolTable();
    void importToolTable();
    void saveDefaultToolTable(const GCode::ToolTable &tools);
    GCode::ToolTable loadDefaultToolTable();

    void updateWorkpiece();
    void loadWorkpiece();
    void setWorkpieceDim(unsigned dim, double value);
    void setWorkpieceOffset(unsigned dim, double value);

    void updateBounds();
    void updateToolPathBounds();
    void updateWorkpieceBounds();

    bool isActive() {return lastStatusActive;}
    void setStatusActive(bool active);

    void showConsole();
    void hideConsole();
    void appendConsole(const std::string &line);

    // Value Observers
    void updatePlaySpeed(const std::string &name, unsigned value);
    void updateViewFlags(const std::string &name, unsigned flags);
    void updatePlayDirection(const std::string &name, bool reverse);
    void updateTimeRatio(const std::string &name, double ratio);

    void updateX(const std::string &name, double value);
    void updateY(const std::string &name, double value);
    void updateZ(const std::string &name, double value);

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
    void resizeEvent(QResizeEvent *event);
    void changeEvent(QEvent *event);

  protected slots:
    void animate();
    void openRecentProjectsSlot(const QString path);

    void on_openProject(QString path);

    void on_bbctrlConnect();
    void on_bbctrlDisconnect();
    void on_bbctrlConnected();
    void on_bbctrlDisconnected();
    void on_machineChanged(QString machine, QString path);

    void on_fileTabManager_currentChanged(int index);
    void on_positionSlider_valueChanged(int position);
    void on_positionSlider_sliderPressed();
    void on_positionSlider_sliderReleased();

    void on_filesListView_activated(const QModelIndex &index);
    void on_filesListView_customContextMenuRequested(QPoint point);

    void on_toolTableListView_activated(const QModelIndex &index);
    void on_toolTableListView_customContextMenuRequested(QPoint point);

    void on_automaticCuboidRadioButton_clicked();
    void on_marginDoubleSpinBox_valueChanged(double value);
    void on_manualCuboidRadioButton_clicked();
    void on_xDimDoubleSpinBox_valueChanged(double value);
    void on_yDimDoubleSpinBox_valueChanged(double value);
    void on_zDimDoubleSpinBox_valueChanged(double value);
    void on_xOffsetDoubleSpinBox_valueChanged(double value);
    void on_yOffsetDoubleSpinBox_valueChanged(double value);
    void on_zOffsetDoubleSpinBox_valueChanged(double value);

    void on_languageChanged(QAction *action);

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

    void on_actionExportToolTable_triggered() {exportToolTable();}
    void on_actionImportToolTable_triggered() {importToolTable();}
    void on_actionSaveDefaultToolTable_triggered();
    void on_actionLoadDefaultToolTable_triggered();

    void on_actionSettings_triggered();
    void on_actionExport_triggered() {exportData();}
    void on_actionSnapshot_triggered() {snapshot();}
    void on_actionConnect_triggered(bool checked);

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
    void on_actionMachine_triggered(bool checked);
    void on_actionWorkpieceBounds_triggered(bool checked);
    void on_actionAxes_triggered(bool checked);
    void on_actionIntensity_triggered(bool checked);
    void on_actionToolPath_triggered(bool checked);

    void on_actionAddFile_triggered();
    void on_actionEditFile_triggered();
    void on_actionRemoveFile_triggered();
    void on_actionAddTool_triggered();
    void on_actionEditTool_triggered();
    void on_actionRemoveTool_triggered();

    void on_actionHideConsole_triggered();
    void on_actionShowConsole_triggered();
    void on_actionToggleConsole_triggered();

    void on_hideConsolePushButton_clicked();
    void on_clearConsolePushButton_clicked();

    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();
    void on_actionZoomAll_triggered();
  };
}
