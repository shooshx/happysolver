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
#include <QtGui>
#include <QMessageBox>
#include <QDomDocument>
#include <QFile>

#include "general.h"
#include "Pieces.h"
#include "Texture.h"
#include "CubeDoc.h"
#include "GLWidget.h"
#include "MainWindow.h" ///< MainWindow::wasClosed() is being used.
#include "PicsSet.h"
#include "NoiseGenerator.h"

#define TEX_X 64
#define TEX_Y 64
#define TEX_BUF_SIZE (TEX_X * TEX_Y * 4) 

// the singleton
PicBucket *PicBucket::g_instance = NULL;
void PicBucket::createSingleton() 
{ 
	if (g_instance == NULL) // never gets erased...
		g_instance = new PicBucket; 
}




static PicDef::PathPoint pathway[17] = {
	{{0, 0}, {true, {0,0}, {1,0}}, {1,0, 1,1} },
	{{1, 0}, {false,{2,0}, {2,1}}, {2,0, 2,1} },
	{{2, 0}, {false,{3,0}, {3,1}}, {3,0, 3,1} },
	{{3, 0}, {false,{4,0}, {4,1}}, {4,0, 4,1} },
	{{4, 0}, {true, {5,0}, {5,1}}, {5,1, 4,1} },
	{{4, 1}, {false,{5,2}, {4,2}}, {5,2, 4,2} },
	{{4, 2}, {false,{5,3}, {4,3}}, {5,3, 4,3} },
	{{4, 3}, {false,{5,4}, {4,4}}, {5,4, 4,4} },
	{{4, 4}, {true, {5,5}, {4,5}}, {4,5, 4,4} },
	{{3, 4}, {false,{3,5}, {3,4}}, {3,5, 3,4} },
	{{2, 4}, {false,{2,5}, {2,4}}, {2,5, 2,4} }, 
	{{1, 4}, {false,{1,5}, {1,4}}, {1,5, 1,4} },
	{{0, 4}, {true, {0,5}, {0,4}}, {0,4, 1,4} },
	{{0, 3}, {false,{0,3}, {1,3}}, {0,3, 1,3} },
	{{0, 2}, {false,{0,2}, {1,2}}, {0,2, 1,2} },
	{{0, 1}, {false,{0,1}, {1,1}}, {0,1, 1,1} },
	{{0, 0}, {false,{-1,-1},{-1,-1}},{-1,-1,-1,-1}} // last one is for wrap up with first. not really read
};



void PicDef::makeBoundingPath()
{
	pathlen = 0;
	for (int step = 0; step < 16; ++step)
	{
		PathPoint &ppd = pathway[step];
		PathPoint &ppdnx = pathway[step + 1];
		if (v.axx(ppd.pnt.x, ppd.pnt.y) == 1)
		{
			// frame
			if (ppd.frm.both) // this can actually be (step%4 == 0) but more coherent this way
			{
				path[pathlen++] = ppd.frm.l1;
				path[pathlen++] = ppd.frm.l2;
			}
			else
				path[pathlen++] = ppd.frm.l1;
			// prependiculars
			if (v.axx(ppdnx.pnt.x, ppdnx.pnt.y) == 0)
				path[pathlen++] = PathCoord(ppd.prp.ex, ppd.prp.ey);
		}
		else // v[][] == 0
		{
			// frame
			if (!ppd.frm.both)
				path[pathlen++] = ppd.frm.l2;
			// prependiculars
			if (v.axx(ppdnx.pnt.x, ppdnx.pnt.y) == 1)
				path[pathlen++] = PathCoord(ppd.prp.sx, ppd.prp.sy); // reversed
		}

	}
	path[pathlen++] = path[0]; // loop back to first one

}


