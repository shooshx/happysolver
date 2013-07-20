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

//#include <QFileDialog>
#include <QMessageBox>

#include "GlobDefs.h"
#include "CubeDoc.h"
#include "Configuration.h"
#include "BuildWorld.h"
#include "MyFile.h"
#include "Shape.h"
#include "Pieces.h"
#include "Solutions.h"
#include "MainWindow.h"
#include "SolveThread.h"
#include "SIDlg.h"
#include "PicsSet.h"
#include "SlvCube.h"
#include "ObjExport.h"

#include <fstream>

QString extractExtension(QString &path)
{
	QString name = strippedName(path);
	int p = name.lastIndexOf('.');
	if (p == -1)
		return QString();
	return name.remove(0, p);

}




CubeDoc::CubeDoc(QWidget *parent)
  :QObject(parent), m_shp(NULL), m_slvs(NULL), m_curWarning(WARN_NONE), m_curPicsWarning(WARN_NONE), m_slvDone(this)
{
	m_build = new BuildWorld;
	m_slvs = new Solutions;
	m_slvs->changedFromSave = &m_slvDone;

	//connect(m_slvs, SIGNAL(changedFromSave(bool)), this, SIGNAL(changedFromSave(bool)));

	m_sthread = NULL;

	m_nCurSlv = -1;
	m_bGoEnabled = false;
	m_nUpToStep = -1;

	m_conf.fromRegistry();
}




// called when suddenly there are solutions
// - if solving, on the first solution

void CubeDoc::OnSolveReadyS(int slvind)
{
	Q_ASSERT(m_slvs->size() > slvind);
	m_nCurSlv = slvind; // re-set it from -1
	
	emit solveIndexChanged(m_nCurSlv);
	emit updateViews(HINT_SLV_READY | HINT_PIC_READSLVCHECK | 0);
	emit solvePopUp();
}


void CubeDoc::OnNewBuild() //SLOT
{
	if (!checkWhileRunning("Stop solving this design now?"))
		return;

	if (!checkUnsaved())
		return;

	delete m_shp;
	m_shp = NULL;

	m_slvs->clear();

	m_build->initializeNew(true); // boxed 
	m_nUpToStep = 6; // 6 pieces to a new box
	m_build->justChanged();
	m_build->justSave();

	OnSolveNone(HINT_BLD_NEW);
	emit slvProgUpdated(SHINT_ALL, 0);
	emit allReset();


}


void CubeDoc::OnFileResetsolves() // SLOT
{
	if (!solvesExist())
		return;

	if (!checkWhileRunning("I can't reset solutions while solving."))
		return;

	if (!checkUnsaved(DTSolutions))
		return;

	m_slvs->clear(m_shp->fcn);

	OnSolveNone();
	emit slvProgUpdated(SHINT_SOLUTIONS, 0);
	emit solveReset();
}

// returns true if we should continue, false if we should stop whatever is needed done
bool CubeDoc::checkWhileRunning(const char* msg)
{
	if (!isSlvEngineRunning())
		return true;

	int ret = QMessageBox::question(g_main, APP_NAME, tr(msg), "Stop solving", "Cancel", QString(), 0, 1);
	if (ret == 1)
		return false;

	solveStop();
	// let the stop reach everywhere (stop animation)
	qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

	return true;

	//TBD - after this its possible the solvedlg didn't get the stop signal yet. fixed it 4.1.1?
}

void CubeDoc::OnFileOpen() // SLOT
{
	// needs to be here, before the dialog box
	if (!checkWhileRunning("I can't load a new file while solving"))
		return;

	if (!checkUnsaved())
		return;

	QString name = QFileDialog::getOpenFileName(g_main, "Open File", QString(), "Solver File (*.slv *.shp);;All Files (*.*)");
	if (name.isNull())
		return;

	realOpen(name);
}


void CubeDoc::OnOpenDocument(QString name) //SLOT called for recent files
{
	if (!QFile::exists(name))
	{
		QString msg = QString("File does not exist:\n") + name;
		QMessageBox::critical(g_main, APP_NAME, msg, QMessageBox::Ok, 0);
		return;
	}

	// needs to be here, before the dialog, but after we know there's a file
	if (!checkWhileRunning("I can't open a new shape while solving"))
		return;

	if (!checkUnsaved())
		return;

	realOpen(name);	
}


