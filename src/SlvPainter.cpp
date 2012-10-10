#include "SlvPainter.h"
#include "Pieces.h"
#include "GLWidget.h"
#include "Shape.h"
#include "SlvCube.h"
#include "OpenGL/glGlob.h"


void SlvPainter::paintPiece(int f, GLWidget* context, bool fTargets) const
{
	const PicDef *pdef = scube->dt[f].sdef;
	Shape::FaceDef *face = &scube->shape->faces[f];

	//int rtnindx = scube->dt[f].abs_rt;
	//rtnindx += pdef->dispRot;
	int rtnindx = rotationSub(scube->dt[f].abs_rt, pdef->dispRot);

	glPushMatrix();

	glTranslated(face->ex.x, face->ex.y, face->ex.z);

	switch (face->dr)
	{
	case XY_PLANE: 
		glTranslated(0, 0, 1);
		glRotated(90, 0, 1, 0); 
		break;
	case XZ_PLANE: 
		glRotated(90, 0, 1, 0);
		glRotated(90, 0, 0, 1); 
		break;
	case YZ_PLANE: 
		break;
	}

	glTranslatef(0.5, 2.5, 2.5);
	glRotated(rtnindx * -90, 1, 0, 0);

	if (rtnindx >= 4)
	{
		glRotated(180, 0, 0, 1);
		glRotated(90, 1, 0, 0);
	}

	glTranslatef(-0.5, -2.5, -2.5);

	//glLoadName(f);
	Vec3b name = Vec3b::fromName(f);
	//printf("%d %d %d\n", name.x, name.y, name.z);
	//glColor3bv((GLbyte*)name.v);
	glColor3f(name.x/255.0, name.y/255.0, name.z/255.0);
	mglCheckErrors("~x6");
	pdef->painter.paint(fTargets, context);
	mglCheckErrors("~x7");
	glPopMatrix();
}

void SlvPainter::paintLines(const MyObject &obj, bool singleChoise, GLWidget *context, ELinesDraw cfgLines) const
{
	if (context->isUsingLight())
		glDisable(GL_LIGHTING);

	glPolygonOffset(0.0, 0.0); // go forward, draw the lines

	glBegin(GL_LINES);

	for (int lni = 0; lni < obj.nLines; ++lni)
	{
		MyLine &curln = obj.lines[lni];

		if ( ((curln.type == MyLine::LINE_ONLY_LONE) && (!singleChoise)) || 
			 ((curln.type == MyLine::LINE_ONLY_WHOLE) && (singleChoise)) )
			continue;

		if ((cfgLines == LINES_WHITE) && (curln.color[0] < 0.1))
			continue;


		if (singleChoise) // means we're alone now
			glColor3f(obj.nakedLinesColor, obj.nakedLinesColor, obj.nakedLinesColor);
		else
			glColor3fv(curln.color.ptr());

		glVertex3fv(curln.p1->p.ptr());
		glVertex3fv(curln.p2->p.ptr());

	} // for lni

	glEnd();


	if (context->isUsingLight())
		glEnable(GL_LIGHTING);

}

void SlvPainter::paint(GLWidget* context, bool fTargets, int singleChoise, int upToStep, ELinesDraw cfgLines) const
{
	if (singleChoise < 0)
	{
		for (int f = 0; f < scube->dt.size(); ++f)
		{
			if ((upToStep >= 0) && (f >= upToStep)) // step by step support
				break;

			paintPiece(f, context, fTargets);

			if ((!fTargets) && (cfgLines != LINES_NONE))
			{
				bool linesSingle = false;
				if (upToStep >= 0)  // step by step support
				{
					// this is somewhat of a PATCH that doesn't work completely well to make the edges have somewhat proper lines
					// since we don't want to regenerate to IFS for every stage (and even that doesn't work so well, see flat10x10)
					const int *knei = scube->shape->faces[f].nei;
					linesSingle = (knei[0] > upToStep) || (knei[1] > upToStep) || (knei[2] > upToStep) || (knei[3] > upToStep);
				}

				paintLines(m_linesIFS[f], linesSingle, context, cfgLines);
			}
		}
	}
	else
	{
		paintPiece(singleChoise, context, fTargets);
		if ((!fTargets) && (cfgLines == LINES_ALL)) // in single choise, do lines only if ALL (and not if BLACK)
			paintLines(m_linesIFS[singleChoise], true, context, cfgLines);
	}
}



bool SlvPainter::exportPieceToObj(QTextStream& meshFile, QTextStream& materialsFiles, int i, unsigned int& numVerts,
								  unsigned int &numTexVerts, unsigned int &numNormals, unsigned int &numObjs) const
{
	const PicDef *pdef = scube->dt[i].sdef;
	Shape::FaceDef *face = &scube->shape->faces[i];
	int rtnindx = scube->dt[i].abs_rt;

	Mat4 curMatrix = Mat4::translation(face->ex.x, face->ex.y, face->ex.z);

	switch (face->dr)
	{
	case XY_PLANE: 
		curMatrix.translate(0, 0, 1);
		curMatrix.rotate(90, 0, 1, 0);
		break;
	case XZ_PLANE: 
		curMatrix.rotate(90, 0, 1, 0);
		curMatrix.rotate(90, 0, 0, 1);
		break;
	case YZ_PLANE: 
		// no need to do any rotation
		break;
	}

	curMatrix.translate(0.5, 2.5, 2.5);
	curMatrix.rotate(rtnindx * -90, 1, 0, 0);

	if (rtnindx >= 4)
	{
		curMatrix.rotate(180, 0, 0, 1);
		curMatrix.rotate(90, 1, 0, 0);
	}

	curMatrix.translate(-0.5, -2.5, -2.5);
	return pdef->painter.exportToObj(meshFile, materialsFiles, numVerts, numTexVerts, numNormals, numObjs, curMatrix);

}


bool SlvPainter::exportToObj(QTextStream& meshFile, QTextStream& materialsFiles) const
{
	unsigned int numVerts = 1, numTexVerts = 1, numNormals = 0, numObjs = 0;
	for (int f = 0; f < scube->dt.size(); ++f)
	{
		exportPieceToObj(meshFile, materialsFiles, f, numVerts, numTexVerts, numNormals, numObjs);
	}
	return true;
}