void PicBucket::makeBitmapList()
{	
	Q_ASSERT(grps.size() > 0);

	int gind, pind, i, x, y, tmp;

	unsigned char staticbuf[128 * 128 * 4];
	unsigned char *origcbuf = NULL;
	QSize origsize(0, 0);
	int origfactor = 1;

	for(gind = 0; gind < grps.size(); ++gind)
	{
		PicGroupDef &cgrp = grps[gind];
		const QImage &gimg = cgrp.tex->img;
		switch (cgrp.drawtype)
		{
		case DRAW_COLOR:
			origcbuf = staticbuf;
			origsize = QSize(64, 64);
			origfactor = 1;
			tmp = origsize.width() * origsize.height() * 4;
			for (i = 0; i < tmp; i += 4)
			{// little endian images
				origcbuf[i + 0] = int(cgrp.b * 255.0); 
				origcbuf[i + 1] = int(cgrp.g * 255.0);
				origcbuf[i + 2] = int(cgrp.r * 255.0);
				origcbuf[i + 3] = 255; //alpha
			}
			break;
		case DRAW_TEXTURE_NORM:
			origcbuf = staticbuf;
			origsize = gimg.size();
			origfactor = (gimg.width() == 64)?1:2; // otherwise its 128.
			tmp = origsize.width() * origsize.height() * 4;
			for (i = 0;  i < tmp; i += 4)
			{
				origcbuf[i + 0] = int( float(gimg.bits()[i]) * cgrp.b);
				origcbuf[i + 1] = int( float(gimg.bits()[i + 1]) * cgrp.g);
				origcbuf[i + 2] = int( float(gimg.bits()[i + 2]) * cgrp.r);
				origcbuf[i + 3] = 255;
			}
			break;
		case DRAW_TEXTURE_BLEND:
			origcbuf = (unsigned char*)gimg.bits(); // move to unsigned, rid of const.
			origsize = gimg.size();
			origfactor = (gimg.width() == 64)?1:2; // otherwise its 128.
		}

		for(pind = 0; pind < cgrp.numPics(); ++pind)
		{ // TBD: for every other then INDIVIDUAL do this for only one time for grp
			PicDef &cpic = cgrp.getPic(pind); //'p'

			QImage pimg; // needs to be in this scope so it won't get destroyed
			if (isIndividual(cgrp.drawtype))
			{
				pimg = cpic.tex->img.mirrored(false, true); // transform it back 
				origcbuf = (unsigned char*)pimg.bits(); // move to unsigned, rid of const.
				origsize = pimg.size();
				origfactor = (pimg.width() == 64)?1:2; // otherwise its 128.
			}

			QImage img(TEX_X + 1, TEX_Y + 1, QImage::Format_ARGB32);
			// image is 1 pixel longer and wider then the texture to save place for the line.
			// this was grueling.
			img.fill(0x00000000);

			unsigned char *cbuf = img.bits();

			for (y = 0; y < TEX_Y; ++y)  // x,y over the target image
			{
				for (x = 0; x < TEX_X; ++x)
				{ 
					for (int i = 0; i < 4; ++i)
					{
						cbuf[(x+(TEX_X+1)*y)*4 + i] = origcbuf[(x*origfactor + origsize.width()*y*origfactor)*4 + i];
					}
				}
			}

			for (x = 0; x < 5; ++x)
			{
				for (int ry = 0; ry < 5; ++ry)
				{
					y = ry; // inversion of DIB
					if (cpic.v.axx(x,ry) == 0)
					{
						int px = int( double(x)/5.0 * double(TEX_X) );
						int py = int( double(y)/5.0 * double(TEX_Y) );
						int xw = int( double(TEX_X) / 5.0);
						int yw = int( double(TEX_Y) / 5.0);

						for (int lx = px * 4; lx < (px + xw + 1) * 4; lx += 4)
						{
							for (int ly = py * 4; ly < (py + yw + 1) * 4; ly += 4)
							{
								cbuf[ly * (TEX_X + 1) + lx + 3] = 0; // alpha;
							}
						}
					}
				}
			}

			cpic.makeBoundingPath();

			cpic.pixmap = QPixmap::fromImage(img);
			QPolygon poly;
			for (i = 0; i < cpic.pathlen; ++i)
			{ // path includes last point == first point
				poly.push_back(QPoint( (int)((cpic.path[i].x / 5.0)*64), (int)((cpic.path[i].y / 5.0)*64)));
			}

			QPainter painter(&cpic.pixmap);
			//painter.drawImage(0,0,img);
			//QPen pen(Qt::red, 2);                 // red solid line, 2 pixels wide
			//painter.setPen(pen);              // set the red, wide pen

			painter.drawPolyline(poly);
			painter.end();


		}
	}
}

