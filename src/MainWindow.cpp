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

#include "GlobDefs.h"

#include "ModelGLControl.h"
#include "BuildGLControl.h"
#include "PicsSelectWidget.h"
#include "MainWindow.h"
#include "CubeDoc.h"

#include "SolveDlg.h"
#include "OptionsDlg.h"
#include "BuildHelpDlg.h"
#include "ModelHelpDlg.h"
#include "GrpColorDlg.h"
#include "ActFocusComboBox.h"
#include "AssembleStepDlg.h"

#include "ui_about.h"

#include <QtGui>
#include <QActionGroup>
#include <QScrollArea>
#include <QStackedWidget>
#include <QDockWidget>
#include <QMessageBox>
#include <QProgressDialog>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>


#define SIZE_DEFAULT_X 800
#define SIZE_DEFAULT_Y 560
#define SIZE_DEFAULT_MAX 1 // maximized or not

#define RESTRICTED_OPTIONS 1


/// select a specific QAction from a QActionGroup according to its data.
/// the data needs to be an int. if no such QAction is found, returns nullptr.
QAction *selectGroupAct(QActionGroup *group, int sel)
{
    QList<QAction*> list = group->actions();
    foreach(QAction* act, list)
    {
        if (act->data().toInt() == sel)
        {
            return act;
        }
    }
    return nullptr;
}


MainWindow::MainWindow()
    :m_picsInitThread(nullptr), m_wasClosed(false), m_bChanged(false), m_lastViewIndex(-1), m_curView(-1)
{
    setWindowTitle(APP_NAME);
    

    // the general directive is that everything should be created as it is supposed to be
    // upon program startup. no additional updates should be necessary after this ctor is done.
    m_doc = new CubeDoc(this);

    createActions();
    //createToolBars();
    createStatusBar();
    // the main creation of the menubar needs to be done after the creation of the dock
    // since the dock has actions that needs to be in the menu
    // but we can't let the menubar be spawned after the gl widgets becuase then it looks
    // to appear from nowhere when most of the view is already loaded.
    // so its spawned here and filled up later.
    spawnMenuBar();

    // this should come before the dock creation due to call list ownership
    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);
    m_glWidget = new GLWidget(m_stack);
//    ShaderProgram::shadersInit();

    m_modelGLControl = new ModelGLControl(m_glWidget, m_doc); // views need to start with MainWindow as parent
    m_buildGLControl = new BuildGLControl(m_glWidget, m_doc);
    m_glWidget->m_handlers.push_back(m_modelGLControl);
    m_glWidget->m_handlers.push_back(m_buildGLControl);
    m_glWidget->switchHandler(m_buildGLControl);
    m_curView = BuildView;

    reConnect3DActions(m_glWidget);
    EnableBuildActs(true);

    m_picsWidget = new PicsSelectWidget(m_stack, m_doc);

    m_stack->addWidget(m_glWidget);
    m_stack->addWidget(m_picsWidget);
    m_stack->setCurrentIndex(0);


    m_dock = new QDockWidget(this);
    m_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_dock->setWindowTitle(tr("Sidebar"));
    m_docktab = new QTabWidget(m_dock);
    m_dock->setWidget(m_docktab);

    m_buildDlg = new BuildHelpDlg(m_docktab, this, m_doc); 
    m_docktab->addTab(m_buildDlg, QIcon(":/images/buildview.png"), "Design");
    m_modelDlg = new ModelHelpDlg(m_docktab, this, m_doc, m_glWidget);
    m_docktab->addTab(m_modelDlg, QIcon(":/images/modelview.png"), "Solutions");

    addDockWidget(Qt::RightDockWidgetArea, m_dock);

    createMenus();  // need to be after we created the docks

    m_slvdlg = new SolveDlg(this, m_doc);
    m_slvdlg->move(pos().x() + width() - m_slvdlg->width() - 7, pos().y() + height() - m_slvdlg->height() - 10);
    m_grpColDlg = new GrpColorDlg(this, m_doc);
    //m_grpColDlg->setAttribute(Qt::WA_Moved); //makes it stick to its position where it is moved
    m_asmStepDlg = new AssembleStepDlg(this);
    m_asmStepDlg->setAttribute(Qt::WA_Moved);



    updateSolveBrowseEnable();
    updateFileEnables();

    selectGroupAct(m_viewActs, BuildView)->trigger();
    connectActions();
    setAcceptDrops(true);

    resizeFromReg();

    show();

    //m_slvdlg->show();
    m_slvdlg->raise();

}

/** PicInitThread does some initialization in a seperate thread to avoid blocking the main GUI.
    TBD: consider removing as the main GUI is already blocked by the creation of the
    pieces display lists.
*/
class PicInitThread : public QThread
{	
protected:
    virtual void run()
    {
        PicBucket::mutableInstance().makeBitmapList();
    }
};


string readFile(const QString filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {// try again in the executable path
        file.setFileName(QCoreApplication::applicationDirPath() + "/" + filename); 
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(g_main, APP_NAME, "error opening file:\n" + filename, QMessageBox::Ok, 0);
            return string();
        }
    }

    QTextStream in(&file);
    QString data = in.readAll();
    return string(data.toLatin1().data());
}

class DlgProg : public ProgressCallback {
public:
    DlgProg(bool showStop) 
        : m_showStop(showStop), m_dlg("Building the pieces. Please Wait...", "Stop", 0, 100, g_main) {}

    virtual void init(int maxv) {
        m_dlg.setRange(0, maxv);
        m_dlg.setSizeGripEnabled(false);
        m_dlg.setWindowTitle("Happy Cube Solver");
        m_dlg.setMinimumDuration(1500);
        if (!m_showStop)
            m_dlg.setCancelButton(nullptr);
    }
    virtual bool setValue(int v) {
        m_dlg.setValue(v);
        qApp->processEvents(); // must call
        return !((m_showStop && m_dlg.wasCanceled()) || (((MainWindow*)g_main)->wasClosed()));
    }

private:
    bool m_showStop;
    QProgressDialog m_dlg;
};