bool CubeDoc::checkUnsaved(int types)
{ // return false if cancel, true if continue closing
	if ( ((types & DTSolutions) != 0) && solvesExist() && m_slvs->getChangedFromSaved() )
	{
		int ret = QMessageBox::question(g_main, APP_NAME, tr("There are unsaved solutions\n\nDo you wish to save them?"), 
			QMessageBox::Yes |  QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape);
		switch (ret)
		{
		case QMessageBox::Yes:		return realSave(DTSolutions);
		case QMessageBox::No:		return true;
		case QMessageBox::Cancel:	return false;
		}
	}
	
	if ( ((types & DTShape) != 0) && (m_build != NULL) && (m_build->getChangedFromSave()))
	{
		int ret = QMessageBox::question(g_main, APP_NAME, tr("The shape was not saved\n\nDo you wish to save it?"), 
			QMessageBox::Yes |  QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape);
		switch (ret)
		{
		case QMessageBox::Yes:		return realSave();
		case QMessageBox::No:		return true;
		case QMessageBox::Cancel:	return false;
		}
	}

	return true;
}


bool CubeDoc::callGenerate(Shape *shape, bool bSilent)
{
	EGenResult ret = shape->generate(m_build);
	if (ret == GEN_RESULT_OK)
		return true;
	else if (!bSilent)
	{
		switch (ret)
		{
		case GEN_RESULT_NO_START:
			QMessageBox::critical(g_main, APP_NAME, tr("Generation of shape failed.\nMake sure your shape has a yellow starting piece."), QMessageBox::Ok, 0); 
			break;
		case GEN_RESULT_NOT_CONNECT:
			QMessageBox::critical(g_main, APP_NAME, tr("Generation of shape failed.\nShape is made of several disconnected parts.\nThe shape must be a single volume or surface."), QMessageBox::Ok, 0); 
			break;
		case GEN_RESULT_ILLEGAL_SIDE:
			QMessageBox::critical(g_main, APP_NAME, tr("Generation of shape failed.\nIllegal side discovered."), QMessageBox::Ok, 0); 
			break;
		case GEN_RESULT_UNKNOWN:
			QMessageBox::critical(g_main, APP_NAME, tr("Generation of shape failed.\nUnknown error, this is a bug!!!"), QMessageBox::Ok, 0); 
			break;
		case GEN_RESULT_OK: // shut gcc up.
			break;
		}
	}
	return false;
}


bool CubeDoc::OnGenShape() // not called by GUI
{
	if (!checkWhileRunning("Cannot process shape while running"))
		return false;

	if (m_shp != NULL)
	{
		if (!checkUnsaved(DTSolutions))
			return false;
	}

	Shape *newshp = new Shape;
	bool ret = callGenerate(newshp, false);

	if (ret) 
	{
		m_build->justGen();

		delete m_shp;
		m_shp = newshp;

		m_slvs->clear(m_shp->fcn);
		m_nCurSlv = 0;

		OnSolveNone(HINT_BLD_PAINT);
		emit slvProgUpdated(SHINT_SOLUTIONS, 0);

		emit solveReset(); // resets the title of the main window

		return true;
	}
	else
	{
		delete newshp;
		newshp = NULL;
	}

	return false;

}


bool CubeDoc::generalSaveFile(const char *defext, const char *selfilter, const char *filter, const char *title, QString &retname)
{
	// we need to wrestle the QFileDialog in order to get default extension + overwrite check
	// this is becuase the static "short-hand" functions are the only way to activate
	// the native dialog and these functions don't support default extension and hence
	// their overwrite check is not what I want as well.
	while (1)
	{
		QString filterstr(selfilter);
		QString name = QFileDialog::getSaveFileName(g_main, title, QString(), filter, &filterstr, QFileDialog::DontConfirmOverwrite);
		if (name.isNull())
			break;

		if (extractExtension(name).isEmpty()) // no extension
		{
			name += ".";
			name += defext;
		}
		 
		if (QFile::exists(name))
		{	
			QString msg = "the file: " + name + "already exists.\nDo you want to replace it?";
			int ret = QMessageBox::warning(g_main, APP_NAME, msg, "    Yes, replace it    ", "No", QString(), 0, 1); // spaces due to QT bug - button too small
			if (ret == 1) // "No" was chosen
				continue;
		}

		retname = (const char*)name.toLatin1();
		return true;
	}
	return false;
}


