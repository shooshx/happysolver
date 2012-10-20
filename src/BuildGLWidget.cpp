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
#include <QMouseEvent>
#include <QMessageBox>
#include <QTimer>
#include <set>

#include "BuildGLWidget.h"

#include "BuildWorld.h"
#include "CubeDoc.h"



#define MAKE_NAME(dim, page, x, y)  (((dim) & 0x3) | (((x) & 0x7F)<<2) | (((y) & 0x7F)<<9) | (((page) & 0xFF)<<16))
#define GET_DIM(name) ((name) & 0x3)
#define GET_X(name) (((name) >> 2) & 0x7F)
#define GET_Y(name) (((name) >> 9) & 0x7F)
#define GET_PAGE(name) (((name) >> 16) & 0xFF)


BuildGLWidget::BuildGLWidget(QWidget *parent, CubeDoc *document)
	:GLWidget(parent), m_doc(document),
	 m_bEditEnabled(true),
	 m_fSetStrtMode(false), m_fUnSetBlueMode(false), 
	 m_bBoxedMode(true), // TBD-hardcoded
	 m_bInternalBoxRemove(false), m_bBoxRemove(false), 
	 m_bLastBoxRemove(false),
	 m_lastChoise(-1), 
	 m_nMarkedTiles(0), m_fadeTimer(NULL), 
	 m_errCylindrAlpha(0.0), m_errCylindrAlphaDt(0.0),
	 m_bDoneUpdate(false) 
{
	m_bBackFaceCulling = false; 

	if (m_bBoxedMode)
	{
		setMouseTracking(true); // causes mouse move events
		setFocusPolicy(Qt::StrongFocus); // receive the ctrl key press

		m_fadeTimer = new QTimer(this);
		connect(m_fadeTimer, SIGNAL(timeout()), this, SLOT(fadeTimeout()));
	}

	m_viewState = GLWidget::Perspective;

	const_cast<int &>(m_minScaleReset) = 6;

	reCalcBldMinMax();
	checkSides(); // build the initial test shape.

	makeBuffers();
}

void BuildGLWidget::initialized()
{
	m_prog.init();
}


void BuildGLWidget::showAllBlue() 
{ 
	m_doc->getBuild().search(SHOW_DONT, SHOW_DO, true, false);
	updateGL();
}


void BuildGLWidget::myPaintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawTargets(false);

	QString str1("%1tiles: %2");
	QString str2;


	if (m_doc->getBuild().getChangedFromSave())
		str1 = str1.arg("*"); //add the * if the shape was changed from the last save
	else
		str1 = str1.arg("");

	str1 = str1.arg(m_doc->getBuild().nFaces);

	GL_BEGIN_TEXT();

	glColor3f(0.3f, 0.3f, 1.0f);
	glRasterPos2f(-0.95f, -0.92f);

	mglPrint(str1);

	if (!m_bBoxedMode)
	{
		if (m_doc->getBuild().fClosed)
		{
			str2 = "           Closed Shape%1";
			glColor3f(0.0f, 0.0f, 1.0f);
		}
		else
		{
			str2 = "           Opened Shape%1";
			glColor3f(1.0f, 0.2f, 0.2f);
		}
		if (m_doc->getBuild().getChangedFromGen())
			str2 = str2.arg("+"); //add the + if the shape was changed from the last generate
		else
			str2 = str2.arg("");

		glRasterPos2f(-0.95f, -0.92f);
		mglPrint(str2);
	}

	GL_END_TEXT();

	m_bDoneUpdate = true;

}

class QuadAdder
{
public:
	QuadAdder(Mesh& mesh) : m_mesh(mesh) {
		m_mesh.m_type = GL_QUADS;
		m_mesh.m_hasColors = true;
		m_mesh.m_hasNames = true;
		m_mesh.m_hasIdx = false;
		m_mesh.clear();
	}

