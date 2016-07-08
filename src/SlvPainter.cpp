#include "SlvPainter.h"
#include "Pieces.h"
#include "BaseGLWidget.h"
#include "Shape.h"
#include "SlvCube.h"
#include "OpenGL/glGlob.h"
#include "OpenGL/Shaders.h"
#include "CubeDocBase.h"

extern int g_testAngle[];

SlvPainter::SlvPainter(const SlvCube* _scube) :m_scube(_scube) 
{
}

void SlvPainter::makeMats() 
{
    if (m_scube == nullptr)
        return;
    // this disables moving the angle for now. need a special event for it. don't do it every frame
    m_scube->shape->makeTransformsMatrics(g_testAngle[0], m_mats);
}

// TBD - GLControl
void SlvPainter::paintPiece(int f, BaseGLWidget* context, bool fTargets) const
{
    const PicDef *pdef = m_scube->dt[f].sdef();
    Shape::FaceDef *face = &m_scube->shape->faces[f];

    auto cd = m_scube->shape->scdrToBuildCoord(face->ex, face->dr);
    auto bldtype = CubeDocBase::s_instance->getBuild().get(cd);
    if (GET_TYPE(bldtype) != TYPE_REAL)
        return; // the piece is not in the build shape (happens in a generate error), don't display the solution piece

    //int rtnindx = m_scube->dt[f].abs_rt;
    //rtnindx += pdef->dispRot;
    int rtnindx = rotationSub(m_scube->dt[f].abs_rt, pdef->dispRot);

    MatStack& model = context->model;
    //cout << "START-MODEL " << model.cur() << endl;

    model.push();

    model.translate(0.5,0.5,-0.5);
    model.cur().mult(m_mats[f]);
    model.translate(-0.5,-0.5,0.5);
    model.rotate(90, 0, 1, 0);


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

    int flag = 0;
    if (f < CubeDocBase::s_instance->m_flagPiece.size()) // can happen if not generated yet
        flag = CubeDocBase::s_instance->m_flagPiece[f]; // flag to be marked in red for remove

    mglCheckErrorsC("x6");
    // if dispRot >= 4 it means the real part we're drawing is inverted from the model so we need to draw the texture on the other side
    bool isFlipped = (pdef->dispRot >= 4);
    // (for blake piece lighting) flip XOR facing out, if its flipped we want to light the other face in absolue piece coordinates
	// need the rtnindx flip since we're rendering the unified mesh
    bool faceOut = (rtnindx >= 4) == (face->facing == Shape::FACING_IN);
    PicPainter(pdef).paint(fTargets, name, context, isFlipped, flag, pdef->dispRot, faceOut );
    mglCheckErrorsC("x7");

    model.pop();
}



void SlvPainter::paintLines(int f, bool singleChoise, BaseGLWidget *context, ELinesDraw cfgLines) const
{
    if (f >= m_linesIFS.m_bufs.size())
        return; // happens in a transferred solution. fix?

    NoiseSlvProgram* prog = ShaderProgram::currentt<NoiseSlvProgram>();
    prog->trans.set(context->transformMat());
    prog->drawtype.set(DRAW_FLAT);
    prog->colorAu.set(Vec3(0.8f, 0.8f, 0.8f));

    glPolygonOffset(0.0, 0.0); // go forward, draw the lines

    m_linesIFS[f].paint();

}

void SlvPainter::paint(BaseGLWidget* context, bool fTargets, int singleChoise, int upToStep, ELinesDraw cfgLines) const
{
    M_ASSERT(m_scube != nullptr);
    M_ASSERT(m_scube->shape != nullptr);
    context->model.translate(0,0,+1);

    //cout << "*****" << endl;

    if (singleChoise < 0)
    {
        for (int f = 0; f < m_scube->dt.size(); ++f)
        {
            if (upToStep >= 0 && f >= upToStep) // step by step support
                break;
            if (m_scube->dt[f].abs_sc == -1) // piece not there (solution transformed)
                continue;
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
                    const int *knei = m_scube->shape->faces[f].nei;
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
    const PicDef *pdef = m_scube->dt[f].sdef();
    Shape::FaceDef *face = &m_scube->shape->faces[f];
    //int rtnindx = m_scube->dt[i].abs_rt;
    int rtnindx = rotationSub(m_scube->dt[f].abs_rt, pdef->dispRot);

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
    return PicPainter(pdef).exportToObj(oe, curMatrix);

}


bool SlvPainter::exportToObj(ObjExport& oe) const
{
    for (int f = 0; f < m_scube->dt.size(); ++f) {
        exportPieceToObj(oe, f);
    }
    return true;
}