void CubeDoc::OnFileSave()
{
	realSave();
}

void CubeDoc::OnFileExport()
{
	QString retname;
	if (!generalSaveFile("obj", "Alias Wavefront Object (*.obj)", 
		"Alias Wavefront Object (*.obj)", "Export Solution", 
		retname))
	{
		return; // pressed cancel
	}

	QFileInfo fi(retname);
	QString matFileName = fi.path() + "/" + fi.baseName() + ".mtl";

	ofstream wrfl(retname.toLatin1().data()), matFile(matFileName.toLatin1().data());

	if (!wrfl.good())
	{
		QMessageBox::critical(g_main, APP_NAME, tr("Error opening file for writing\nCheck that the file isn't locked by another program and try again"), QMessageBox::Ok, 0);
		return;
	}
		
	if (!matFile.good())
	{
		QMessageBox::critical(g_main, APP_NAME, tr("Error opening file for writing\nCheck that the file isn't locked by another program and try again"), QMessageBox::Ok, 0);
		return;
	}

	QFileInfo mfi(matFileName);
	matFileName = mfi.fileName();

	const SlvCube* pSolution = getCurrentSolve();
	if (pSolution == NULL)
	{
		QMessageBox::critical(g_main, APP_NAME, tr("Error exporting current solution\nThere's no current solution"), QMessageBox::Ok, 0);
		return;
	}

	wrfl << "mtllib " << matFileName.toLatin1().data() << "\n";
	ObjExport oe(wrfl, &matFile);

	if (!pSolution->painter.exportToObj(oe))
	{
		QMessageBox::critical(g_main, APP_NAME, tr("Error while exporting current solution"), QMessageBox::Ok, 0);
		return;
	}

	wrfl.close();
	matFile.close();
}


bool CubeDoc::realSave(int unGenSlvAnswer)
{
	bool hasSolves = solvesExist(); // sample it here once, do we actually have any solves?
	bool doUngenerate = false; 
	// in case the current buildWorld is not the shape world and we save the solutions, 
	// we need to recostruct it
	bool saveCurShape = true;
	// in case the current shape isn't the shape of the buildworld we're saving,
	// just don't save it.

	if (hasSolves)
	{
		if (!checkWhileRunning("Cannot save solutions while running."))
			return false;

		if (m_build->getChangedFromGen()) // the build is different from the shape
		{ // we have solves but they are not the solves of this shape
			int ans;
			if (unGenSlvAnswer == -1)
			{
				ans = QMessageBox::question(g_main, APP_NAME, "The solutions I have do not match the design and can't be save together to one file.\nWould you like to save the solutions or the design?", 
				"   Save Solutions   ", "Save Design", "Cancel", 0, 2); // spaces due to QT bug - button too small
			}
			else
			{
				ans = (unGenSlvAnswer == DTSolutions)?(0):(1);
			}
			if (ans == 0) // solutions
				doUngenerate = true;
			else if (ans == 1) // shape
				saveCurShape = false; // not saving solves is implicit
			else if (ans == 2)
				return false;
		}
	}

	// what to choose? shp or slv?
	// don't really ask the user. in most case he won't notice
	// just choose for him. if there are solutions, save as slv, if none, save as shp
	// if the user changes is, then belive him he knows what he's doing...
	QString retname;
	if (!generalSaveFile(hasSolves?"slv":"shp", hasSolves?"Solution Files (*.slv)":"Shape Files (*.shp)", 
		"Shape Files (*.shp);;Solution Files (*.slv);;All Files (*.*)", hasSolves?"Save Solutions":"Save Shape", 
		retname))
	{
		return false; // pressed cancel
	}

	MyFile wrfl;
	wrfl.openWrite(retname.toLatin1());

	if (wrfl.getState() != STATE_OPEN_WRITE)
	{
		QMessageBox::critical(g_main, APP_NAME, tr("Error opening file for writing\nCheck that the file isn't locked by another program and try again"), QMessageBox::Ok, 0);
		wrfl.close();
		return false;
	}

	BuildWorld *savebuild = m_build;
	if (doUngenerate)
	{
		savebuild = new BuildWorld;
		savebuild->unGenerate(m_shp);
	}
	if (!savebuild->saveTo(&wrfl))
	{
		QMessageBox::critical(g_main, APP_NAME, tr("Error saving generate data"), QMessageBox::Ok, 0);
		if (doUngenerate)
			delete savebuild;
		wrfl.close();
		return false;
	}
	wrfl.writeNums(0, true); // write CRLF

	if (doUngenerate)
		delete savebuild;

	if (saveCurShape)
	{
		if ((!hasSolves) && ((m_shp == NULL) || (m_build->getChangedFromGen())))
		{ // no solves - not the unGenerate case. if we have solves, don't generate and erase them.
			Shape tmpshp;
			if (callGenerate(&tmpshp, true)) // ok to try a generate
			{
				OnGenShape(); // TBD: use return value?
				Q_ASSERT(m_shp != NULL); // we just checked it earlyer.
			}
		}

		if (m_shp != NULL) // the shape has something.
		{
			if (!m_shp->saveTo(&wrfl))
			{
				QMessageBox::critical(g_main, APP_NAME, tr("Error saving shape data."), QMessageBox::Ok, 0);
				wrfl.close();
				return false;
			}
			wrfl.writeNums(0, true); // CRLF

			if (hasSolves)
			{
				if (!m_slvs->saveTo(&wrfl, NULL)) //bla! FIXME
				{
					QMessageBox::critical(g_main, APP_NAME, tr("Error saving solutions."), QMessageBox::Ok, 0);
					wrfl.close();
					return false;
				}
			}
		}
	}

	m_build->justSave();
	wrfl.close();

	emit fileSaved(retname);
	emit updateViews(HINT_BLD_PAINT | HINT_PIC_NULL); 
	// should repaint the build view to get rid of the '*'
	// this is only needed if we didn't generated the shape. we can't be sure.

	return true;
}


