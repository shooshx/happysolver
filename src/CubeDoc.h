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

#ifndef __CUBE_DOC_H__INCLUDED__
#define __CUBE_DOC_H__INCLUDED__

#include "Configuration.h"
#include "BuildWorld.h"
#include "Solutions.h"
#include "SolveThread.h"
#include "CubeDocBase.h"

#include <QObject>
#include <QThread>

/** \file
    Declares the CubeDoc class which holds all of the current application data.
    Defines hint constants used in in the updateViews() and slvProgUpdated() signals
*/

class BuildWorld;
class SlvCube;
class QGLWidget;
class QAction;
class QComboBox;
class PicsSet;

// hint system of the updateView signal
#define HINT_SLV_NXPR    0x00000001  // need to paint the current solution
#define HINT_SLV_NONE    0x00000002  // need to paint the fact there is no solutions
#define HINT_SLV_READY   0x00000004 // need to do something for a new solution
#define HINT_SLV_PAINT   0x00000008 // need to only repaint

#define HINT_PIC_NULL         0x00003000
#define HINT_PIC_READSLVCHECK 0x00001000 // read the check data from the current solution
#define HINT_PIC_UPDATECHECK  0x00002000  // just update the gui according the the bucket data

#define HINT_BLD_MINMAX 0x01000000
#define HINT_BLD_PAINT  0x02000000
#define HINT_BLD_NEW    0x04000000

#define GET_SLV_HINT(hint) (hint & 0x00000FFF)
#define GET_PIC_HINT(hint) (hint & 0x00FFF000)
#define GET_BLD_HINT(hint) (hint & 0xFF000000)




class SolveThread;


/// this is a global pointer to the main MainWindow instance. it is used in places where
/// there is no easy reach to a parent window. 
/// This pointer is set in the main() and instantiated in main.cpp
extern QWidget* g_main;


/** CubeDoc centeralizes the data of the current design and solutions displayed in the application.
    The Happy Cube Solver generally implements an MFC inspired Document-View model.
    All of the data which defines the current "state" of the application is stored in this class.
    This includes the current design from the design view which is stored in m_build,
    the current active solutions which are stored in m_slvs and the Shape class which
    corresponds to these solutions which is stored in m_shp.
    The only exception is the current state of the pieces selection which is stored directly
    in the global PicBucket singleton.
    CubeDoc's job is to manage this data and keep it current and in sync. it does not deal
    at all with display issues which are implemented in the different widgets.
    CubeDoc is responsible of opening and saving shape and solution files and for starting up
    the solution engine.
    There is only one instance of CubeDoc throughout the live of the application. this instance
    is created by MainWindow. even though solutions and shape can be loaded, saved and reset,
    the CubeDoc instance is never destroyed or replaced. This is a slight deviation from
    the Document-View model but it makes things much easyer. specifically signal and slots
    connection.
    \see Shape Solutions BuildWorld
*/
class CubeDoc : public QObject, public CubeDocBase
{
    Q_OBJECT
public:
    CubeDoc(QWidget *parent);
    ~CubeDoc() {}

    bool InitializePics();
    void waitInitFinish();


    bool isSlvEngineRunning() { 
        return (m_sthread != nullptr) && (m_sthread->fRunning); 
    }
    const RunStats* getRunningStats();


    /// used by the close event in the main window
    /// \return true if we can move on. false if we need to stop what we're doing
    bool checkUnsaved(int types = DTSolutions | DTShape);

    enum EWarnMsg
    {
        WARN_NONE = 0,
        WARN_SHAPE_SIDES,
        WARN_SHAPE_NOT_CONNECT,
        WARN_SHAPE_NO_START,
        WARN_NOT_ENOUGH_PIECES,
        WARN_VERY_LITTLE_PIECES,
        WARN_FULL_ENUM_NO_SOLVES
    };

    bool isGoEnabled() const { return m_bGoEnabled; }
    EWarnMsg getWarning() const { return m_curWarning; }
    EWarnMsg getPicsWarning() const { return m_curPicsWarning; }

    void emitChangedFromSave(bool b) {
        emit changedFromSave(b);
    }

private:
    void OnNewDocument();

    bool callGenerate(Shape *shape, bool bSilent);
    bool checkWhileRunning(const char* msg);
    void easter();

    bool generalSaveFile(const char *defext, const char* selfilter, const char *filter, const char *title, QString &retname);

    void openAndHandle(QString name);


    // parameter is the possible answer to the question of what to do in the ungenned slv situation
    // can be either -1(show messagebox),Solutions or Shape
    bool realSave(int unGenSlvAnswer = -1);

private slots:
    void OnSolveReadyS(int slvind);

public slots:
    void OnOpenDocument(QString qname);
    void OnNewBuild();

    void OnFileResetsolves();
    void OnFileOpen();
    bool OnGenShape(); 
    void OnFileSave(); 
    void OnFileExport(); 

    void slvTrigger(QAction* act);
    void slvSetIndex(int ind);

    /// starts and stops the solution engine thread.
    void solveGo();
    /// stops the solve thread if its running. return only after it is stopped.
    void solveStop();

    void OnFullEnumNoSlv();
    void evaluateBstatus(); // evaluate build status, generate warnings and determine if Go is enabled
    void setShowUpToStep(int step) ;

signals:
    void updateViews(int hint); //emitted when the attached views need to update their graphics

    void newSolutionsLoaded(QString filename); // emitted when a new solutions file was just loaded, update recent files and title
    void newShapeLoaded(QString filename); // emitted when a new shape was just loaded, update recent files, title and view
    void fileSaved(QString filename); // emitted when the file was saved, update recent files (shp), title
    void allReset(); // emitted when the shape is resetted and with it the solutions. update title and view
    void solveReset(); // all solves are gone. update title (not view, if its on model, leave it blank)
    void solvePopUp(); // emitted when suddenly there are solutions, where there were none before.
    void solveIndexChanged(int ind); // updates the combo box (current solution index was changed)
    void solveNone(); // emitted when suddently there are NO solutions

    void slvProgUpdated(int hint, int data); // Solve Progress Updates. directed to the SolveDlg
    void changedFromSave(bool state); // sets the asterist in the title


private:

    class SlvDoneNotify : public IChangeNotify {
    public:
        SlvDoneNotify(CubeDoc *that) : m_that(that) {}
        void notify(bool b) {
            m_that->emitChangedFromSave(b);
        }
        CubeDoc *m_that;
    } m_slvDone;



    SolveThread *m_sthread;


    /// used in checkUnsaved()
    enum EDataType { DTSolutions = 0x01, DTShape = 0x02 };

    void OnSolveNone(int bldhint = 0); // called to set things after its known there are no solves
    void updateSlvCombo();

    bool m_bGoEnabled;
    EWarnMsg m_curWarning;
    EWarnMsg m_curPicsWarning; // the warning about pics. can be WARN_NONE, WARN_NOT_ENOUGH_PIECES,	WARN_VERY_LITTLE_PIECES,


};

inline QString strippedName(const QString &fullFileName)
{
    return fullFileName.section('/', -1); // qt makes all filenames with '/'
}


#endif
