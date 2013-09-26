// Happy Cube Solver - Building and designing models for the Happy cube puzzles
// Copyright (C) 1995-2006 Shy Shalom, shooshX@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef __MAINWINDOW_H__INCLUDED__
#define __MAINWINDOW_H__INCLUDED__

#include "BuildGLWidget.h" ///< needed for the enum BuildGLWidget::EActStatus

#include <QMainWindow>

/** \file
    Declares MainWindow which is the widget all other widgets decend from.
*/

class QSlider;
class ModelGLWidget;
class PicsSelectWidget;
class QAction;
class QActionGroup;
class QMenu;
class QStackedWidget;

class CubeDoc;
class QGLWidget;
class GLWidget;
class ActFocusComboBox;
class SolveDlg;
class ModelHelpDlg;
class BuildHelpDlg;
class GrpColorDlg;
class AssembleStepDlg;
class QTabWidget;
class PicInitThread;


/** MainWindow is the main GUI window. All other GUI elemets are spawned from it.
    Physically, MainWindow is the widget that surrounds the whole application
    and which contains the OpenGL view and the side bar.
    MainWindow creates all of the menus, actions, toolbars (which are not visible by default),
    status bar, side bars and OpenGL views. It is also responsible for connecting
    their signals and slots. Infact, all GUI-related signals and slots are connected
    by MainWindow in connectActions().
    MainWindow is also responsible for creating any dialog boxes when they are needed
    with the exception of the Open/Save file dialogues which are created by CubeDoc.
    It also centeralizes the required pre-run initalization in initialize()
    although much of the work there is unrealted directly to the GUI.
    There is only a single instance of this class thoughout the lifetime of the application.
    It is created in the main() function of the application. This instance holds the CubeDoc
    instance and all instances of all widgets created in the application.
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
 
    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;

    // called after opengl is initiated, after the ctor
    bool initialize();
    void loadStartupFile(QString filename);
    bool wasClosed() { return m_wasClosed; }

public slots:
    void setCurrentFile(QString fileName) { setCurrentFile(fileName, "recentFilesList", m_recentFilesActs); }
    void setTitleUntitled();
    void switchToModel_NoLast() { switchTo_NoLast(ModelView); }
    void switchToBuild_NoLast() { switchTo_NoLast(BuildView); }
    void switchToPics_NoLast() { switchTo_NoLast(PicsView); }
    void switchToBuildOrPics_NoLast();
    void setView(QAction *act);
    //void changedActionsAct(QAction *act);
    void updateSolveBrowseEnable();
    void updateFileEnables();
    void slvProgUpdate(int hint, int data);
    void doModalOptionsDlg();
    void setTitleChanged(bool state);

    void setActSlvIndex(int n); // doc->act (slv combo)
    void setDocSlvIndex();   // act->doc (slv combo)

    void doModalAboutDlg();
    void doUsage();
    void doHomepage();

    void setStatusTextBuild(int t, BuildGLWidget::EActStatus remove);

private slots:
    void openRecentFile();
    void setView(int tab); // tab index from the sidebar

signals:
    void openFile(QString filename); // unknown file type. from recent and command line

    void initializeComplete();
    void picsLoadComplete();

private:

    enum EView { ModelView = 0, BuildView = 1, PicsView = 2 }; 

    void createActions();
    void createMenus();
    void spawnMenuBar();
    //void createToolBars();
    void createStatusBar();
    void reConnect3DActions(GLWidget *target);
    void connectActions();
    void switchTo_NoLast(EView view);


    // read the recent files list from the registry
    void readRecentFileActions(const QString &list, QAction *acts[]);

    void updateRecentActions(QAction *acts[], const QStringList &files);
    void setCurrentFile(const QString &fileName, const QString &list, QAction *acts[]);
    void showTitle();

    void readRecentFileActions(const QString &list);

    void EnableBuildActs(bool en);
    void Enable3DActions(bool en);

    void saveSizeToReg();
    void resizeFromReg();

protected:
    virtual void closeEvent(QCloseEvent* e);
    virtual void keyPressEvent(QKeyEvent *event); // capture the keys from all windows
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

private:
    PicInitThread *m_picsInitThread;

    ModelGLWidget *m_modelGlWidget;
    BuildGLWidget *m_buildGlWidget;
    PicsSelectWidget *m_picsWidget;
    QStackedWidget *m_stack;
    BuildHelpDlg *m_buildDlg;
    ModelHelpDlg *m_modelDlg;
    QDockWidget *m_dock;
    QTabWidget *m_docktab;

    GLWidget *m_glWidget; ///< points to either m_modelGlWidget or m_buildGlWidget

    SolveDlg *m_slvdlg;
    GrpColorDlg *m_grpColDlg;
    AssembleStepDlg *m_asmStepDlg;

    CubeDoc* m_doc;


    /// used to detect if the window was closed before it got to exec(). (closed in the inialization)
    bool m_wasClosed;

    // title members
    QString m_filename;
    bool m_bChanged;

    // these indexes has to do with the order in which the views enter the stack
    int m_lastViewIndex;
    int m_curView;
    // bToLast means if its OK to move to the last view and not to this one (second click)
    void SwitchView(int nSwitchTo, bool bToLast);


    QMenu *m_fileMenu;
    QMenu *m_recentFilesMenu;
    QMenu *m_viewMenu;
    QMenu *m_toobarsPopMenu;
    QMenu *m_actionsMenu;
    QMenu *m_buildMenu;
    QMenu *m_actionMenu;
    QMenu *m_helpMenu;
    //QToolBar *m_3DviewToolbar;
    //QToolBar *m_viewsToolbar;
    //QToolBar *m_buildToolbar;
    //QToolBar *m_solvesToolbar;

public:
    QAction *m_newShapeAct;
    QAction *m_resetSolAct;
    QAction *m_openAct;
    QAction *m_saveAct;
    QAction *m_exportAct;
    QAction *m_exitAct;
    QAction *m_goAct;

    enum { MaxRecentFiles = 4 };
    QAction *m_recentFilesActs[MaxRecentFiles + 1]; // the last one is the seperator

    QAction *m_statusBarTriggered;

    QActionGroup *m_viewActs;
    QAction *m_picsViewAct; // needed seperatly for the build help dlg

    QAction *m_resetViewAct;
    QAction *m_continueModeAct; // continuous mode, where the object rotates on its own
    //QActionGroup *m_viewFrustrumActs;
    //QActionGroup *m_axisActs;
    //QActionGroup *m_actionsActs;
    //QActionGroup *m_spaceActs;
    QAction *m_showSlvDlgAct, *m_showGrpColorAct, *m_showAsmDlgAct;
    QAction *m_editOptionsAct;

    QAction *m_aboutAct, *m_aboutQtAct, *m_helpUsage, *m_helpHomepage;
    
    QAction *m_selectYellowAct;
    QAction *m_selectBlueAct;
    QAction *m_showAllBlueAct;

    enum ESlvAction { FirstSolve, PrevSolve, NextSolve, LastSolve };
    QActionGroup *m_slvBrowse;
    QAction *m_slvFirstAct;
    QAction *m_slvPrevAct;
    ActFocusComboBox *m_slvCombo;
    QAction *m_slvNextAct;
    QAction *m_slvLastAct;

    QAction *m_slvNumAct; // used to hold the number of solution for the combo boxen
};


#endif // __MAINWINDOW_H__INCLUDED__
