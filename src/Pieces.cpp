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
	Q_ASSERT(defs.size() > 0);

	int gind, pind, i, x, y, tmp;

	unsigned char staticbuf[128 * 128 * 4];
	unsigned char *origcbuf = NULL;
	QSize origsize(0, 0);
	int origfactor = 1;

	for(gind = 0; gind < numDefs(); ++gind)
	{			
		const QImage &gimg = defs[gind].tex->img;
		switch (defs[gind].drawtype)
		{
		case DRAW_COLOR:
			origcbuf = staticbuf;
			origsize = QSize(64, 64);
			origfactor = 1;
			tmp = origsize.width() * origsize.height() * 4;
			for (i = 0; i < tmp; i += 4)
			{// little endian images
				origcbuf[i + 0] = int(defs[gind].b * 255.0); 
				origcbuf[i + 1] = int(defs[gind].g * 255.0);
				origcbuf[i + 2] = int(defs[gind].r * 255.0);
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
				origcbuf[i + 0] = int( float(gimg.bits()[i]) * defs[gind].b);
				origcbuf[i + 1] = int( float(gimg.bits()[i + 1]) * defs[gind].g);
				origcbuf[i + 2] = int( float(gimg.bits()[i + 2]) * defs[gind].r);
				origcbuf[i + 3] = 255;
			}
			break;
		case DRAW_TEXTURE_BLEND:
			origcbuf = (unsigned char*)gimg.bits(); // move to unsigned, rid of const.
			origsize = gimg.size();
			origfactor = (gimg.width() == 64)?1:2; // otherwise its 128.
		}

		for(pind = 0; pind < defs[gind].numPics(); ++pind)
		{ // TBD: for every other then INDIVIDUAL do this for only one time for grp
			PicDef &p = defs[gind].pics[pind];

			QImage pimg; // needs to be in this scope so it won't get destroyed
			if (isIndividual(defs[gind].drawtype))
			{
				pimg = p.tex->img.mirrored(false, true); // transform it back 
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
					if (defs[gind].pics[pind].v.axx(x,ry) == 0)
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

			p.makeBoundingPath();

			p.pixmap = QPixmap::fromImage(img);
			QPolygon poly;
			for (i = 0; i < p.pathlen; ++i)
			{ // path includes last point == first point
				poly.push_back(QPoint( (int)((p.path[i].x / 5.0)*64), (int)((p.path[i].y / 5.0)*64)));
			}

			QPainter painter(&p.pixmap);
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
	if (defs[gind].tex != NULL)
	{
		QImage img = defs[gind].blendImage();
		int texind = defs[gind].tex->ind;
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

	QProgressDialog pdlg("Updating...", "", 0, numDefs(), g_main);
	pdlg.setWindowTitle("Happy Cube Solver");
	pdlg.setMinimumDuration(0);
	pdlg.setCancelButton(NULL);

	for (int g = 0; g < numDefs() ; ++g)
	{
		pdlg.setValue(g);
		if ((defs[g].tex == NULL) || (!defs[g].tex->bound))
			continue;
		int texind = defs[g].tex->ind;

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
	QDomElement fams = docElem.firstChildElement("outlines");

	int i = 0;
	int count = 0;
	// count the number of cubes (groups) we have
	QDomElement fam = fams.firstChildElement("family");
	while (!fam.isNull())
	{
		int ind = fam.attribute("index").toInt();
		if (ind >= families.size())
			families.resize(ind + 1);
		families[ind].startIndex = count;
		families[ind].numGroups = fam.elementsByTagName("group").size();
		families[ind].name = fam.attribute("name");
		families[ind].onResetSetCount = fam.attribute("onResetSetCount").toInt();
		families[ind].iconFilename = fam.attribute("icon");
		count += families[ind].numGroups;
		fam = fam.nextSiblingElement(); // next family
		++i;
	}
	defs.resize(count);

//	unsigned int xxx4 = GetTickCount();

	sumPics = 0;
	i = 0; // group number

	fam = fams.firstChildElement("family");
	while (!fam.isNull())
	{
		QDomElement cube = fam.firstChildElement("group"); // a group of pieces is a cube
		while (!cube.isNull())
		{
			defs[i].name = cube.attribute("name");
			grpnames[defs[i].name] = i;
	
			QDomElement fill = cube.firstChildElement("fill");

			defs[i].drawtype = getDrawType(fill.attribute("type"));

			if (fill.hasAttribute("texind"))
			{
				int txind = fill.attribute("texind").toInt();
				Q_ASSERT((txind >= 0) && (txind <= texs.size()));
				defs[i].baseTex = texs[txind];
			}
			else
				defs[i].baseTex = NULL;
			defs[i].tex = defs[i].baseTex;

			defs[i].r = float(fill.attribute("r").toInt()) / 255.0f; 
			defs[i].g = float(fill.attribute("g").toInt()) / 255.0f; 
			defs[i].b = float(fill.attribute("b").toInt()) / 255.0f;
			defs[i].exR = float(fill.attribute("exR").toInt()) / 255.0f; 
			defs[i].exG = float(fill.attribute("exG").toInt()) / 255.0f; 
			defs[i].exB = float(fill.attribute("exB").toInt()) / 255.0f;
			defs[i].blackness = (EBlackness)fill.attribute("k").toInt();
	
			if (defs[i].drawtype == DRAW_TEXTURE_INDIVIDUAL_HALF)
			{
				defs[i].sideTexX = fill.attribute("sideX").toInt();
				defs[i].sideTexY = fill.attribute("sideY").toInt();
				int stxind = fill.attribute("sideTexInd").toInt();
				Q_ASSERT((stxind >= 0) && (stxind <= texs.size()));
				defs[i].sideTex = texs[stxind];
			}
			if (defs[i].drawtype == DRAW_TEXTURE_BLEND)
			{
				defs[i].tex = newTexture(defs[i].blendImage());
			}

			QDomNodeList pics = cube.elementsByTagName("piece");
			if (pics.size() > 0)
			{
				defs[i].pics.resize(pics.size());

				for (int picn = 0; picn < defs[i].numPics(); ++picn)
				{
					QDomElement ep = pics.at(picn).toElement();
					QString text = ep.text();
					text.remove(' ');
					PicDef &curdef = defs[i].pics[picn];
					curdef.mygrp = &defs[i];

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
					if (isIndividual(defs[i].drawtype))
					{
						curdef.xOffs = ep.attribute("x1").toInt();
						curdef.yOffs = ep.attribute("y1").toInt();
						curdef.tex = newTexture(defs[i].tex->img.copy(curdef.xOffs, curdef.yOffs, 128, 128).mirrored(false, true));
					}

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
				defs[i].pics.resize(defs[ifrom].numPics());
				for (int picn = 0; picn < defs[i].numPics(); ++picn)
				{
					PicDef &curdef = defs[i].pics[picn];
					const PicDef &fromDef = defs[ifrom].pics[picn];
					curdef.v = fromDef.v; // copy only the data of the piece
					curdef.mygrp = &defs[i];

					if (isIndividual(defs[i].drawtype))
					{
						curdef.xOffs = fromDef.xOffs;
						curdef.yOffs = fromDef.yOffs;
						curdef.tex = newTexture(defs[i].tex->img.copy(curdef.xOffs, curdef.yOffs, 128, 128).mirrored(false, true));
					}
				}
				

			}

			sumPics += defs[i].numPics();
			++i;
			cube = cube.nextSiblingElement();
		} // cubes
		fam = fam.nextSiblingElement(); // next family
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
			const PicGroupDef &grp = defs[g];
			for (int p = 0; p < grp.numPics(); ++p)
			{
				grp.pics[p].setSelected(setTo);
			}
		}
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
	QProgressDialog pdlg("Building the pieces. Please Wait...", "Stop", 0, sumPics, g_main);
	pdlg.setSizeGripEnabled(false);
	pdlg.setWindowTitle("Happy Cube Solver");
	pdlg.setMinimumDuration(1500);
	if (!showStop)
		pdlg.setCancelButton(NULL);

	// grows if needed, never shrinks.
	PoolStats& ps = poolStats[dpc.numberOfPasses];
	PicPainter::getAllocator().init(ps.points, ps.poly, ps.he);
	PicPainter::getAllocator().clearMaxAlloc();

	int cnt = 0;
	bool cancel = false;
	for (int g = 0; (g < numDefs()) && (!cancel); ++g)
	{
		PicGroupDef *gp = &(defs[g]);
		for (int p = 0; (p < gp->numPics()) && (!cancel); ++p)
		{
            pdlg.setValue(cnt++);
			gp->pics[p].painter.init(dpc, mainContext);

            qApp->processEvents(); // must call
			cancel = ((showStop && pdlg.wasCanceled()) || (((MainWindow*)g_main)->wasClosed()));
		}
	}
    pdlg.setValue(cnt);
	PicPainter::getAllocator().checkMaxAlloc();
}


int PicBucket::selectedCount() const
{
	int g,p;
	int count = 0;
	for (g = 0; g < numDefs() ; ++g)
	{
		const PicGroupDef *gp = &(defs[g]);
		for (p = 0; p < gp->numPics(); ++p)
		{
			count += gp->pics[p].getSelected();
		}
	}
	return count;
}


void PicType::load(int _gind, int _pind, bool bCsym)
{
	PicArr tmppic;
	int i,j;

	gind = _gind; // members
	pind = _pind;
	thedef = &(PicBucket::instance().defs[gind].pics[pind]);
	thegrp = &(PicBucket::instance().defs[gind]);

	bool bSym = (bCsym && isIndividual(thegrp->drawtype)); // determine if it's symetric according to if it's half-texturized
		
	thedef->v.copyTo(tmppic);
		
	tmppic.turned = true;
	for(i = 0; i < 8; ++i)
	{ // make all it's rotations
		tmppic.copyTo(rtns[i]);
		rtns[i].rtnindx = i;
		if (i != 3) 
			tmppic.turn();
		else 
		{
			tmppic.turned = false;
			tmppic.revY();
		}
	}
	rtnnum = 8;

	bool dflag[8] = { false };
	for(i = 0; i < 7; ++i)
	{
		for(j = i + 1; j < 8; ++j)
		{
			if (rtns[i].equalTo(rtns[j], bSym))
				dflag[j] = true;
		}
	}
	for(i = 7; i >= 0; --i)
	{
		if (dflag[i]) 
		{
			rtnnum--;
			for(j = i; j < 7; j++)
				rtns[j + 1].copyTo(rtns[j]);
		}
	}
}


void PicArr::turn()
{
	int tx = 4 ,ty = 0;
	PicArr tmp;
	tmp.turned = turned;

	for(int y = 0; y < 5; ++y)
	{
		for(int x = 0; x < 5; ++x) 
		{
			tmp.set(tx,ty) = axx(x,y);
			ty = ty+1;
		}
		ty = 0;
		tx = tx-1;
	}
	tmp.copyTo(*this);
}

void PicArr::revY()
{
	int ty = 4;
	PicArr tmp;
	tmp.turned = turned;

	for(int y = 0; y < 5; ++y) 
	{
		for(int x = 0; x < 5; ++x)
			tmp.set(x,ty) = axx(x,y);
		ty = ty-1;
	}
	tmp.copyTo(*this);
}

bool PicArr::equalTo(const PicArr &dest, bool bSym) const
{
	for(int x = 0; x < 5; ++x)
	{
		for(int y = 0; y < 5; ++y)
		{
			if (dest.axx(x,y) != axx(x,y)) return false;
		}
	}

	if (bSym && (turned != dest.turned))
		return false;

	return true;
}



void PicArr::copyTo(PicArr &dest) const
{
	for(int x = 0; x < 5; ++x)
	{ 
		for(int y = 0; y < 5; ++y)
		{
			dest.set(x,y) = axx(x,y);
		}
	}
	dest.turned = turned;
	dest.rtnindx = rtnindx;
}


PicsSet::PicsSet(const SlvCube *scube)
	: bConsiderSymetric(scube->bConsidersSym), pics(scube->picssz)
{
	
	for (int i = 0; i < scube->picssz; ++i)
	{	
		pics[i].load(scube->picdt[i].gind, scube->picdt[i].pind, bConsiderSymetric);
	}

	makereps();
}


PicsSet::PicsSet(bool bSym) 
	: bConsiderSymetric(bSym)
{
	int idef, ipic, pn = 0;
	const PicBucket &bucket = PicBucket::instance();
	for(idef = 0; idef < bucket.numDefs(); ++idef) 
	{
		for(ipic = 0; ipic < bucket.defs[idef].numPics(); ++ipic)
		{
			pn += bucket.defs[idef].pics[ipic].getSelected();
		}
	}
	
	int pi = 0; //pi -- the current pic loaded

	pics.resize(pn);
	for(idef = 0; idef < bucket.numDefs(); ++idef) 
	{
		for(ipic = 0; ipic < bucket.defs[idef].numPics(); ++ipic)
		{
			// if the piece is selected a number of times, load it that number
			// of times in to the set.
			for (int i = 0; i < bucket.defs[idef].pics[ipic].getSelected(); ++i)
			{
				pics[pi++].load(idef, ipic, bConsiderSymetric);
			}
		}
	}

	makereps();

}

///	Construct the repetitions of pieces.
/// construct the "rep" member of PicType. rep is an array of containing the indexes of
/// all other pics that are similar in shape to the current one.
void PicsSet::makereps()
{
	// temp repetitions
	int *trep = new int[PicBucket::instance().sumPics];

	for(int i = 0; i < size(); ++i)
	{
		// setting the repetitions of i
		pics[i].repnum = 0;
		int j;
		for(j = 0; j < size(); ++j)
		{
			// checking if j is the same
			// we need to have the comparison with 'turned' only if both i and j are 
			// asymmetric since only in this case pieces can't cover for one another
			bool bSym = bConsiderSymetric && 
				isIndividual(pics[i].thegrp->drawtype) &&
				isIndividual(pics[j].thegrp->drawtype);
				
			int rt = 0;
			bool found = false;
			while ((rt < pics[i].rtnnum) && (!found))
			{
				found = pics[j].rtns[rt].equalTo(pics[i].rtns[0], bSym); // sure there is a need for turned
				rt++;
			}
			if (found) 
			{
				trep[pics[i].repnum++] = j;
			}
		}

		pics[i].rep = new int[pics[i].repnum];
		for(j = 0; j < pics[i].repnum; ++j)
		{
			pics[i].rep[j] = trep[j];
		}
	}

	delete trep;
}