QImage endianSwapped(const QImage& src, int n)
{
	int oto[4] = { n / 1000, (n / 100) % 10, (n / 10) % 10, n % 10 };
	int o[4];

	int h = src.height(), w = src.width();
	QImage res(w, h, QImage::Format_ARGB32);
	for (int i = 0; i < h; ++i) 
	{
		uint *q = (uint*)res.scanLine(i);
		uint *p = (uint*)src.scanLine(i);
		uint *end = p + w;
		while (p < end) 
		{
			o[0] = (*p >> 24);
			o[1] = (*p >> 16) & 0xFF;
			o[2] = (*p >> 8) & 0xFF;
			o[3] = (*p) & 0xFF;

			uint res = 0;
			for (int j = 0; j < 4; ++j)
			{
				switch (oto[j])
				{
				case 1: res |= o[j] << 24; break;
				case 2: res |= o[j] << 16; break;
				case 3: res |= o[j] << 8; break;
				case 4: res |= o[j]; break;
				}
			}
			*q = res; //(*p >> 8) | (*p << 24);

			p++;
			q++;
		}
	}
	return res;
}

void PicBucket::updateTexture(int gind)
{
	if (grps[gind].tex != NULL)
	{
		QImage img = grps[gind].blendImage();
		int texind = grps[gind].tex->ind;
		texs[texind]->img = img;
		
		QImage b;
		//if (nSwapEndians != 1234)
		b = endianSwapped(img, nSwapEndians);
		//else
		//	b = img;
		emit updateTexture(texind, b);
	}
}

// causes an access violation on QContext destruction
void PicBucket::updateSwapEndian(int newen)
{
	if (newen == nSwapEndians)
		return;
	nSwapEndians = newen;

	QProgressDialog pdlg("Updating...", "", 0, grps.size(), g_main);
	pdlg.setWindowTitle("Happy Cube Solver");
	pdlg.setMinimumDuration(0);
	pdlg.setCancelButton(NULL);

	for (int g = 0; g < grps.size(); ++g)
	{
		pdlg.setValue(g);
		if ((grps[g].tex == NULL) || (!grps[g].tex->bound))
			continue;
		int texind = grps[g].tex->ind;

		QImage img = texs[texind]->img;
		QImage b;
		//if (nSwapEndians != 1234)
		b = endianSwapped(img, nSwapEndians);
		//else
		//	b = img;

		emit updateTexture(texind, b);
		
	}
}


Texture* PicBucket::newTexture(QImage im, bool emitb)
{
	int texind = texs.count();
	Texture *t = new Texture(texind, im, emitb);
	texs.append(t);
	if (emitb)
	{
		QImage b;
		//if (nSwapEndians != 1234)
		b = endianSwapped(t->img, nSwapEndians);
		//else
		//	b = t->img;
		emit boundTexture(texind, b);
	}
	return t;
}

QImage PicGroupDef::blendImage()
{
	//QImage img(TEX_X, TEX_Y, QImage::Format_ARGB32);
	//int size = TEX_X * TEX_Y * 4;
	QImage img(baseTex->img.width(), baseTex->img.height(), QImage::Format_ARGB32);
	int size = baseTex->img.width() * baseTex->img.height() * 4;
	unsigned char *cbuf = img.bits();

	const QImage& baseTexImg = baseTex->img;
	for (int c = 0;  c < size; c += 4)
	{
		float fb = float(baseTexImg.bits()[c]);
		float fg = float(baseTexImg.bits()[c + 1]);
		float fr = float(baseTexImg.bits()[c + 2]);	

		cbuf[c + 0] = int( (255.0-fb) * b + fb * exB);
		cbuf[c + 1] = int( (255.0-fg) * g + fg * exG);
		cbuf[c + 2] = int( (255.0-fr) * r + fr * exR);
		cbuf[c + 3] = 255;
	}
	return img;	
}


static EDrawType getDrawType(const QString& s) {
	if (s == "COLOR") return DRAW_COLOR;
	if (s == "TEXTURE_NORM") return DRAW_TEXTURE_NORM;
	if (s == "TEXTURE_BLEND") return DRAW_TEXTURE_BLEND;
	if (s == "TEXTURE_INDIVIDUAL_HALF") return DRAW_TEXTURE_INDIVIDUAL_HALF;
	if (s == "TEXTURE_INDIVIDUAL_WHOLE") return DRAW_TEXTURE_INDIVIDUAL_WHOLE;
	Q_ASSERT(false);
	return (EDrawType)0;
}