	void add(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, const Vec4& color, uint name)
	{
		m_mesh.m_vtx.push_back(a);   m_mesh.m_vtx.push_back(b);   m_mesh.m_vtx.push_back(c);   m_mesh.m_vtx.push_back(d);
		Vec3b nv = Vec3b::fromName(name);
		m_mesh.m_name.push_back(nv); m_mesh.m_name.push_back(nv); m_mesh.m_name.push_back(nv); m_mesh.m_name.push_back(nv);
		m_mesh.m_color4.push_back(color); m_mesh.m_color4.push_back(color);  m_mesh.m_color4.push_back(color); m_mesh.m_color4.push_back(color);
	}

private:
	Mesh& m_mesh;

};

class LineAdder 
{
public:
	LineAdder(Mesh& mesh) : m_mesh(mesh), m_rep(&mesh.m_vtx) {
		m_mesh.m_type = GL_LINES;
		m_mesh.m_hasColors = false;
		m_mesh.m_hasNames = true;
		m_mesh.m_hasIdx = true;
		m_mesh.m_uniformColor = true;
		m_mesh.clear();
	}

	struct SortedPair {
		SortedPair(int _a, int _b) :a(_a), b(_b) {
			if (b < a)
				swap(a, b);
		}
		bool operator<(const SortedPair& o) const {
			if (a == o.a)
				return b < o.b;
			return a < o.a;
		}
		int a, b;
	};

	void addPair(int a, int b) {
		SortedPair s(a, b);
		auto it = m_added.find(s);
		if (it != m_added.end())
			return;
		m_mesh.m_idx.push_back(a);
		m_mesh.m_idx.push_back(b);
		m_added.insert(s);
	}

	void add(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, const Vec4& color)
	{
		int ia, ib, ic, id;
		m_rep.add(a, &ia);
		m_rep.add(b, &ib);
		m_rep.add(c, &ic);
		m_rep.add(d, &id);
		addPair(ia, ib);
		addPair(ib, ic);
		addPair(ic, id);
		addPair(id, ia);
		m_mesh.m_uColor = color;
	}

private:
	Mesh& m_mesh;
	VecRep m_rep;
	set<SortedPair> m_added;
};


void BuildGLWidget::makeBuffers() 
{
	const BuildWorld &build = m_doc->getBuild();
	QuadAdder realTiles(m_realTiles);
	QuadAdder transTiles(m_transTiles);
	LineAdder realLines(m_realLines);
	LineAdder transLines(m_transLines);

	for(int dim = 0; dim < 3; ++dim)
	{
		const SqrLimits &lim = build.m_limits[dim];
		for(int page = lim.minpage; page < lim.maxpage; ++page)
		{
			for(int x = lim.minx; x < lim.maxx ; ++x)
			{
				for(int y = lim.miny; y < lim.maxy; ++y)
				{
					int theget = build.get(dim, page, x, y);
					if (GET_VAL(theget) == 0)
						continue;
					uint name = MAKE_NAME(dim, page, x, y);

					Vec3 a,b,c,d;
					switch (dim)
					{
					case YZ_PLANE:
						a = Vec3(page, x, y);
						b = Vec3(page, x + 1, y);
						c = Vec3(page, x + 1, y + 1);
						d = Vec3(page, x, y + 1);
						break;
					case XZ_PLANE:
						a = Vec3(x, page, y);
						b = Vec3(x + 1, page, y );
						c = Vec3(x + 1, page, y + 1);
						d = Vec3(x, page, y + 1);
						break;
					case XY_PLANE:
						a = Vec3(x, y, page);
						b = Vec3(x + 1, y, page);
						c = Vec3(x + 1, y + 1, page);
						d = Vec3(x, y + 1, page);
						break;
					}	

					int valshow = GET_VAL_SHOW(theget);
					Vec4 color;

					if (GET_TYPE(theget) == TYPE_VIR)
					{
						color = Vec4(0.0f, 0.0f, 0.8f, 0.5f);
						transTiles.add(a, b, c, d, color, name);
						transLines.add(a, b, c, d, Vec4(0.2f, 0.2f, 1.0f, 0.5f));
					}
					else
					{
						if (valshow == FACE_NORM_SELR)
							color = Vec4(1.0f, 1.0f - 0.25f*1.0f, 1.0f - 0.25f*1.0f, 1.0f);
						else
							color = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
						realTiles.add(a, b, c, d, color, name);
						realLines.add(a, b, c, d, Vec4(0.2f, 0.2f, 0.2f, 1.0f));
					}
				}
			}
		}
	}
}
			