void CubeDoc::setShowUpToStep(int step) 
{ 
	if (step == m_nUpToStep)
		return;
	m_nUpToStep = step; 
	emit updateViews(HINT_BLD_PAINT | HINT_SLV_PAINT | HINT_PIC_NULL); 
}


// TBD - check messages.
void CubeDoc::realOpen(QString name)
{
	MyFile rdfl;
	rdfl.openRead(name.toLatin1());
	bool hasSolutions = false;
	
	BuildWorld *newbuild = new BuildWorld;
	
	if (!newbuild->loadFrom(&rdfl))
	{ // no build, we can't do nothing.
		delete newbuild; newbuild = NULL;
		rdfl.close();
		QString msg = QString("Error opening solution file.\nfile: %1\n\nUnable to read shape information").arg(name);
		QMessageBox::critical(g_main, APP_NAME, msg, QMessageBox::Ok, 0);
		return;
	}

	// generate the shape from the build. this is the shape we're going to use in the end
	Shape *gendshape = new Shape;
	gendshape->generate(newbuild); // get the return value? no real need to.

	Solutions *newslvs = NULL;	
	Shape *loadedshp = new Shape;

	if (!loadedshp->loadFrom(&rdfl))
	{ // no shape (hence no solutions) but there is a build
		delete loadedshp; 
		loadedshp = NULL; // get rid of it, its no good.
		// newbuild - no call to justGen (stay with ctor values)

	}
	else // there is a shape loaded (you can't have solutions without a shape in the file.)
	{	
		// don't use the shape just loaded, instead, generate the build, and draw a transform
		// from the loaded shape for the genereted one.
		// TBD MOVE THIS. only needed if there are solutions.
		TTransformVec movedTo(loadedshp->fcn);
		bool trivialTransform = false;
		if (!loadedshp->createTrasformTo(gendshape, movedTo, &trivialTransform))
		{
			QMessageBox::critical(g_main, APP_NAME, "failed shape transform, bug.", QMessageBox::Ok, 0);
			return;
		}

		delete loadedshp; // we're done with it.
		loadedshp = NULL;

		newbuild->justGen(); // shouldn't gen it next time.

		newslvs = new Solutions(gendshape->fcn);
		if (!newslvs->readFrom(&rdfl, gendshape))
		{
			delete newslvs; newslvs = NULL;
		}
		else
		{ // has solutions
			if (newslvs->slvsz != gendshape->fcn)
			{ // solutions has errors
				QString msg = QString("Error opening solution file.\nfile: %1\n\nFile contains shape faces inconsistency\nSolutions not loaded").arg(name);
				QMessageBox::critical(g_main, APP_NAME, msg, QMessageBox::Ok, 0);
				delete newslvs; newslvs = NULL;
			}
			else
			{
				if (!trivialTransform)
					newslvs->transform(movedTo);
				hasSolutions = true;
			}
		}
	}

	// don't do anything before all the gui is done (messageboxes etc')
	// because a paint to the build would reveal something half baked.

	delete m_build;	
	m_build = newbuild;

	delete m_shp; 
	m_shp = gendshape;

	// directly initialize the step indicator now.
	m_nUpToStep = m_shp->fcn;

	// what to do with solutions?
	if (hasSolutions)
	{
		delete m_slvs;

		m_slvs = newslvs;
		m_slvs->changedFromSave = &m_slvDone;
		//connect(m_slvs, SIGNAL(changedFromSave(bool)), this, SIGNAL(changedFromSave(bool))); // reconnect
 
		// announce there is a new solution(s)
		// tell the main frame to switch to the model view... (this will happen becuase there were no solutions before)
		m_nCurSlv = 0; // re-set it from -1

		g_main->setUpdatesEnabled(false); // TBD: should use proper signal
		// updates need to be disabled to avoid the flicker caused by the DoReset that comes with
		// the Model view reset in updateView

		emit solvePopUp(); 
		// needs to be first since it causes a move to modelview
		// without it the text doesn't work for some reason.
		
		emit updateViews(HINT_SLV_READY | HINT_PIC_READSLVCHECK | HINT_BLD_NEW);
		g_main->setUpdatesEnabled(true);

		emit solveIndexChanged(m_nCurSlv);

		// update solves dialog
		emit slvProgUpdated(SHINT_SOLUTIONS, -1);
		// add to recent files
		emit newSolutionsLoaded(name);
	}
	else // an open without solutions
	{
		if (m_shp != NULL)
			m_slvs->clear(m_shp->fcn);
		else
			m_slvs->clear(); // size will get set when generated

		m_nCurSlv = 0;

		OnSolveNone(HINT_BLD_NEW);
		emit slvProgUpdated(SHINT_ALL, 0);
		emit newShapeLoaded(name);

	}


	rdfl.close();
}