PicDef& PicGroupDef::getPic(int myi) {
	return PicBucket::mutableInstance().pdefs[picsi[myi]];
}
const PicDef& PicGroupDef::getPic(int myi) const {
	return PicBucket::mutableInstance().pdefs[picsi[myi]];
}

const PicGroupDef* PicDef::mygrp() const {
	return &PicBucket::instance().grps[mygrpi];
}



bool PicBucket::loadXML(const QString& xmlname)
{

	//	unsigned int xxx1 = GetTickCount();
	QDomDocument doc;
	QFile file(xmlname);

	if (!file.open(QIODevice::ReadOnly))
	{// try again in the executable path
		file.setFileName(QCoreApplication::applicationDirPath() + "/" + xmlname); 
		if (!file.open(QIODevice::ReadOnly))
		{
			QMessageBox::critical(g_main, APP_NAME, "error opening file:\n" + xmlname, QMessageBox::Ok, 0);
			return false;
		}
	}
	if (!doc.setContent(&file)) 
	{
		QMessageBox::critical(g_main, APP_NAME, "error parsing XML file:\n" + xmlname, QMessageBox::Ok, 0);
		file.close();
		return false;
	}
	file.close();
//	unsigned int xxx2 = GetTickCount();

	QMap<QString, int> grpnames; // maps the name of a group to its index. used for copying pieces
	QDomElement docElem = doc.documentElement();
	QDomElement texe = docElem.firstChildElement("textures");

	////////////// texture files ////////////////////////
	QDomElement t = texe.firstChildElement("texture");
	while (!t.isNull())
	{
		QString txname = t.attribute("filename");
		if (!txname.isEmpty())
		{
			QImage img(txname, "PNG");
			if (img.isNull())
			{
				img.load(QCoreApplication::applicationDirPath() + "/" + txname, "PNG");
				if (img.isNull())
				{
					QMessageBox::critical(g_main, APP_NAME, "Failed to load texture file: " + txname, QMessageBox::Ok, 0);
					return false;
				}
			}
		    
			// don't bind this texture yet, it will be bound later when in is modified specifically
			newTexture(img, img.height() <= 128);
		}
		else
		{
			int copyind = t.attribute("copy").toInt();
			int cX = t.attribute("x1").toInt();
			int cY = t.attribute("y1").toInt();
			newTexture(texs[copyind]->img.copy(cX, cY, 128, 128), true);
		}
		
		t = t.nextSiblingElement();
	}

//	unsigned int xxx3 = GetTickCount();

	////////////////////// pics ////////////////////////////////
	QDomElement xfams = docElem.firstChildElement("outlines");

	int grpCount = 0;
	// count the number of cubes (groups) we have
	QDomElement xfam = xfams.firstChildElement("family");
	while (!xfam.isNull())
	{
		int ind = xfam.attribute("index").toInt(); // the index it should show at the main build helper menu
		if (ind >= families.size())
			families.resize(ind + 1);
		PicFamily &cfam = families[ind];
		cfam.startIndex = grpCount;
		cfam.numGroups = xfam.elementsByTagName("group").size();
		cfam.name = xfam.attribute("name");
		cfam.onResetSetCount = xfam.attribute("onResetSetCount").toInt();
		cfam.iconFilename = xfam.attribute("icon");

		grpCount += cfam.numGroups;
		xfam = xfam.nextSiblingElement(); // next family
	}

	pdefs.resize(grpCount*6);
	// the painter includes a pointer to the def so it needs to be initialized in the real vector
	for(int i = 0; i < grpCount*6; ++i)
		pdefs[i].reset();


	sumPics = 0;
	int grpi = 0; // (i) group number
	int pdefi = 0;

	xfam = xfams.firstChildElement("family");
	while (!xfam.isNull())
	{
		QDomElement cube = xfam.firstChildElement("group"); // a group of pieces is a cube
		while (!cube.isNull())
		{
			PicGroupDef cgrp;
			cgrp.name = cube.attribute("name");
			grpnames[cgrp.name] = grpi;
	
			QDomElement fill = cube.firstChildElement("fill");

			cgrp.drawtype = getDrawType(fill.attribute("type"));

			if (fill.hasAttribute("texind"))
			{
				int txind = fill.attribute("texind").toInt();
				Q_ASSERT((txind >= 0) && (txind <= texs.size()));
				cgrp.baseTex = texs[txind];
			}
			else
				cgrp.baseTex = NULL;
			cgrp.tex = cgrp.baseTex;

			cgrp.r = float(fill.attribute("r").toInt()) / 255.0f; 
			cgrp.g = float(fill.attribute("g").toInt()) / 255.0f; 
			cgrp.b = float(fill.attribute("b").toInt()) / 255.0f;
			cgrp.exR = float(fill.attribute("exR").toInt()) / 255.0f; 
			cgrp.exG = float(fill.attribute("exG").toInt()) / 255.0f; 
			cgrp.exB = float(fill.attribute("exB").toInt()) / 255.0f;
			cgrp.blackness = (EBlackness)fill.attribute("k").toInt();
	
			if (cgrp.drawtype == DRAW_TEXTURE_INDIVIDUAL_HALF)
			{
				cgrp.sideTexX = fill.attribute("sideX").toInt();
				cgrp.sideTexY = fill.attribute("sideY").toInt();
				int stxind = fill.attribute("sideTexInd").toInt();
				Q_ASSERT((stxind >= 0) && (stxind <= texs.size()));
				cgrp.sideTex = texs[stxind];
			}
			if (cgrp.drawtype == DRAW_TEXTURE_BLEND)
			{
				cgrp.tex = newTexture(cgrp.blendImage());

			}

			QDomNodeList xpics = cube.elementsByTagName("piece");
			if (xpics.size() > 0)
			{
				//cgrp.pics.resize();

				for (int pici = 0; pici < xpics.size(); ++pici)
				{
					PicDef& curdef = pdefs[pdefi];
					curdef.mygrpi = grpi;
					curdef.indexInGroup = pici;

					QDomElement ep = xpics.at(pici).toElement();
					QString text = ep.text();
					text.remove(' ');
					for (int y = 0; y < 5; ++y)
					{
						for (int x = 0; x < 5; ++x)
						{
							// get the first number;
							if (text.at(0) == '0') curdef.v.set(x, y) = 0;
							else curdef.v.set(x,y) = 1;
							text.remove(0, 1);

						}
					}
					if (isIndividual(cgrp.drawtype))
					{
						curdef.xOffs = ep.attribute("x1").toInt();
						curdef.yOffs = ep.attribute("y1").toInt();
						curdef.tex = newTexture(cgrp.tex->img.copy(curdef.xOffs, curdef.yOffs, 128, 128).mirrored(false, true));
					}
					cgrp.picsi.push_back(pdefi); // the index in the bucket
					++pdefi;
				}
			}
			else // no "piece" elements
			{
				QDomElement pics = cube.firstChildElement("pieces");
				QString copyfrom = pics.attribute("copy");
				if (grpnames.find(copyfrom) == grpnames.end())
				{
					QMessageBox::critical(g_main, APP_NAME, "can't copy piece from: " + copyfrom, QMessageBox::Ok, 0);
					return false;
				}
				
				int ifrom = grpnames[copyfrom];
				int copyCount = grps[ifrom].numPics();
				//defs[i].pics.resize(defs[ifrom].numPics());
				for (int pici = 0; pici < copyCount; ++pici)
				{
					PicDef &curdef = pdefs[pdefi];
					curdef.mygrpi = grpi;
					curdef.indexInGroup = pici;

					const PicDef &fromDef = grps[ifrom].getPic(pici);
					curdef.v = fromDef.v; // copy only the data of the piece


					if (isIndividual(cgrp.drawtype))
					{
						curdef.xOffs = fromDef.xOffs;
						curdef.yOffs = fromDef.yOffs;
						curdef.tex = newTexture(cgrp.tex->img.copy(curdef.xOffs, curdef.yOffs, 128, 128).mirrored(false, true));
					}

					cgrp.picsi.push_back(pdefi);
					++pdefi;
				}

			}

			grps.push_back(cgrp);
			sumPics += cgrp.numPics();
			++grpi;
			cube = cube.nextSiblingElement();
		} // cubes
		xfam = xfam.nextSiblingElement(); // next family
	} // families

	// set the reset selection
	setToFamResetSel();


//	unsigned int xxx5 = GetTickCount();

//	QMessageBox::information(NULL, "bla", QString("%1\n%2\n%3\n%4").arg(xxx2 - xxx1).arg(xxx3 - xxx2).arg(xxx4 - xxx3).arg(xxx5 - xxx4));
//		.arg(MyPoint::g_ctorCount).arg(MyPoint::g_dtorCount).arg(MyPolygon::g_ctorCount).arg(MyPolygon::g_dtorCount));

	return true;

}