// TBD possibly move to PicBucket
bool MainWindow::initialize()
{
    PicBucket::mutableInstance().setSwapEndians(m_doc->m_conf.disp.nSwapTexEndians);

    m_modelGLControl->initTex();
    // needs to be here because only here we have the glwidget
    if (!PicBucket::mutableInstance().loadXML(readFile(":/stdpcs.xml").c_str()))
        return false;

    m_picsInitThread = new PicInitThread();
    connect(m_picsInitThread, SIGNAL(finished()), this, SIGNAL(picsLoadComplete()));
    m_picsInitThread->start(); 

    DlgProg dlgprog(false);
    PicBucket::mutableInstance().buildMeshes(m_doc->m_conf.disp, &dlgprog); // creates the meshes from scratch
    //PicBucket::mutableInstance().loadMeshes("C:/projects/cubeGL/happysolver/small_meshes_all.txt"); // loads ununified meshes file

    //PicBucket::mutableInstance().loadUnified(":/unified_meshes_all.txt"); // loads unified meshes file
    return true;
}

void MainWindow::loadStartupFile(QString filename) 
{ 
    m_picsInitThread->wait(); // need to wait for the GUI to finish building.
    flushAllEvents(); // get it all chewed and spat.
    // this is all needed so the pics count will be shown truthfully.
    emit openFile(filename); 
}

// TBD - move this to Configuration
void MainWindow::saveSizeToReg()
{
    QSettings reg("Happy Cube Solver", "Happy Cube Solver (QT)");

    reg.beginGroup("GUI");
    reg.setValue("maximized", (int)isMaximized());
    reg.setValue("sizeX", size().width());
    reg.setValue("sizeY", size().height());

    reg.endGroup();
}

void MainWindow::resizeFromReg()
{
    QSettings reg("Happy Cube Solver", "Happy Cube Solver (QT)");

    reg.beginGroup("GUI");
    if (reg.value("maximized", SIZE_DEFAULT_MAX).toInt() != 0)
    {
        showMaximized();
    }
    else
    {
        resize(reg.value("sizeX", SIZE_DEFAULT_X).toInt(), reg.value("sizeY", SIZE_DEFAULT_Y).toInt());
    }
    reg.endGroup();

}

void MainWindow::closeEvent(QCloseEvent* e)
{
    m_doc->solveStop();
    if (m_doc->checkUnsaved())
    {
        m_slvdlg->close();
        m_asmStepDlg->close();
        saveSizeToReg();
        e->accept();
    }
    else
        e->ignore();
    m_wasClosed = true;
}


QSize MainWindow::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize MainWindow::sizeHint() const
{
    return QSize(SIZE_DEFAULT_X, SIZE_DEFAULT_Y);
}