#define ERR_CYLINDER_RADIOUS 0.1

void BuildGLWidget::drawErrorCyliders()
{
	GLUquadricObj* qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, GLU_FILL);
	const Shape &tstshp = m_doc->getBuild().getTestShape();

	for (int i = 0; i < tstshp.sdnError; ++i)
	{
		Shape::SideDef &sd = tstshp.errorSides[i];
		float x = sd.ex.x / 4.0,y = sd.ex.y / 4.0, z = sd.ex.z / 4.0;

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glPushMatrix();

		switch (sd.dr)
		{
		case X_AXIS: glTranslatef(x - 0.1, y, z); glRotatef(90, 0, 1, 0);  break;
		case Y_AXIS: glTranslatef(x, y + 1.1, z); glRotatef(90, 1, 0, 0);  break;
		case Z_AXIS: glTranslatef(x, y, z - 0.1); break;
		}

		glColor4f(1.0, 0.0, 0.0, m_errCylindrAlpha);

		gluCylinder(qobj, ERR_CYLINDER_RADIOUS, ERR_CYLINDER_RADIOUS, 1.2, 15, 2); // the cylinder
		gluDisk(qobj, 0, ERR_CYLINDER_RADIOUS, 15, 1); // two disks to cap it
		glTranslatef(0, 0, 1.2f); 
		gluDisk(qobj, 0, ERR_CYLINDER_RADIOUS, 15, 1);

		glPopMatrix();
	}

	gluDeleteQuadric(qobj);
}


void BuildGLWidget::drawTargets(bool inChoise)
{
	float m[16], p[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	glGetFloatv(GL_PROJECTION_MATRIX, p);

	auto tm = model.cur(), tp = proj.cur();
	ProgramUser use(&m_prog);
	m_prog.trans.set(transformMat());



	glDisable(GL_TEXTURE_2D);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_POLYGON_OFFSET_FILL);

	glPolygonOffset(1.0, 1.0);
	m_realTiles.paint(inChoise);

	if (!inChoise) {
		glPolygonOffset(0, 0);
		m_realLines.paint();

		glPolygonOffset(1.0, 1.0);
		m_transTiles.paint();
		glPolygonOffset(0, 0);
		m_transLines.paint();

	}
}


#if 0