void PicBucket::setToFamResetSel()
{
	for(int f = 0; f < families.size(); ++f)
	{
		const PicFamily &fam = families[f];
		int setTo = fam.onResetSetCount;
		for (int g = fam.startIndex; g < fam.startIndex + fam.numGroups; ++g)
		{
			PicGroupDef &grp = grps[g];
			for (int p = 0; p < grp.numPics(); ++p)
			{
				grp.getPic(p).setSelected(setTo);
			}
		}
	}
}


void PicBucket::distinctMeshes()
{
	PicsSet ps;
	for(int i = 0; i < pdefs.size(); ++i) {
		ps.add(i);
	}
	printf("%d distinct meshes out of %d pieces\n", ps.comp.size(), pdefs.size());

	for(int i = 0; i < ps.comp.size(); ++i) {
		auto comp = ps.comp[i];
		shared_ptr<PicDisp> pd(new PicDisp);
		pd->m_arr = comp.rtns[0];
		for(int j = 0; j < comp.addedInds.size(); ++j) {
			auto added = comp.addedInds[j];
			pdefs[added.addedInd].dispRot = added.defRot;
			pdefs[added.addedInd].disp = pd;
		}
		meshes.push_back(pd);
	}

}


/** PoolStats is the datum of the poolStats list which holds the optimal sizes of MyAllcator pools.
	Sizes for the MyPoint, MyPolygon and HalfEdge pools, measured as a function of the number of
	subdivision passe using MyObject::subdivide().
	the poolStats list is used in PicBucket::buildMeshes().
*/
struct PoolStats
{
	int points, poly, he;
};

