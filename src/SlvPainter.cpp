#include "SlvPainter.h"
#include "Pieces.h"
#include "GLWidget.h"
#include "Shape.h"
#include "SlvCube.h"
#include "OpenGL/glGlob.h"
#include "OpenGL/Shaders.h"


void SlvPainter::paintPiece(int f, GLWidget* context, bool fTargets) const
{
	const PicDef *pdef = scube->dt[f].sdef;
	Shape::FaceDef *face = &scube->shape->faces[f];

	//int rtnindx = scube->dt[f].abs_rt;
	//rtnindx += pdef->dispRot;
	int rtnindx = rotationSub(scube->dt[f].abs_rt, pdef->dispRot);

	MatStack& model = context->model;

	model.push();
	model.translate(face->ex.x, face->ex.y, face->ex.z);

	switch (face->dr)
	{
	case XY_PLANE: 
		model.translate(0, 0, 1);
		model.rotate(90, 0, 1, 0); 
		break;
	case XZ_PLANE: 
		model.rotate(90, 0, 1, 0);
		model.rotate(90, 0, 0, 1); 
		break;
	case YZ_PLANE: 
		break;
	}

	model.translate(0.5, 2.5, 2.5);
	model.rotate(rtnindx * -90, 1, 0, 0);

	if (rtnindx >= 4) {
		model.rotate(180, 0, 0, 1);
		model.rotate(90, 1, 0, 0);
	}

	model.translate(-0.5, -2.5, -2.5);


	Vec3b nameb = Vec3b::fromName(f + 1);
	Vec3 name = Vec3(nameb.x/255.0, nameb.y/255.0, nameb.z/255.0);
	//printf("%d %d %d\n", name.x, name.y, name.z);

	mglCheckErrorsC("x6");
	// if dispRot >= 4 it means the real part we're drawing is inverted from the model so we need to draw the texture on the other side
	pdef->painter.paint(fTargets, name, context, (pdef->dispRot < 4) );
	mglCheckErrorsC("x7");

	model.pop();
}



void SlvPainter::paintLines(int f, bool singleChoise, GLWidget *context, ELinesDraw cfgLines) const
{
	NoiseSlvProgram* prog = ShaderProgram::currentt<NoiseSlvProgram>();
	prog->trans.set(context->transformMat());
	prog->drawtype.set(DRAW_FLAT);
	prog->colorAu.set(Vec3(0.8f, 0.8f, 0.8f));

	glPolygonOffset(0.0, 0.0); // go forward, draw the lines

	m_linesIFS[f].paint();

}

void SlvPainter::paint(GLWidget* context, bool fTargets, int singleChoise, int upToStep, ELinesDraw cfgLines) const
{
	if (singleChoise < 0)
	{
		for (int f = 0; f < scube->dt.size(); ++f)
		{
			if ((upToStep >= 0) && (f >= upToStep)) // step by step support
				break;
			mglCheckErrorsC("x3");
			paintPiece(f, context, fTargets);
			mglCheckErrorsC("x4");

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

				paintLines(f, linesSingle, context, cfgLines);
				mglCheckErrorsC("x5");
			}
		}
	}
	else
	{
		paintPiece(singleChoise, context, fTargets);
		if ((!fTargets) && (cfgLines == LINES_ALL)) // in single choise, do lines only if ALL (and not if BLACK)
			paintLines(singleChoise, true, context, cfgLines);
	}
}



bool SlvPainter::exportPieceToObj(ObjExport& oe, int f) const
{
	const PicDef *pdef = scube->dt[f].sdef;
	Shape::FaceDef *face = &scube->shape->faces[f];
	//int rtnindx = scube->dt[i].abs_rt;
	int rtnindx = rotationSub(scube->dt[f].abs_rt, pdef->dispRot);

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

	if (rtnindx >= 4) {
		curMatrix.rotate(180, 0, 0, 1);
		curMatrix.rotate(90, 1, 0, 0);
	}

	curMatrix.translate(-0.5, -2.5, -2.5);
	return pdef->painter.exportToObj(oe, curMatrix);

}


bool SlvPainter::exportToObj(ObjExport& oe) const
{
	for (int f = 0; f < scube->dt.size(); ++f) {
		exportPieceToObj(oe, f);
	}
	return true;
}