// a call to this method should be follewed by-
// emit of slvProgUpdated
// emit something that will bring us back to buildview - refactor to solveNone() ??
// TBD - possibly factor these in here.
void CubeDoc::OnSolveNone(int bldhint)
{
	m_nCurSlv = -1;

	// resets the combo box to nothing
	emit solveNone();
	updateViews(HINT_SLV_NONE | HINT_PIC_READSLVCHECK | bldhint);

}


////// solution viewer ////////////////////////////////

void CubeDoc::slvTrigger(QAction* act)
{
	if (m_nCurSlv == -1)
		return;

	switch (act->data().toInt())
	{
	case MainWindow::FirstSolve:
		if (m_nCurSlv == 0) // already there
			return;
		m_nCurSlv = 0;
		break;

	case MainWindow::PrevSolve:
		if (m_nCurSlv == 0) // nowhere to go
			return;
		--m_nCurSlv;
		break;

	case MainWindow::NextSolve:
		if (m_nCurSlv == m_slvs->size() - 1) // nowhere to go
			return;
		++m_nCurSlv;
		break;

	case MainWindow::LastSolve:
		if (m_nCurSlv == m_slvs->size() - 1) // already there
			return;
		m_nCurSlv = m_slvs->size() - 1;
		break;
	}

	emit solveIndexChanged(m_nCurSlv);
	emit updateViews(HINT_SLV_NXPR | HINT_PIC_READSLVCHECK);
}