void BuildGLWidget::drawTargetsPart(bool fTrans, bool fLines)
{
	int dim,x, y, page, thisname;

	const BuildWorld &build = m_doc->getBuild();
	int upToStep = m_doc->getUpToStep(); // optimization

	for(dim = 0; dim < 3; ++dim)
	{
		const SqrLimits &lim = build.m_limits[dim];
		for(page = lim.minpage; page < lim.maxpage; ++page)
		{
			for(x = lim.minx; x < lim.maxx ; ++x)
			{
				for(y = lim.miny; y < lim.maxy; ++y)
				{
					int theget = build.get(dim, page, x, y);
					if ((GET_VAL(theget) != 0) && (GET_VAL(theget) != FACE_DONT_TRANS) && (GET_SHOW(theget) != SHOW_DONT) &&
						((!fTrans && GET_TYPE(theget) != TYPE_VIR) || 
						(fTrans && GET_TYPE(theget) == TYPE_VIR)) )
					{
						int valshow = GET_VAL_SHOW(theget);

						// should we even check for up-to-step?
						if ((upToStep >= 0) && ((valshow == FACE_NORM) || (valshow == FACE_STRT)))
						{ // TBD: this is rather inefficient! fix it.
							int curind = build.getTestShapeFcInd(CoordBuild(dim, page, x, y));
							if ((curind == -1) || (curind >= upToStep))
								continue;
						}

						thisname = MAKE_NAME(dim, page, x, y);
						glLoadName(thisname);
						glBegin(GL_QUADS);
						if (!fLines)
						{
							float inten = GET_INTENSITY(theget)/8.0f;

							switch (valshow)
							{
							case FACE_STRT:	if (m_fSetStrtMode) { glColor3f(1.0f, 1.0f, 0.0f); break;}
							case FACE_NORM:	glColor3f(1.0f, 1.0f, 1.0f); break;
							case FACE_STRT_SELR: if (m_fSetStrtMode) { glColor3f(1.0f, 1.0f - 0.3f*inten, 0.0f + 0.3f*inten); break; }
							case FACE_NORM_SELR: glColor3f(1.0f, 1.0f - 0.25f*inten, 1.0f - 0.25f*inten); break;
							case FACE_TRANS_SEL: glColor4f(0.0f, 0.0f, 0.8f, inten*0.5f); break;
							case FACE_TRANS: glColor4f(0.0f, 0.0f, 0.8f, 0.5f); break;
							case FACE_DONT_TRANS: glColor4f(0.0f, 0.8f, 0.0f, 0.5f); break; // green, error
							default: glColor3f(0.5f, 0.5f, 0.5f);
							}
						}
						else
						{
							switch (valshow)
							{
							case FACE_NORM:
							case FACE_STRT:	glColor3f(0.2f, 0.2f, 0.2f); break;
							case FACE_STRT_SELR:
							case FACE_NORM_SELR: glColor3f(0.5f, 0.0f, 0.0f); break;
							case FACE_TRANS_SEL:
							case FACE_TRANS: glColor4f(0.2f, 0.2f, 1.0f, 0.5f);	break;
							case FACE_DONT_TRANS: glColor4f(0.2f, 1.0f, 0.2f, 0.5f); break; // green, error
							default: glColor3f(0.5f, 0.5f, 0.5f);
							}
						}


						switch (dim)
						{
						case YZ_PLANE:
							glVertex3i(page, x, y);
							glVertex3i(page, x + 1, y);
							glVertex3i(page, x + 1, y + 1);
							glVertex3i(page, x, y + 1);
							break;
						case XZ_PLANE:
							glVertex3i(x, page, y);
							glVertex3i(x + 1, page, y );
							glVertex3i(x + 1, page, y + 1);
							glVertex3i(x, page, y + 1);
							break;
						case XY_PLANE:
							glVertex3i(x, y, page);
							glVertex3i(x + 1, y, page);
							glVertex3i(x + 1, y + 1, page);
							glVertex3i(x, y + 1, page);
							break;
						}	
						glEnd();

					}
				}
			}
		}
	}
}