void MainWindow::createActions()
{
    m_newShapeAct = new QAction(QIcon(":/images/newshape.png"), tr("&New Shape"), this);
    m_newShapeAct->setIconText(tr("New"));
    m_newShapeAct->setStatusTip(tr("Restart a new Shape build"));

    m_openAct = new QAction(QIcon(":/images/opensolve.png"), tr("&Open..."), this);
    m_saveAct = new QAction(QIcon(":/images/savesolve.png"), tr("&Save As..."), this);
    m_exportAct = new QAction(QIcon(":/images/savesolve.png"), tr("&Export..."), this);
    m_exportAct->setToolTip("Export current solution");
    m_goAct = new QAction("&Solve It!", this);

    m_resetSolAct = new QAction(QIcon(":/images/resetsolves.png"), tr("&Reset Solutions"), this);
    m_resetSolAct->setIconText("Erase");
    m_resetSolAct->setToolTip("Erase all solutions");

    for (int i = 0; i < MaxRecentFiles; ++i) 
    {
        m_recentFilesActs[i] = new QAction(this);
        m_recentFilesActs[i]->setVisible(false);
        connect(m_recentFilesActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile())); 
    }

    m_exitAct = new QAction(tr("E&xit"), this);

    m_statusBarTriggered = new QAction(tr("&Status Bar"), this);
    m_statusBarTriggered->setCheckable(true);
    m_statusBarTriggered->setChecked(true); // status bar is on

    m_resetViewAct = new QAction(QIcon(":/images/resetview.png"), tr("Reset View"), this);
    m_resetViewAct->setStatusTip(tr("Reset 3D projection matrix"));

    m_helpHomepage = new QAction("&Home Page...", this);
    m_helpUsage = new QAction("&Usage..." ,this);
    m_aboutAct = new QAction(QIcon(":/images/about.png"), tr("&About Happy Cube Solver..."), this);
    m_aboutQtAct = new QAction("About &Qt", this);

    // 3d view actions
    m_continueModeAct = new QAction(QIcon(":/images/continueRotate.png"), tr("&Continuous Mode"), this);
    m_continueModeAct->setCheckable(true);

    QAction *act;
    /*m_viewFrustrumActs = new QActionGroup(this);
    act = new QAction(QIcon(":/images/viewortho.png"), tr("&Orthographic"), m_viewFrustrumActs);
    act->setData(GLWidget::Ortho);
    act->setCheckable(true);
    act = new QAction(QIcon(":/images/viewpersp.png"), tr("&Perspective"), m_viewFrustrumActs);
    act->setData(GLWidget::Perspective);
    act->setCheckable(true);

    m_actionsActs = new QActionGroup(this);
    act = new QAction(QIcon(":/images/rotate.png"), tr("&Rotate"), m_actionsActs);
    act->setData(GLWidget::Rotate);
    act->setCheckable(true);
    act = new QAction(QIcon(":/images/translate.png"), tr("&Translate"), m_actionsActs);
    act->setData(GLWidget::Translate);
    act->setCheckable(true);
    act = new QAction(QIcon(":/images/scale.png"), tr("&Scale"), m_actionsActs);
    act->setData(GLWidget::Scale);
    act->setCheckable(true);

    m_axisActs = new QActionGroup(this);
    act = new QAction(QIcon(":/images/axisXY.png"), tr("XY"), m_axisActs);
    act->setData(GLWidget::XYaxis);
    act->setCheckable(true);
    act = new QAction(QIcon(":/images/axisXZ.png"), tr("XZ"), m_axisActs);
    act->setData(GLWidget::XZaxis);
    act->setCheckable(true);
    act = new QAction(QIcon(":/images/axisYZ.png"), tr("YZ"), m_axisActs);
    act->setData(GLWidget::YZaxis);
    act->setCheckable(true);
    act = new QAction(QIcon(":/images/axisX.png"), tr("X"), m_axisActs);
    act->setData(GLWidget::Xaxis);
    act->setCheckable(true);
    act = new QAction(QIcon(":/images/axisY.png"), tr("Y"), m_axisActs);
    act->setData(GLWidget::Yaxis);
    act->setCheckable(true);
    act = new QAction(QIcon(":/images/axisZ.png"), tr("Z"), m_axisActs);
    act->setData(GLWidget::Zaxis);
    act->setCheckable(true);

    m_spaceActs = new QActionGroup(this);
    act = new QAction(QIcon(":/images/worldspace.png"), tr("&World Space"), m_spaceActs);
    act->setData(GLWidget::WorldSpace);
    act->setCheckable(true);
    act = new QAction(QIcon(":/images/objectspace.png"), tr("&Object Space"), m_spaceActs);
    act->setData(GLWidget::ObjectSpace);
    act->setCheckable(true);
    */

    m_viewActs = new QActionGroup(this);
    act = new QAction(QIcon(":/images/modelview.png"), tr("&Solution View"), m_viewActs);
    act->setData(ModelView);
    act->setCheckable(true);
    act = new QAction(QIcon(":/images/buildview.png"), tr("&Build shape view"), m_viewActs);
    act->setData(BuildView);
    act->setCheckable(true);
    m_picsViewAct = new QAction(QIcon(":/images/picsview.png"), tr("&Piece selection view"), m_viewActs);
    m_picsViewAct->setData(PicsView);
    m_picsViewAct->setCheckable(true);
    m_viewActs->setExclusive(false); // not exclusive because we want the buttons to unpress

    // build actions
    m_selectYellowAct = new QAction(QIcon(":/images/selectyellow.png"), tr("Define start tile mode"), this);
    m_selectYellowAct->setCheckable(true);
    m_selectYellowAct->setToolTip(tr("Define start"));
    m_selectBlueAct = new QAction(QIcon(":/images/selectblue.png"), tr("Clear blue tiles mode"), this);
    m_selectBlueAct->setCheckable(true);
    m_selectBlueAct->setToolTip(tr("Clear blue tiles"));
    m_showAllBlueAct = new QAction(QIcon(":/images/showallblues.png"), tr("Unclear all blue tiles"), this);
    m_showAllBlueAct->setToolTip(tr("Unclear blue tiles"));

    // solution browse actions
    m_slvBrowse = new QActionGroup(this); // group them in a group for comfort of signal
    m_slvBrowse->setExclusive(false);
    m_slvFirstAct = new QAction(QIcon(":/images/solvefirst.png"), tr("First Solution"), m_slvBrowse);
    m_slvFirstAct->setToolTip(tr("First Solution"));
    m_slvFirstAct->setData(FirstSolve	);
    m_slvPrevAct = new QAction(QIcon(":/images/solveprev.png"), tr("Previous Solution"), m_slvBrowse);
    m_slvPrevAct->setToolTip(tr("Previous Solution"));
    m_slvPrevAct->setData(PrevSolve);
    m_slvNextAct = new QAction(QIcon(":/images/solvenext.png"), tr("Next Solution"), m_slvBrowse);
    m_slvNextAct->setToolTip(tr("Next Solution"));
    m_slvNextAct->setData(NextSolve);
    m_slvLastAct = new QAction(QIcon(":/images/solvelast.png"), tr("Last Solution"), m_slvBrowse);
    m_slvLastAct->setToolTip(tr("Last Solution"));
    m_slvLastAct->setData(LastSolve);

    m_showSlvDlgAct = new QAction(QIcon(":/images/solvedlg.png"), tr("&Progress Dialog"), this);
    m_showSlvDlgAct->setCheckable(true);
    m_showSlvDlgAct->setToolTip(tr("Solution Progress Dialog"));

    m_showGrpColorAct = new QAction(tr("Pieces &Color Dialog"), this);
    m_showGrpColorAct->setCheckable(true);
    m_showAsmDlgAct = new QAction(QIcon(":/images/stepbystep.png"), tr("Step by Step &Dialog"), this);
    m_showAsmDlgAct->setCheckable(true);

    m_editOptionsAct = new QAction(QIcon(":/images/options.png"), tr("&Options..."), this);
    m_editOptionsAct->setToolTip(tr("Edit Options..."));
    
    m_slvNumAct = new QAction(this);

    m_transferShapeAct = new QAction("Transfer", this);
}

/// create the filler place for the menu bar to be filled later
void MainWindow::spawnMenuBar()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));
}

//. fill the menu bar with the menus
void MainWindow::createMenus()
{
    m_fileMenu->addAction(m_newShapeAct);
    m_fileMenu->addAction(m_openAct);
    m_fileMenu->addAction(m_saveAct);
#ifdef RESTRICTED_OPTIONS
    m_fileMenu->addAction(m_exportAct);
#endif
    m_fileMenu->addAction(m_resetSolAct);
    m_fileMenu->addSeparator();
//	m_recentFilesMenu = new QMenu(tr("Recent &Files"), m_fileMenu);
//	m_fileMenu->addMenu(m_recentFilesMenu);

    for (int i = 0; i < MaxRecentFiles; ++i)
        m_fileMenu->addAction(m_recentFilesActs[i]);
    m_recentFilesActs[MaxRecentFiles] = m_fileMenu->addSeparator();
    readRecentFileActions("recentFilesList", m_recentFilesActs);

    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAct);

    m_viewMenu = menuBar()->addMenu(tr("&View"));
    //m_toobarsPopMenu = new QMenu(tr("&Toolbars"), m_viewMenu);
    //m_toobarsPopMenu->addAction(m_3DviewToolbar->toggleViewAction());
    //m_toobarsPopMenu->addAction(m_viewsToolbar->toggleViewAction());
    //m_toobarsPopMenu->addAction(m_buildToolbar->toggleViewAction());
    //m_toobarsPopMenu->addAction(m_solvesToolbar->toggleViewAction());
    //m_toobarsPopMenu->addSeparator();
    //m_viewMenu->addMenu(m_toobarsPopMenu);
    m_viewMenu->addAction(m_dock->toggleViewAction());
    m_viewMenu->addAction(m_statusBarTriggered);
    m_viewMenu->addSeparator();
    //m_viewMenu->addActions(m_viewActs->actions());
    //m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_editOptionsAct);
    m_viewMenu->addAction(m_showSlvDlgAct);
