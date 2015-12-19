#include "BuildGLControl.h"
#include "CubeDoc.h"

#include <QMouseEvent>
#include <QTimer>

#include <iostream>
using namespace std;


BuildGLControl::BuildGLControl(GLWidget *gl, CubeDoc *doc)
    :BuildControlBase(gl, doc), m_gl(gl)
{
    if (m_bBoxedMode)
    {
        m_fadeTimer = new QTimer(this);
        connect(m_fadeTimer, SIGNAL(timeout()), this, SLOT(fadeTimeout()));
    }
}


void BuildGLControl::switchIn()
{
    BuildControlBase::switchIn();
    m_fadeTimer->start(25);
}

void BuildGLControl::switchOut()
{
    BuildControlBase::switchOut();
    m_fadeTimer->stop();
}


void BuildGLControl::slvProgStatsUpdate(int hint, int data)
{
    Q_UNUSED(data)
    switch (hint)
    {
    case SHINT_START: enableEdit(false); break;
    case SHINT_STOP: enableEdit(true); break;
    }
}

void BuildGLControl::keyEvent(QKeyEvent *event)
{
    m_bInternalBoxRemove = ((event->modifiers() & Qt::ControlModifier) != 0);
    if (doMouseMove(-1, -1)) // simulate a mouse move
        m_gl->updateGL();
    emit changedAction(isInRemove());
}


void BuildGLControl::changeAction(bool remove) // someone is saying its changed
{
    if (remove == hXor(m_bBoxRemove, m_bInternalBoxRemove))
        return;

    m_bBoxRemove = remove;
    if (doMouseMove(-1, -1)) // simulate a mouse move
        m_gl->updateGL();
}

void BuildGLControl::fadeTimeout()
{
    BuildWorld& build = m_doc->getBuild();
    if ((!m_inFade) && (build.getTestResult() != GEN_RESULT_ILLEGAL_SIDE))
        return;


    if (m_inFade) {
        m_fadeFactor += 0.2;
        if (m_fadeFactor >= 1.0)
            m_inFade = false;
    }

    if (build.getTestResult() == GEN_RESULT_ILLEGAL_SIDE)
    {
        m_errCylindrAlpha += m_errCylindrAlphaDt;
        if ((m_errCylindrAlpha >= 1.0) || (m_errCylindrAlpha <= 0.1))
            m_errCylindrAlphaDt = -m_errCylindrAlphaDt;
    }
    //printf("fade update\n");
    m_gl->updateGL();

}

void BuildGLControl::updateView(int hint)
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
        m_gl->doReset();

    case HINT_BLD_PAINT:
        //update();
        m_gl->updateGL();
        break;
    }
}
 
extern int g_testAngle[];
extern bool doPrint;

void BuildGLControl::changeRotAngle(int a)
{
    g_testAngle[0] = g_testAngle[1] = g_testAngle[2] = a;
    
   // cout << a << endl;
    m_doc->getBuild().setUnTest();
    //doPrint = false;
    checkSides();
    //doPrint = true;
    makeBuffers();
    m_gl->updateGL();
}
