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

#include "MainWindow.h"
#include "Pieces.h"

#include <QApplication>
#include <QIcon>
#include <QtGui>
#include <iostream>
#include <fstream>

#include "Quaternion.h"


QWidget* g_main = nullptr;


#if (defined(Q_WS_MAC) || defined(Q_WS_X11)) // on mac OS the mng plugin is static
//Q_IMPORT_PLUGIN(QMngPlugin) //mng plugin as static
#endif

extern QGLFormat g_format;

#include <QDialog>
#include <QVBoxLayout>
#include <QGroupBox>

using namespace std;

/*
static int rotationAdd(int base, int defRot) 
{
    if ((defRot < 4) == (base < 4))
        return (base - defRot + 4) % 4;
    else
        return ((base + defRot) % 4) + 4;

}

static int rotationSub(int x, int defRot)
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
*/

#ifdef _WINDOWS
void console()
{
    AllocConsole();
    FILE *pFileCon = nullptr;
    pFileCon = freopen("CONOUT$", "w", stdout);
    pFileCon = freopen("CONIN$", "r", stdin);

    COORD coordInfo;
    coordInfo.X = 300;
    coordInfo.Y = 3000;

    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coordInfo);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE),ENABLE_QUICK_EDIT_MODE| ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT);
}
#endif


void makeArrowObject(ostream& os);
void makeSphere(ostream& os);

void mainQuadObj() {
    //ofstream af("C:/projects/cubeGL/happysolver/arrow.obj");
    //makeArrowObject(af);

    ofstream af("C:/projects/cubeGL/happysolver/sphere.obj");
    makeSphere(af);
    getchar();
}

void testQuat() 
{
    Mat4 m;
    m.identity();
    m.rotate(45, 1, 0, 0);
    //m.transpose();

    Quaternion q = Quaternion::fromMat(m);

    Mat4 m2 = q.toMatrix();

    Quaternion q2 = Quaternion::fromMat(m2);

    getchar();
}

void testQuat2()
{
    Mat4 m1, m2, m3;
    m1.identity();
    m1.rotate(0, 1, 0, 0);

    m2.identity();
    m2.rotate(90, 1, 0, 0);

    m3.identity();
    m3.rotate(90, 0, 1, 0);

    Quaternion q1 = Quaternion::fromMat(m1);
    Quaternion q2 = Quaternion::fromMat(m2);
    Quaternion q3 = Quaternion::fromMat(m3);

    Quaternion t;
    t = Quaternion::slerp(q1, q2, 0);
    t = Quaternion::slerp(q1, q2, 1);
    t = Quaternion::slerp(q1, q2, 0.5);

    t = Quaternion::slerp(q1, q3, 0);
    t = Quaternion::slerp(q1, q3, 1);
    t = Quaternion::slerp(q1, q3, 0.5);



    Vec3 v0(0,1,0);
    Vec3 v_s = m1.transformVec(v0);
    Vec3 v_e = m2.transformVec(v0);
    for(double t = 0; t < 1.0; t += 0.1)
    {
        auto qs = Quaternion::slerp(q1, q2, t);
        Mat4 ms = qs.toMatrix();
        Vec3 v = ms.transformVec(v0);
        cout << v << endl;

    }


    getchar();
}



int main(int argc, char *argv[])
{

// 	int res[8][8];
// 	for(int base = 0; base < 8; ++base) {
// 		for(int rot = 0; rot < 8; ++rot) {
// 			int x = rotationAdd(base, rot);
// 			int e = rotationSub(x, rot);
// 
// 			printf("%d + %d = %d -> %d\n", base, rot, x, e);
// 
// 			res[rot][x] = base;
// 		}
// 		printf("\n");
// 	}
// 
// // 	for(int x = 0; x < 8; ++x) {
// // 		for(int rot = 0; rot < 8; ++rot) {
// // 			int e = rotationSub(x, rot);
// // 			printf("%d - %d = %d : %d\n", rot, x, res[x][rot], e);
// // 		}
// // 		printf("\n");
// // 	}
// 
// 	return 0;
    console();

    mainQuadObj();
    return 0;

    //PicBucket::buildAllMeshes();
    //return 1;

    QApplication app(argc, argv);

    QIcon appicon(":/images/HappySolver64f.png");
    appicon.addFile(":/images/HappySolver32f.png");
    appicon.addFile(":/images/HappySolver16f.png");
    app.setWindowIcon(appicon);

    g_format.setSampleBuffers(true); // TBD - do better
    PicBucket::createSingleton();



    MainWindow window;
    window.setWindowIcon(appicon);
    g_main = &window;
    

    // do the icon as fast as possible.

    flushAllEvents();


    if (!window.initialize())
        return 0;
    // window could have been closed during initlization. test it.
    if (window.wasClosed())
        return 0;

    QString filename;
    for (int i = 1; i < argc; ++i)
    {
        cout << "arg " << i << argv[i] << endl;
        if (argv[i][0] != '-')
        {
            filename = argv[i];
            break;
        }
    }
    if (!filename.isEmpty())
    {
        cout << "Opening file " << filename.toLatin1().data() << endl;
        window.loadStartupFile(filename);
    }

    return app.exec();
}

/** \mainpage
    \section sec_intro Introduction
    The Happy %Cube Solver is an interactive environment based on OpenGL for designing 
    and building cubes and formations from the Happy %Cube 3D puzzles. 
    It integrates a graphic engine for creating and viewing formations and a powerful 
    computational engine for solving them.

    Happy Cubes are small foamy puzzles made of pieces of about 4cm by 4cm by 8 mm in size
    The puzzles come in groups of 6 such pieces of the same color or texture. When
    assembled, these pieces make up a basic cube of about 4cm by 4cm by 4cm in size.
    Pieces of several different Happy %Cube puzzles can be joined together to make up
    larger and more complex structures. Such a sturcture is for instance the "double tower" 
    which is made of 10 pieces and is shaped like two cubes one on top of the other.
    another example for a complex structure is the "star" which is made of 30 pieces
    arranged as 6 cubes joined at a common missing face.

    see <a href="http://www.happycube.com/">The Happy %Cube Website</a> for more information
    about the Happy %Cube puzzles.

    Normally, designing and building a complex structure can be a long and daunting
    task due to the large amount of pieces and assembly possibilities. The Happy %Cube Solver
    takes away the pain of this task and leaves the fun parts of designing the structure
    and assembling it out of Happy %Cube pieces.
    The Happy %Cube Solver allows the user to design the structures he wishes to build 
    in a full 3D space. The user can rotate and view the design in any angle and add or remove
    cubes to it. Once the structure is complete, the user can select the pieces set of which 
    to assemble out of the large variety of available Happy %Cube pieces.
    this set should coincide with the set of pieces the user actually own physically for later
    assembly.
    Pressing "Solve It" in the main GUI would start the solution engine. After a few seconds
    to few minutes, depending on the structure size and piece set, The solver will find
    a solution to the structure and again, display it in full 3D space for the user to rotate
    and view.

    This manual is the code documentation of the Happy %Cube Solver.
*/