#ifdef RESTRICTED_OPTIONS
    m_viewMenu->addAction(m_showGrpColorAct);
#endif
    m_viewMenu->addAction(m_showAsmDlgAct);

// 	m_actionsMenu = menuBar()->addMenu(tr("&3D"));
// 	m_actionsMenu->addAction(m_resetViewAct);
// 	m_actionsMenu->addSeparator();
// 	m_actionsMenu->addActions(m_actionsActs->actions());
// 	m_actionsMenu->addSeparator();
// 	m_actionsMenu->addActions(m_axisActs->actions());
// 	m_actionsMenu->addSeparator();
// 	m_actionsMenu->addActions(m_spaceActs->actions());
// 	m_actionsMenu->addSeparator();
// 	m_actionsMenu->addActions(m_viewFrustrumActs->actions());

    m_actionMenu = menuBar()->addMenu(tr("&Tools"));
    m_actionMenu->addAction(m_goAct);
    m_actionMenu->addAction(m_resetViewAct);
    m_actionMenu->addAction(m_selectYellowAct);

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_helpUsage);
    m_helpMenu->addAction(m_helpHomepage);
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_aboutAct);
    m_helpMenu->addAction(m_aboutQtAct);

}

/*
void MainWindow::createToolBars()
{
    m_3DviewToolbar = addToolBar(tr("3D view control"));
    m_3DviewToolbar->addAction(m_resetViewAct);
    QObject::connect(m_statusBarTriggered, SIGNAL(triggered(bool)), statusBar(), SLOT(setVisible(bool)));
    m_3DviewToolbar->addSeparator();
   // m_3DviewToolbar->addActions(m_viewFrustrumActs->actions());
   // m_3DviewToolbar->addSeparator();
//    m_3DviewToolbar->addActions(m_actionsActs->actions());
  //  m_3DviewToolbar->addSeparator();
   // m_3DviewToolbar->addActions(m_axisActs->actions());
//	m_3DviewToolbar->addSeparator();
//	m_3DviewToolbar->addActions(m_spaceActs->actions());

    m_viewsToolbar = addToolBar(tr("Views"));
    m_viewsToolbar->addActions(m_viewActs->actions());
    m_viewsToolbar->addAction(m_showSlvDlgAct);
    m_viewsToolbar->addAction(m_editOptionsAct);

    m_buildToolbar = addToolBar(tr("Build"));
    m_buildToolbar->addAction(m_newShapeAct);
    m_buildToolbar->addAction(m_selectYellowAct);
    m_buildToolbar->addAction(m_selectBlueAct);
   // m_buildToolbar->addAction(m_showAllBlueAct);

    m_solvesToolbar = addToolBar(tr("Solutions"));
    m_solvesToolbar->addAction(m_slvFirstAct);
    m_solvesToolbar->addAction(m_slvPrevAct);
    m_slvCombo = new ActFocusComboBox(m_slvNumAct, m_solvesToolbar);
    m_slvCombo->setEditable(true);
    m_slvCombo->setInsertPolicy(QComboBox::NoInsert);
    m_slvCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    m_slvCombo->setMinimumContentsLength(4);

    m_solvesToolbar->addWidget(m_slvCombo);
    m_solvesToolbar->addAction(m_slvNextAct);
    m_solvesToolbar->addAction(m_slvLastAct);

    m_viewsToolbar->hide();
    m_buildToolbar->hide();
    m_3DviewToolbar->hide();
    m_solvesToolbar->hide();

}
*/

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
    statusBar()->setVisible(m_statusBarTriggered->isChecked());

}

/// when a 3D view is switched, some actions need to be reconnected to the currently
/// active view.
void MainWindow::reConnect3DActions(GLWidget *target)
{
    //m_viewFrustrumActs->disconnect(SIGNAL(triggered(QAction*)));
    //selectGroupAct(m_viewFrustrumActs, target->currentViewState())->trigger();
    //connect(m_viewFrustrumActs, SIGNAL(triggered(QAction*)), target, SLOT(setViewFrustrum(QAction*)));

    //m_actionsActs->disconnect(SIGNAL(triggered(QAction*)));
    //QAction *actact = selectGroupAct(m_actionsActs, target->currentMouseAction());
    //actact->trigger();
    //changedActionsAct(actact); 	// enable/disable axis actions
    //connect(m_actionsActs, SIGNAL(triggered(QAction*)), target, SLOT(setAction(QAction*)));
    // there is a need to reconnect it everytime since it gets disconnected
    // a specific disconnect is too much trouble.
    //connect(m_actionsActs, SIGNAL(triggered(QAction*)), this, SLOT(changedActionsAct(QAction*)));

    //m_axisActs->disconnect(SIGNAL(triggered(QAction*)));
    //selectGroupAct(m_axisActs, target->currentAxis())->trigger();
    //connect(m_axisActs, SIGNAL(triggered(QAction*)), target, SLOT(setAxis(QAction*)));

    //m_spaceActs->disconnect(SIGNAL(triggered(QAction*)));
    //selectGroupAct(m_spaceActs, target->currentTransformType())->trigger();
    //connect(m_spaceActs, SIGNAL(triggered(QAction*)), target, SLOT(setSpace(QAction*)));

    m_resetViewAct->disconnect(SIGNAL(triggered(bool)));
    connect(m_resetViewAct, SIGNAL(triggered(bool)), target, SLOT(resetState()));
    m_continueModeAct->disconnect(SIGNAL(triggered(bool)));
    connect(m_continueModeAct, SIGNAL(triggered(bool)), target, SLOT(setContinuous(bool)));

}

