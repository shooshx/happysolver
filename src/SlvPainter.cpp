#include "SlvPainter.h"
#include "Pieces.h"
#include "GLWidget.h"
#include "Shape.h"
#include "SlvCube.h"


void SlvPainter::paintPiece(int f, GLWidget* context, bool fTargets) const
{
	const PicDef *pdef = scube->dt[f].sdef;
	Shape::FaceDef *face = &scube->shape->faces[f];
	int rtnindx = scube->dt[f].abs_rt;

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

	glLoadName(f);
	pdef->painter.paint(fTargets, context);


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


// C = A * B
void MultiplyMatrices(const float matA[16], float matB[16], float matC[16])
{
	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
		{
			float res = 0.0f;
			for (unsigned int k = 0; k < 4; ++k)
			{
				res += matA[i + k * 4] * matB[k + j * 4];
			}
			matC[i + j * 4] = res;
		}
	}
}

void CopyMatrix(const float from[16], float to[16])
{
	for (unsigned int i = 0; i < 16; ++i)
	{
		to[i] = from[i];
	}
}

void RotationMatrix(float m[16], float angle, float x, float y, float z)
{
	float c = cos(angle / 180.0f * M_PI);
	float s = sin(angle / 180.0f * M_PI);
	m[0] = x * x * (1 - c) + c    ; m[4] = x * y * (1 - c) - z * s; m[8 ] = x * z * (1 - c) + y * s; m[12] = 0.0f;
	m[1] = y * x * (1 - c) + z * s; m[5] = y * y * (1 - c) + c;		m[9 ] = y * z * (1 - c) - x * s; m[13] = 0.0f;
	m[2] = x * z * (1 - c) - y * s; m[6] = y * z * (1 - c) + x * s; m[10] = z * z * (1 - c) + c;	 m[14] = 0.0f;
	m[3] = 0.0f;					m[7] = 0.0f;					m[11] = 0.0f;					 m[15] = 1.0f;
}

void TranslationMatrix(float m[16], float x, float y, float z)
{
	m[0] = 1.0f; m[4] = 0.0f; m[8 ] = 0.0f; m[12] = x;
	m[1] = 0.0f; m[5] = 1.0f; m[9 ] = 0.0f; m[13] = y;
	m[2] = 0.0f; m[6] = 0.0f; m[10] = 1.0f; m[14] = z;
	m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
}

bool SlvPainter::exportPieceToObj(QTextStream& meshFile, QTextStream& materialsFiles, int i, unsigned int& numVerts,
								  unsigned int &numTexVerts, unsigned int &numNormals, unsigned int &numObjs) const
{
	const PicDef *pdef = scube->dt[i].sdef;
	Shape::FaceDef *face = &scube->shape->faces[i];
	int rtnindx = scube->dt[i].abs_rt;

//	glPushMatrix();

	float curMatrix[16], A[16], B[16];

//	glTranslated(face->ex.x, face->ex.y, face->ex.z);
	TranslationMatrix(curMatrix, face->ex.x, face->ex.y, face->ex.z);

	switch (face->dr)
	{
	case XY_PLANE: 
//		glTranslated(0, 0, 1);
		CopyMatrix(curMatrix, A);
		TranslationMatrix(B, 0, 0, 1);
		MultiplyMatrices(A, B, curMatrix);
		
//		glRotated(90, 0, 1, 0); 
		CopyMatrix(curMatrix, A);
		RotationMatrix(B, 90, 0, 1, 0);
		MultiplyMatrices(A, B, curMatrix);
		break;
	case XZ_PLANE: 
//		glRotated(90, 0, 1, 0);
		CopyMatrix(curMatrix, A);
		RotationMatrix(B, 90, 0, 1, 0);
		MultiplyMatrices(A, B, curMatrix);


//		glRotated(90, 0, 0, 1); 
		CopyMatrix(curMatrix, A);
		RotationMatrix(B, 90, 0, 0, 1);
		MultiplyMatrices(A, B, curMatrix);
		break;
	case YZ_PLANE: 
		break;
	}

//	glTranslatef(0.5, 2.5, 2.5);
	CopyMatrix(curMatrix, A);
	TranslationMatrix(B, 0.5, 2.5, 2.5);
	MultiplyMatrices(A, B, curMatrix);

//	glRotated(rtnindx * -90, 1, 0, 0);
	CopyMatrix(curMatrix, A);
	RotationMatrix(B, rtnindx * -90, 1, 0, 0);
	MultiplyMatrices(A, B, curMatrix);

	if (rtnindx >= 4)
	{
//		glRotated(180, 0, 0, 1);
		CopyMatrix(curMatrix, A);
		RotationMatrix(B, 180, 0, 0, 1);
		MultiplyMatrices(A, B, curMatrix);

//		glRotated(90, 1, 0, 0);
		CopyMatrix(curMatrix, A);
		RotationMatrix(B, 90, 1, 0, 0);
		MultiplyMatrices(A, B, curMatrix);
	}

//	glTranslatef(-0.5, -2.5, -2.5);
	CopyMatrix(curMatrix, A);
	TranslationMatrix(B, -0.5, -2.5, -2.5);
	MultiplyMatrices(A, B, curMatrix);

//	glLoadName(f);
	return pdef->painter.exportToObj(meshFile, materialsFiles, numVerts, numTexVerts, numNormals, numObjs, curMatrix);


//	glPopMatrix();
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