void BuildGLWidget::drawTargets(bool inChoise)
{
//	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_POLYGON_OFFSET_FILL);


	glPolygonOffset(1.0, 1.0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	drawTargetsPart(false, false);
	glPolygonOffset(0,0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawTargetsPart(false, true);

	// draw the error cylinders before the blue tiles.
	if ((!inChoise) && (m_doc->getBuild().getTestResult() == GEN_RESULT_ILLEGAL_SIDE))
	{
		drawErrorCyliders();
	}

	// if in boxed mode, real transparent ones are not real targets.
	// if in tiled mode, they are.
	// => don't show the blue ones if we're in choise and in boxed mode
	if ((!inChoise) || (!m_bBoxedMode)) 
	{
		// transperats
		glPolygonOffset(1.0, 1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawTargetsPart(true, false);
		glPolygonOffset(0,0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawTargetsPart(true, true);
	}
}
#endif

void BuildGLWidget::checkSides()
{
	if (m_doc->getBuild().testShape() == GEN_RESULT_ILLEGAL_SIDE)
	{ // has errors
		m_errCylindrAlpha = 1.0f;
		m_errCylindrAlphaDt = -0.1f;
	}
}


void BuildGLWidget::boxedDblClick(int choise, QMouseEvent *event)
{
	if ((choise == -1) || (!m_bEditEnabled))
		return;

	BuildWorld& build = m_doc->getBuild();

	bool remove = hXor(m_bBoxRemove, m_bInternalBoxRemove);

	if (remove && (build.nFaces <= 6)) // last cube, don't remove, ignore command silently
		return;

	CoordBuild bb[6];
	Vec3i g;
	if (!getChoiseTiles(choise, remove, bb, g))
		return; // out of bounds, do nothing

	bool hasStrt = false;
	int strtOrigDim = -1; // the original dimention of the strt tile
	for (int j = 0; j < 6; ++j) {// check if we're going to step on a start tile
		if (GET_VAL(build.get(bb[j])) == FACE_STRT)
		{ hasStrt = true; strtOrigDim = bb[j].dim; break; }
	}
		
	int facePut = -1;
	for (int j = 0; j < 6; ++j) 
	{
		if (GET_TYPE(build.get(bb[j])) == TYPE_REAL)  //do the removes
		{
			build.set(bb[j], 0);
			--build.nFaces;
		}
		else
		{
			facePut = FACE_NORM; 
			// search for a face with the orginal dimention
			if (hasStrt && (bb[j].dim == strtOrigDim)) { facePut = FACE_STRT; hasStrt = false; }
			build.set(bb[j], facePut);
			++build.nFaces;
		}
	}
	
	if (hasStrt) // take care of strt tile if it hasn't been taken care of yet
	{
		if (facePut != -1) // there are faces, but not ones in the original dimention
		{
			for (int j = 0; j < 6; ++j) // choose a face we put NORM in and change it to STRT
				if (build.get(bb[j]) == FACE_NORM) { build.set(bb[j], FACE_STRT); break; }
		}
		else // there are no faces to transfer the STRT to... settle for anything
		{
			build.search(FACE_NORM, FACE_STRT, false, true);
		}
	}

	build.m_space.axx(g).fill = remove?0:1;

	build.justChanged();
	build.reClacLimits();

	checkSides(); // testShape does the generate, created cyliders

	emit changedTilesCount(build.nFaces);
	
	updateGL();

	mouseMoveEvent(event); // simulate a move event to show the SELECT 
	makeBuffers();
}


bool BuildGLWidget::getChoiseTiles(int choise, bool remove, CoordBuild bb[6], Vec3i& g)
{
	CoordBuild c(GET_DIM(choise), GET_PAGE(choise), GET_X(choise), GET_Y(choise));
	BuildWorld& build = m_doc->getBuild();

	Vec3i g1, g2;
	BuildWorld::get3dCoords(c, g1, g2);

	if (hXor(build.m_space.axx(g1).fill == 1, remove))
		g = g2;
	else
		g = g1;

	if ((g.x < 1) || (g.x >= build.m_space.szx - 2) ||   // stay away from zeros and 49s
		(g.y < 1) || (g.y >= build.m_space.szy - 2) || 
		(g.z < 1) || (g.z >= build.m_space.szz - 2))
		return false;

	int xxx = build.m_space.axx(g).fill;
	Q_ASSERT(build.m_space.axx(g).fill == (remove?1:0)); // XXXa really nasty bug.

	BuildWorld::getBuildCoords(g, bb);
	return true;
}

void BuildGLWidget::fadeTimeout()
{
	BuildWorld& build = m_doc->getBuild();
	if ((m_nMarkedTiles == 0) && (build.getTestResult() != GEN_RESULT_ILLEGAL_SIDE))
		return;

	for(int i = 0; i < m_nMarkedTiles; ++i)
	{
		int theget = build.get(m_curMarkedTiles[i]);
		int getin = GET_INTENSITY(theget);
		//int bla = SET_INTENSITY(theget, getin+1);
		if (getin < 8)
			build.set(m_curMarkedTiles[i], SET_INTENSITY(theget, getin+2));
		else
			m_nMarkedTiles = 0; // if one reached it, they all did.
	}

	if (build.getTestResult() == GEN_RESULT_ILLEGAL_SIDE)
	{
		m_errCylindrAlpha += m_errCylindrAlphaDt;
		if ((m_errCylindrAlpha >= 1.0) || (m_errCylindrAlpha <= 0.1))
			m_errCylindrAlphaDt = -m_errCylindrAlphaDt;
	}
	updateGL();

}

// returns true if an updateGL is needed, false if not
bool BuildGLWidget::doMouseMove(QMouseEvent *event, bool remove)
{
	if ((!m_bBoxedMode) || (m_fSetStrtMode))
		return false;

	BuildWorld& build = m_doc->getBuild();
	
	int choise;
	if (event != NULL) // support non-mouse updates
	{
		choise = doChoise(event->x(), event->y());
		//printf("%8X  dim=%d  page=%2d  x=%2d  y=%2d\n", choise, GET_DIM(choise), GET_PAGE(choise), GET_X(choise), GET_Y(choise));
		if ((choise == m_lastChoise) && (remove == m_bLastBoxRemove))
			return false; // check if remove state just changed so we need to redraw
	}
	else
		choise = m_lastChoise;

	m_lastChoise = choise;
	m_bLastBoxRemove = remove;

	EActStatus act = remove?REMOVE:ADD;

	if (choise != -1)
	{ // something chosen
		int theget;
		CoordBuild bb[6];
		Vec3i g;
		if ((getChoiseTiles(choise, remove, bb, g) && (g != m_lastCubeChoise)))
		{ // selection was changed
			if (m_bEditEnabled)
			{
				build.clean(BuildWorld::CLEAN_TRANS_SHOW);
				m_nMarkedTiles = 0;

				if (!remove)
				{
					for (int j = 0; j < 6; ++j) // record the actions needed,
					{
						if (GET_TYPE(build.get(bb[j])) != TYPE_REAL)  //do the removes
						{
							build.set(bb[j], FACE_TRANS_SEL);
							m_curMarkedTiles[m_nMarkedTiles++] = bb[j];
						}
					}
				}
				else if (build.nFaces > 6) // if its the last one, don't show the red
				{
					for (int j = 0; j < 6; ++j) // record the actions needed,
					{
						theget = build.get(bb[j]);
						if (GET_TYPE(theget) == TYPE_REAL)  //do the removes
						{
							build.set(bb[j], theget | SHOW_REOMOVE);
							m_curMarkedTiles[m_nMarkedTiles++] = bb[j];
						}
					}
				}
				else
					act = CANT_REMOVE;

				makeBuffers();
			}
			else
				act = EDIT_DISABLE;

			m_lastCubeChoise = g;
			emit changedTileHover(choise, act);
		}
		// don't emit changedTileHover
	}
	else
	{
		// clean the trans place or remove from the last time
		build.clean(BuildWorld::CLEAN_TRANS_SHOW);
		m_lastCubeChoise = Vec3i(-1,-1,-1);
		emit changedTileHover(choise, act);
	}

	return true;
}

void BuildGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	m_bInternalBoxRemove = ((event->modifiers() & Qt::ControlModifier) != 0);
	bool needupdate = doMouseMove(event, hXor(m_bBoxRemove, m_bInternalBoxRemove));

	m_bDoneUpdate = false;
	GLWidget::mouseMoveEvent(event);  // important to keep rotation live
	if (needupdate && (!m_bDoneUpdate))
		updateGL();
}

void BuildGLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	int choise = doChoise(event->x(), event->y());

	if ((m_bBoxedMode) && (!m_fSetStrtMode))
		boxedDblClick(choise, event);
	else
		tiledDblClick(choise, event);

}

void BuildGLWidget::keyEvent(QKeyEvent *event)
{
	m_bInternalBoxRemove = ((event->modifiers() & Qt::ControlModifier) != 0);
	bool remove = hXor(m_bBoxRemove, m_bInternalBoxRemove);

	if (doMouseMove(NULL, remove)) // simulate a mouse move
		updateGL();
	emit changedAction(remove);
}


void BuildGLWidget::changeAction(bool remove) // someone is saying its changed
{
	if (remove == hXor(m_bBoxRemove, m_bInternalBoxRemove))
		return;

	m_bBoxRemove = remove;
	if (doMouseMove(NULL, remove)) // simulate a mouse move
		updateGL();
	
}



void BuildGLWidget::switchIn()
{
	m_fadeTimer->start(25);
}

void BuildGLWidget::switchOut()
{
	m_fadeTimer->stop();
}



void BuildGLWidget::reCalcBldMinMax()
{

	BuildWorld &build = m_doc->getBuild();
	build.clean(BuildWorld::CLEAN_TRANS_SHOW);
	build.reClacLimits();

	const SqrLimits &lYZ = build.m_limits[YZ_PLANE];
	aqmin = Vec3(lYZ.minpage, lYZ.minx, lYZ.miny);
	aqmax = Vec3(lYZ.maxpage, lYZ.maxx, lYZ.maxy);

	const SqrLimits &lXZ = build.m_limits[XZ_PLANE];
	aqmin.pmin(Vec3(lXZ.minx, lXZ.minpage, lXZ.miny));
	aqmax.pmax(Vec3(lXZ.maxx, lXZ.maxpage, lXZ.maxy));

	const SqrLimits &lXY = build.m_limits[XY_PLANE];
	aqmin.pmin(Vec3(lXY.minx, lXY.miny, lXY.minpage));
	aqmax.pmax(Vec3(lXY.maxx, lXY.maxy, lXY.maxpage));

	//double dx = aqmax[0] - aqmin[0], dy = aqmax[1] - aqmin[1], dz = aqmax[2] - aqmin[2];

	aqmin[0] += 1; aqmin[1] += 1; aqmin[2] += 1;
	aqmax[0] -= 1; aqmax[1] -= 1; aqmax[2] -= 1;

	// needed because of the adjustment in reCalcLimits
	aqmin[0] -= 1; aqmin[1] -= 1; aqmin[2] -= 1;

}

void BuildGLWidget::updateView(int hint)
{
	BuildWorld& build = m_doc->getBuild();

	switch (GET_BLD_HINT(hint))
	{
	// TBD: hint new build!
	case HINT_BLD_NEW: // a different build then the one we have now.
		checkSides();
		emit changedTilesCount(build.nFaces);
	case HINT_BLD_MINMAX:
		reCalcBldMinMax(); //fall through
		DoReset();

	case HINT_BLD_PAINT:
		update();
		updateGL();
		break;
	}
}



void BuildGLWidget::tiledDblClick(int choise, QMouseEvent *event)
{
	Q_UNUSED(event)
	if (choise == -1)
		return;

	BuildWorld& build = m_doc->getBuild();
	CoordBuild c(GET_DIM(choise), GET_PAGE(choise), GET_X(choise), GET_Y(choise));
	int theget = build.get(c);

	if (GET_VAL(theget) == FACE_TRANS)
	{
		if (!m_fUnSetBlueMode)
		{
			build.set(c, FACE_NORM);
			build.nFaces++;
			build.justChanged();
			build.doTransparent();
			updateGL();
		}
		else
		{
			build.set(c, FACE_TRANS_NONE);
			updateGL();
		}
	}
	else
	{	
		if (GET_TYPE(theget) == TYPE_REAL)
		{
			if (!m_fSetStrtMode)
			{
				if (build.nFaces == 1)
				{
					QMessageBox::information(this, APP_NAME, tr("I'm sure you don't really want to remove the only piece in the structure."), QMessageBox::Ok);
				}
				else
				{
					build.set(c, 0);
					build.nFaces--;
					build.justChanged();
					build.doTransparent();
					updateGL();
				}
			}
			else
			{
				if (theget != FACE_STRT)
				{
					build.search(FACE_STRT, FACE_NORM); // clean the current strt
					build.set(c, FACE_STRT);
					build.justChanged();

					checkSides(); // does the generate. (for recalculating the stepping)

					updateGL();
				}
			}
		}
	}
}


void BuildGLWidget::slvProgStatsUpdate(int hint, int data)
{
	Q_UNUSED(data)
	switch (hint)
	{
	case SHINT_START: enableEdit(false); break;
	case SHINT_STOP: enableEdit(true); break;
	}
}
 