/*
void MainWindow::changedActionsAct(QAction *act)
{
    // if the current action of the current view is scale, make sure the axis actions are disabled
    bool isScale = (m_currentGlWidget->currentMouseAction() == GLWidget::Scale);
    if (isScale != (!m_axisActs->isEnabled()))
    {
        m_axisActs->setEnabled(!isScale);
    }
}
*/

void MainWindow::connectActions()
{
    // main
    connect(m_exitAct, SIGNAL(triggered(bool)), this, SLOT(close()));
    // build
    connect(m_selectYellowAct, SIGNAL(triggered(bool)), m_buildGLControl, SLOT(setSelectYellow(bool)));
   // connect(m_selectBlueAct, SIGNAL(triggered(bool)), m_buildGLControl, SLOT(setUnSetBlueMode(bool)));
    //connect(m_showAllBlueAct, SIGNAL(triggered(bool)), m_buildGLControl, SLOT(showAllBlue()));

    // views
    connect(m_viewActs, SIGNAL(triggered(QAction*)), this, SLOT(setView(QAction*)));
    connect(m_picsWidget, SIGNAL(switchToDesign()), this, SLOT(switchToBuild_NoLast()));
    connect(m_docktab, SIGNAL(currentChanged(int)), this, SLOT(setView(int)));

    connect(m_helpUsage, SIGNAL(triggered(bool)), this, SLOT(doUsage()));
    connect(m_helpHomepage, SIGNAL(triggered(bool)), this, SLOT(doHomepage()));
    connect(m_aboutAct, SIGNAL(triggered(bool)), this, SLOT(doModalAboutDlg()));
    connect(m_aboutQtAct, SIGNAL(triggered(bool)), qApp, SLOT(aboutQt()));

    // document
    connect(this, SIGNAL(picsLoadComplete()), m_picsWidget, SLOT(continueCreate()));
    connect(m_doc, SIGNAL(updateViews(int)), m_modelGLControl, SLOT(updateView(int)));
    connect(m_doc, SIGNAL(updateViews(int)), m_buildGLControl, SLOT(updateView(int)));
    connect(m_doc, SIGNAL(updateViews(int)), m_picsWidget, SLOT(updateView(int)));
    connect(m_doc, SIGNAL(updateViews(int)), m_modelDlg, SLOT(updateView(int)));
    //connect(&PicBucket::instance(), SIGNAL(boundTexture(int, QImage)), m_modelGLControl, SLOT(doBindTexture(int, QImage)));
    //connect(&PicBucket::instance(), SIGNAL(updateTexture(int, QImage)), m_modelGLControl, SLOT(doUpdateTexture(int, QImage)));

    connect(m_doc, SIGNAL(newSolutionsLoaded(QString)), this, SLOT(setCurrentFile(QString)));
    connect(m_doc, SIGNAL(newShapeLoaded(QString)), this, SLOT(setCurrentFile(QString)));
    connect(m_doc, SIGNAL(newShapeLoaded(QString)), this, SLOT(switchToBuild_NoLast())); // solveNone not speific enough (to build)
    connect(m_doc, SIGNAL(fileSaved(QString)), this, SLOT(setCurrentFile(QString)));
    connect(m_doc, SIGNAL(changedFromSave(bool)), this, SLOT(setTitleChanged(bool)));

    connect(m_doc, SIGNAL(allReset()), this, SLOT(setTitleUntitled()));
    connect(m_doc, SIGNAL(allReset()), m_picsWidget, SLOT(changeToResetValues())); 
    connect(m_doc, SIGNAL(allReset()), this, SLOT(switchToBuild_NoLast())); // solveNone not speific enough (to build)
    connect(m_doc, SIGNAL(solveReset()), this, SLOT(setTitleUntitled()));

    connect(m_goAct, SIGNAL(triggered(bool)), m_doc, SLOT(solveGo()));
    connect(m_resetSolAct, SIGNAL(triggered(bool)), m_doc, SLOT(OnFileResetsolves()));
    connect(m_openAct, SIGNAL(triggered(bool)), m_doc, SLOT(OnFileOpen()));
    connect(m_saveAct, SIGNAL(triggered(bool)), m_doc, SLOT(OnFileSave()));
    connect(m_exportAct, SIGNAL(triggered(bool)), m_doc, SLOT(OnFileExport()));
    connect(m_newShapeAct, SIGNAL(triggered(bool)), m_doc, SLOT(OnNewBuild()));
    connect(this, SIGNAL(openFile(QString)), m_doc, SLOT(OnOpenDocument(QString)));
    
    connect(m_slvBrowse, SIGNAL(triggered(QAction*)), m_doc, SLOT(slvTrigger(QAction*)));
    
    connect(m_doc, SIGNAL(solvePopUp()), this, SLOT(switchToModel_NoLast()));
    connect(m_doc, SIGNAL(solvePopUp()), this, SLOT(updateFileEnables()));
    connect(m_doc, SIGNAL(solvePopUp()), m_grpColDlg, SLOT(update()));
    connect(m_doc, SIGNAL(solveIndexChanged(int)), this, SLOT(setActSlvIndex(int)));
    connect(m_doc, SIGNAL(solveIndexChanged(int)), this, SLOT(updateSolveBrowseEnable()));
    connect(m_doc, SIGNAL(solveIndexChanged(int)), m_grpColDlg, SLOT(update()));

 //   connect(m_doc, SIGNAL(solveNone()), this, SLOT(switchToBuildOrPics_NoLast())); // can come from go or save as well
    connect(m_doc, SIGNAL(solveNone()), this, SLOT(updateSolveBrowseEnable()));
    connect(m_doc, SIGNAL(solveNone()), this, SLOT(updateFileEnables()));

    connect(m_slvNumAct, SIGNAL(changed()), this, SLOT(setDocSlvIndex()));

    // SolveDlg
    connect(m_slvdlg, SIGNAL(go()), m_doc,  SLOT(solveGo()));
    connect(m_slvdlg, SIGNAL(resetSolves()), m_doc, SLOT(OnFileResetsolves()));
    connect(m_doc, SIGNAL(slvProgUpdated(int, int)), m_slvdlg, SLOT(statsUpdate(int, int)));
    connect(m_doc, SIGNAL(slvProgUpdated(int, int)), this, SLOT(slvProgUpdate(int, int)));
    connect(m_showSlvDlgAct, SIGNAL(triggered(bool)), m_slvdlg, SLOT(setVisible(bool)));
    connect(m_slvdlg, SIGNAL(visibilityChanged(bool)), m_showSlvDlgAct, SLOT(setChecked(bool)));

    // dialogs
    connect(m_editOptionsAct, SIGNAL(triggered()), this, SLOT(doModalOptionsDlg()));

    connect(m_buildDlg, SIGNAL(changedAction(bool)), m_buildGLControl, SLOT(changeAction(bool)));
    connect(m_buildGLControl, SIGNAL(changedAction(bool)), m_buildDlg, SLOT(changeAction(bool)));
    // TBD-changedTilesCount, should pass via CubeDoc (updateView and from BuildWorld)
    connect(m_buildGLControl, SIGNAL(changedTilesCount(int)), m_buildDlg, SLOT(setTilesCount(int)));
    connect(m_buildGLControl, SIGNAL(changedTilesCount(int)), m_picsWidget, SLOT(setBuildTilesCount(int)));
    connect(m_doc, SIGNAL(newShapeSize(int)), m_asmStepDlg, SLOT(setTilesCount(int)));
    connect(m_picsWidget, SIGNAL(changedPieceCount(int)), m_buildDlg, SLOT(setPieceCount(int)));
    connect(this, SIGNAL(picsLoadComplete()), m_buildDlg, SLOT(completePicsWidgets()));
    connect(m_buildDlg, SIGNAL(changedFamBox(int, int)), m_picsWidget, SLOT(changeFamBox(int, int)));
    connect(m_buildDlg, SIGNAL(switchToPicsTab(int)), this, SLOT(switchToPics_NoLast()));
    connect(m_buildDlg, SIGNAL(switchToPicsTab(int)), m_picsWidget, SLOT(changeTab(int)));
    connect(m_doc, SIGNAL(slvProgUpdated(int, int)), m_buildDlg, SLOT(statsUpdate(int, int)));
    connect(m_doc, SIGNAL(slvProgUpdated(int, int)), m_modelDlg, SLOT(statsUpdate(int, int)));
    connect(m_doc, SIGNAL(slvProgUpdated(int, int)), m_buildGLControl, SLOT(slvProgStatsUpdate(int, int)));
    connect(m_doc, SIGNAL(slvProgUpdated(int, int)), m_picsWidget, SLOT(slvProgStatsUpdate(int, int)));

   // connect(m_buildDlg, SIGNAL(zoomChanged(int)), m_glWidget, SLOT(externalZoom(int)));
   // connect(m_modelDlg, SIGNAL(zoomChanged(int)), m_glWidget, SLOT(externalZoom(int)));
    //connect(m_buildDlg, SIGNAL(zoomChanged(int)), m_buildGLControl, SLOT(changeRotAngle(int)));
    connect(m_modelDlg, SIGNAL(angleChanged(int)), m_buildGLControl, SLOT(changeRotAngle(int)));

    connect(m_buildGLControl, SIGNAL(zoomChanged(int)), m_buildDlg, SLOT(updateZoom(int)));
    connect(m_modelGLControl, SIGNAL(zoomChanged(int)), m_modelDlg, SLOT(updateZoom(int)));

    connect(m_modelGLControl, SIGNAL(changedHoverPiece(int)), m_modelDlg, SLOT(changeViewPiece(int)));
//    connect(m_modelGLControl, SIGNAL(rotated(GLWidget::EAxis, int, int)), m_modelDlg->getPieceView(), SLOT(externRotate(GLWidget::EAxis, int, int)));
//    connect(m_modelGLControl, SIGNAL(callReset()), m_modelDlg->getPieceView(), SLOT(resetState()));
    connect(m_modelGLControl, SIGNAL(chosenSinglePiece(int)), m_modelDlg, SLOT(changedExternViewPiece(int)));

    connect(m_buildGLControl, SIGNAL(changedTilesCount(int)), m_doc, SLOT(evaluateBstatus()));
    connect(m_picsWidget, SIGNAL(changedPieceCount(int)), m_doc, SLOT(evaluateBstatus()));

    connect(m_showGrpColorAct, SIGNAL(triggered(bool)), m_grpColDlg, SLOT(setVisible(bool)));
    connect(m_grpColDlg, SIGNAL(visibilityChanged(bool)), m_showGrpColorAct, SLOT(setChecked(bool)));

    connect(m_grpColDlg, SIGNAL(changed(int)), m_modelGLControl, SLOT(updateView(int)));
    connect(m_asmStepDlg, SIGNAL(stepChanged(int)), m_doc, SLOT(setShowUpToStep(int)));
    connect(m_showAsmDlgAct, SIGNAL(triggered(bool)), m_asmStepDlg, SLOT(setVisible(bool)));
    connect(m_asmStepDlg, SIGNAL(visibilityChanged(bool)), m_showAsmDlgAct, SLOT(setChecked(bool)));

    // status bar
    connect(m_buildGLControl, SIGNAL(changedTileHover(int, BuildGLControl::EActStatus)), this, SLOT(setStatusTextBuild(int, BuildGLControl::EActStatus)));

    connect(m_transferShapeAct, SIGNAL(triggered(bool)), m_doc, SLOT(transferShape()));

}