void CubeDoc::slvSetIndex(int ind)
{
	if (m_nCurSlv == -1) // there are no solves so what am I doing here?
		return;
	if ((ind < 0) || (ind >= m_slvs->size()))
		return;
	if (m_nCurSlv == ind)
		return;

	m_nCurSlv = ind;

	emit solveIndexChanged(m_nCurSlv);
	emit updateViews(HINT_SLV_NXPR | HINT_PIC_READSLVCHECK);
}

void CubeDoc::easter()
{
	SIDlg dlg(g_main);
	dlg.exec();
}

/////////////////// solution running ///////////////////

const RunStats* CubeDoc::getRunningStats()
{ 
	static RunStats dummy;
	if (m_sthread == NULL)
		return &dummy;
	return &m_sthread->m_stats; 
}


void CubeDoc::solveGo()
{
	if (isSlvEngineRunning())	{
		m_sthread->fExitnow = TRUE;
		return;
	}

	printf("solveGo!\n");
	if ((m_shp == NULL) || (m_build->getChangedFromGen()))
	{
		if (!OnGenShape())
			return;
	}

	// now build the Pics. it's important that the PicsSet will be built in this thread
	// since it involves the bucket.
	// here, the snapshot of the bucket piece selection
	// for any solutions to be produced in this run
	// the snapshot of the EngineConf is captured in Cube::Cube()
	// in the thread itself. (is that too late?)
	PicsSet *pics = new PicsSet(m_conf.engine.nAsym != ASYM_REGULAR);
	if (pics->added.size() < m_shp->fcn)
	{
		QMessageBox::critical(g_main, APP_NAME, tr("Unable to complay, too few pieces for this shape"), QMessageBox::Ok, 0);
		return;
	}

	if (pics->added.size() == m_shp->fcn + 42 + 1) // 42 is too easy to come by.
		easter();

	// we're all good to go!
		
	if (m_sthread == NULL)
	{
		m_sthread = new SolveThread;
			
		connect(m_sthread, SIGNAL(slvProgUpdated(int, int)), this, SIGNAL(slvProgUpdated(int, int)));
		connect(m_sthread, SIGNAL(solvePopUp(int)), this, SLOT(OnSolveReadyS(int)));
		connect(m_sthread, SIGNAL(fullEnumNoSlv()), this, SLOT(OnFullEnumNoSlv()));
	}
	m_sthread->fExitnow = FALSE;
	m_sthread->setRuntime(m_slvs, m_shp, pics, &m_conf.engine);
	m_sthread->start();

}

void CubeDoc::solveStop()
{
	if (!isSlvEngineRunning())
		return;

	m_sthread->fExitnow = TRUE;
	m_sthread->wait();
}

void CubeDoc::OnFullEnumNoSlv()
{
	m_curWarning = WARN_FULL_ENUM_NO_SOLVES;
	m_bGoEnabled = false;
	emit slvProgUpdated(SHINT_WARNING, -1); // a temporary message, till the next evaluateBstatus is called
}

// evaluate warning, solving enable status
void CubeDoc::evaluateBstatus()
{
	// isRunning? TBD

	m_bGoEnabled = true;
	m_curWarning = WARN_NONE;
	m_curPicsWarning = WARN_NONE;

	if (PicBucket::instance().selectedCount() < m_build->nFaces)
	{
		m_bGoEnabled = false; // this will be false no matter what
		m_curPicsWarning = WARN_NOT_ENOUGH_PIECES;
	}
	else if ((PicBucket::instance().selectedCount() < (1.2 * m_build->nFaces)) && (m_build->nFaces > 10))
	{ // less the 20% spare, a finger number
		m_curPicsWarning = WARN_VERY_LITTLE_PIECES;
	}


	EGenResult gres = m_build->testShape();
	if (gres != GEN_RESULT_OK)
	{
		m_bGoEnabled = false;
		if (gres == GEN_RESULT_ILLEGAL_SIDE)
		{
			m_curWarning = WARN_SHAPE_SIDES;
		}
		else if (gres == GEN_RESULT_NOT_CONNECT)
		{
			m_curWarning = WARN_SHAPE_NOT_CONNECT;
		}
		else if (gres == GEN_RESULT_NO_START)
		{
			m_curWarning = WARN_SHAPE_NO_START;
		}
	}
	else 
	{
		m_curWarning = m_curPicsWarning;
	}
	

	emit slvProgUpdated(SHINT_WARNING, -1);
}