static PoolStats poolStats[] = {
	{ 100, 100, 0 },         // 0 passes: 72, 70, 0
	{ 300, 370, 300 },       // 1 pass: 282, 350, 280
	{ 1200, 1500, 1200 },    // 2 passes: 1122, 1470, 1120
	{ 4500, 6000, 4500 },    // 3 passes: 4482, 5950, 4480
	{ 17950, 23900, 17950 }}; // 4 passes: 17922, 23870, 17920



void PicBucket::buildMeshes(const DisplayConf& dpc, bool showStop, GLWidget *mainContext)
{
	// fill meshes
	distinctMeshes();

	QProgressDialog pdlg("Building the pieces. Please Wait...", "Stop", 0, meshes.size(), g_main);
	pdlg.setSizeGripEnabled(false);
	pdlg.setWindowTitle("Happy Cube Solver");
	pdlg.setMinimumDuration(1500);
	if (!showStop)
		pdlg.setCancelButton(NULL);

	// grows if needed, never shrinks.
	PoolStats& ps = poolStats[dpc.numberOfPasses];
	PicDisp::getAllocator().init(ps.points, ps.poly, ps.he);
	PicDisp::getAllocator().clearMaxAlloc();

	int cnt = 0;
	bool cancel = false;

	for (int pi = 0; pi < meshes.size() && !cancel; ++pi)
	{
        pdlg.setValue(cnt++);
		meshes[pi]->init(dpc, mainContext);

        qApp->processEvents(); // must call
		cancel = ((showStop && pdlg.wasCanceled()) || (((MainWindow*)g_main)->wasClosed()));
	}

	pdlg.setValue(cnt);
	PicDisp::getAllocator().checkMaxAlloc();
}


int PicBucket::selectedCount() const
{
	int count = 0;
	for (int pi = 0; pi < pdefs.size(); ++pi)
	{
		count += pdefs[pi].getSelected();
	}
	return count;
}


int rotationAdd(int base, int defRot) 
{
	if ((defRot < 4) == (base < 4))
		return (base - defRot + 4) % 4;
	else
		return ((base + defRot) % 4) + 4;

}

int rotationSub(int x, int defRot)
{
	if (defRot < 4) {
		if (x < 4)
			return (x + defRot + 4) % 4;
		else
			return ((x - defRot) % 4) + 4;
	}
	else {
		if (x < 4)
			return ((x + defRot) % 4) + 4;
		else
			return (x - defRot + 4) % 4;
	}

}