void MainWindow::setActSlvIndex(int n)
{
    SlvData data(n, m_slvNumAct->data().value<SlvData>().size);
    m_slvNumAct->setData(QVariant::fromValue(data)); // needed because setData is not a slot and takes a variant
}
void MainWindow::setDocSlvIndex()
{
    m_doc->slvSetIndex(m_slvNumAct->data().value<SlvData>().index);
}

void MainWindow::setStatusTextBuild(int t, BuildGLControl::EActStatus act)
{
    if (t == -1)
        statusBar()->clearMessage();
    else
    {
        switch (act)
        {
        case BuildGLControl::REMOVE:
            statusBar()->showMessage("Double click a tile to Remove a cube"); break;
        case BuildGLControl::CANT_REMOVE:
            statusBar()->showMessage("You Cannot remove the only cube in the design"); break;
        case BuildGLControl::ADD:
            statusBar()->showMessage("Double click a tile to Add a cube"); break;
        case BuildGLControl::EDIT_DISABLE:
            statusBar()->showMessage("Can't edit while trying to solve design."); break;
        }
    }
}

void MainWindow::doModalOptionsDlg()
{
    DisplayConf lastdc = m_doc->m_conf.disp;

    OptionsDlg dlg(this, &m_doc->m_conf);
    connect(&dlg, SIGNAL(updateSlv3D(int)), m_modelGLControl, SLOT(updateView(int)));

    if (dlg.exec() == QDialog::Accepted)
    {
        bool rend = lastdc.diffRender(m_doc->m_conf.disp);
        if (rend) // is a new render needed?
        {
            DlgProg dlgprog(true);
            PicBucket::mutableInstance().buildMeshes(m_doc->m_conf.disp, &dlgprog);
        }
        if (rend || lastdc.diffOnlyPaint(m_doc->m_conf.disp))
        {
        //    m_modelGLControl->setUsingLight(m_doc->m_conf.disp.bLight);
            m_glWidget->updateGL();
        //    m_modelDlg->getPieceView()->setUsingLight(m_doc->m_conf.disp.bLight);
        //    m_modelDlg->getPieceView()->updateGL();
        }
        if (lastdc.nSwapTexEndians != m_doc->m_conf.disp.nSwapTexEndians) // swap endians changed
        {
        //	QMessageBox::information(this, APP_NAME, "You've changed the flip texture color option.\nthis option will only take effect after you restart the application.", QMessageBox::Ok);
            // doesn't work for some reason. needs restart of the application. TBD.
            //PicBucket::mutableInstance().updateSwapEndian(m_doc->m_opts.disp.nSwapTexEndians);
        }
    }
}

/// Show the about dialog box
void MainWindow::doModalAboutDlg()
{
    QDialog dlg(this);
    Ui::AboutDlg ui;
    ui.setupUi(&dlg);
    // now fix it.
    dlg.setMinimumSize(dlg.size()); // prevent it from resizing
    dlg.setMaximumSize(dlg.size());

    QPalette pal;
    pal.setCurrentColorGroup(QPalette::Active);
    pal.setColor(QPalette::Base, pal.color(QPalette::Disabled, QPalette::Base));
    ui.emailEdit->setPalette(pal);
//	ui.icqEdit->setPalette(pal);

    // NOTE: how to make this gif
    // use camtasia to capture the screen
    // Use "Easy GIF Animator 5 (Karlis Blumentals) to set the transparency color for all frames
    QMovie anim(":/images/cube_ts.gif");
    anim.setCacheMode(QMovie::CacheAll); //needed for looping
    ui.movie->setMovie(&anim);
    anim.start();
    dlg.show();
    dlg.exec();
}

void MainWindow::doUsage()
{
    QDesktopServices::openUrl(QUrl("http://happysolver.sourceforge.net/usage/"));
}

void MainWindow::doHomepage()
{
    QDesktopServices::openUrl(QUrl("http://happysolver.sourceforge.net/"));
}


// called by the slots
void MainWindow::setCurrentFile(const QString &fileName, const QString &list, QAction *acts[])
{
    QString curFile = fileName;
    if (!curFile.isEmpty())
        m_filename = curFile;
    showTitle();

    QSettings settings("Happy Cube Solver", "Happy Cube Solver (QT)");
    QStringList files = settings.value(list).toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue(list, files);
    
    updateRecentActions(acts, files);
}

void MainWindow::setTitleUntitled()
{
    m_filename.clear();
    showTitle();
}

void MainWindow::setTitleChanged(bool state)
{
    m_bChanged = state;
    showTitle();
}

void MainWindow::showTitle()
{
    QString title;
    if (!m_filename.isEmpty())
        title = strippedName(m_filename) + " - ";
    title += APP_NAME + (m_bChanged?QString("*"):QString());
    setWindowTitle(title);
}


void MainWindow::openRecentFile() // SLOT handles both shapes and solves
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        emit openFile(action->data().toString());
}

// called in initialization of the menus
void MainWindow::readRecentFileActions(const QString &list, QAction *acts[])
{
    QSettings settings("Happy Cube Solver", "Happy Cube Solver (QT)");
    QStringList files = settings.value(list).toStringList();
    updateRecentActions(acts, files);
}

void MainWindow::updateRecentActions(QAction *acts[], const QStringList &files)
{
    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) 
    {
        QString text = tr("&%1 %2").arg(i + 1).arg(files[i]);
        acts[i]->setText(text);
        acts[i]->setData(files[i]);
        acts[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        acts[j]->setVisible(false);

    acts[MaxRecentFiles]->setVisible(numRecentFiles == 0);
}

/// the soultion browser (next, previous, free selection from a combobox)
/// is controlled by the actions in this method. They need to be changed
/// from here because they are accessed both by ModelHelpDlg and the solutions
/// toolbar
void MainWindow::updateSolveBrowseEnable()
{
    int num = m_doc->getSolvesNumber();
    if (num == 0)
    {
        m_slvBrowse->setEnabled(false);
        m_slvNumAct->setEnabled(false);
        return;
    }
    m_slvBrowse->setEnabled(true);
    m_slvNumAct->setEnabled(true);
    m_slvNumAct->setData(QVariant::fromValue(SlvData(m_slvNumAct->data().value<SlvData>().index, num)));

    int index = m_doc->getCurrentSolveIndex();

    m_slvFirstAct->setEnabled(index != 0);
    m_slvPrevAct->setEnabled(index != 0);
    m_slvNextAct->setEnabled(index != num - 1);
    m_slvLastAct->setEnabled(index != num - 1);
}

void MainWindow::updateFileEnables()
{ // anything that is disabled should have a damn good reason for it
  // we rely on the message box that will tell the user there's a need to stop the engine.
  // this is more intuitive to the user then just disabling the option.
    bool running = m_doc->isSlvEngineRunning();

    m_resetSolAct->setEnabled(m_doc->solvesExist());
    m_exportAct->setEnabled(m_doc->solvesExist());
    m_editOptionsAct->setEnabled(!running);

    // solutions tab in the dock
    if (m_doc->solvesExist())
    {
        m_docktab->setTabEnabled(1, true);
        m_docktab->setTabToolTip(1, "View solutions");
    }
    else
    { // QT bug - tooltip not shown.
        if (m_doc->isGoEnabled())
            m_docktab->setTabToolTip(1, "No solutions to view\nPress \"Solve It!\" to find solutions");
        else
            m_docktab->setTabToolTip(1, "No solutions to view\nResolve the error and press \"Solve It!\"");
   //     m_docktab->setTabEnabled(1, false); SINGLE-VIEW
    }
    
}

void MainWindow::slvProgUpdate(int hint, int data)
{
    switch (hint)
    {
    case SHINT_SOLUTIONS: // number of solutions changed, maybe we need to change the toolbar enables
        updateSolveBrowseEnable();
        break;
    case SHINT_START:
    case SHINT_STOP:
        updateFileEnables(); // for the options
        break;
    }

    // take care of the go action
    if (!m_doc->isSlvEngineRunning())
    {
        m_goAct->setText("&Solve It!");
        m_goAct->setEnabled(m_doc->isGoEnabled());
        if (m_doc->isGoEnabled())
            m_goAct->setToolTip("Find Solutions");
        else
            m_goAct->setToolTip("Can't find solutions\nYou need to resolve the error first");
    }
    else
    {
        m_goAct->setText("Stop!");
        m_goAct->setToolTip("Stop solutions engine");
    }
}



void MainWindow::EnableBuildActs(bool en)
{
    m_selectYellowAct->setEnabled(en);
    m_selectBlueAct->setEnabled(en);
    m_showAllBlueAct->setEnabled(en);
}

void MainWindow::Enable3DActions(bool en)
{
    m_resetViewAct->setEnabled(en);
    m_continueModeAct->setEnabled(en);
//    m_viewFrustrumActs->setEnabled(en);
//    m_axisActs->setEnabled(en);
//    m_actionsActs->setEnabled(en);
//    m_spaceActs->setEnabled(en);
}

void MainWindow::keyPressEvent(QKeyEvent *event) // capture the keys from all windows
{
    m_buildGLControl->keyEvent(event); // delegate to the build view
    m_modelGLControl->keyEvent(event);
    QMainWindow::keyPressEvent(event);
}
void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    m_buildGLControl->keyEvent(event);
    m_modelGLControl->keyEvent(event);
    QMainWindow::keyReleaseEvent(event);
}


void MainWindow::setView(QAction *act) 
{ 
    SwitchView(act->data().toInt(), true); 
}

void MainWindow::setView(int tab)
{ // the tab was changed
    if (tab == 0)
        switchToBuild_NoLast();
    else // tab == 1;
        switchToModel_NoLast();
}

void MainWindow::switchToBuildOrPics_NoLast()
{ 
    if (m_stack->currentIndex() == ModelView)
        switchTo_NoLast(BuildView); 
}

void MainWindow::switchTo_NoLast(EView view) 
{ 
    // this is a sort of a hack to make the QButtonGroup update to reality.
    QAction *va = selectGroupAct(m_viewActs, view);
    if (!va->isChecked())
        va->toggle(); // make the QAction selection correct
    SwitchView(view, false); 
}


void MainWindow::SwitchView(int nSwitchTo, bool bToLast)
{
    if (m_curView == nSwitchTo)
    {
        if ((bToLast) && (m_lastViewIndex != -1)) 
            m_curView = m_lastViewIndex;
        else 
            return; // do nothing, don't change it
    } 
    else {
        m_lastViewIndex = m_curView;
        m_curView = nSwitchTo;
    }

    // needed in order for the actions to depress
    QList<QAction*> acts = m_viewActs->actions();
    for(int i = 0; i < 3; ++i)
    {
        if (i != m_curView)
            acts[i]->setChecked(false);
        else
            acts[i]->setChecked(true);
    }


    //setUpdatesEnabled(false);
    //m_docktab->blockSignals(true);

    switch (m_curView)
    {
    case ModelView:
        m_docktab->setCurrentIndex(1);
        m_stack->setCurrentIndex(0);

        m_glWidget->switchHandler(m_modelGLControl);
        m_glWidget->updateGL();

        Enable3DActions(true);
        EnableBuildActs(false);
        break;

    case BuildView:
        m_docktab->setCurrentIndex(0);
        m_stack->setCurrentIndex(0);

        m_glWidget->switchHandler(m_buildGLControl);
        m_glWidget->updateGL();

        Enable3DActions(true);
        EnableBuildActs(true);
        break;

    case PicsView:
        m_docktab->setCurrentIndex(0); // pics selection wants the design dialog
        m_stack->setCurrentIndex(1);

        Enable3DActions(false);
        EnableBuildActs(false);
        break;
    }


}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
   if (e->mimeData()->hasFormat("text/uri-list")) 
        e->accept();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    if (!e->mimeData()->hasFormat("text/uri-list"))
        return;
    
    QList<QUrl> urls = e->mimeData()->urls();
    if (urls.size() > 0)
    { // take the first one
        QString fileName = urls[0].toLocalFile();
        if ((fileName.endsWith(".slv") || (fileName.endsWith(".shp"))))
        {// wait for something to finish?
            emit openFile(fileName);
        }
        else
        {
            QMessageBox::critical(this, APP_NAME,tr("Unknown file format:\n%1").arg(fileName));
        }
    }
}

