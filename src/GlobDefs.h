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

#ifndef __GLOBDEFS_H_INCLUDED__
#define __GLOBDEFS_H_INCLUDED__

#include <QtGlobal> 

/** \file
	Defines various macros and global variables used by many files.
*/

/// used in message boxes
#define APP_NAME QObject::tr("Happy Cube Solver")

class QWidget;
/// this is a global pointer to the main MainWindow instance. it is used in places where
/// there is no easy reach to a parent window. 
/// This pointer is set in the main() and instantiated in main.cpp
extern QWidget* g_main;

#if (defined(Q_WS_MAC) || defined(Q_WS_X11))
 // OSX has special sizes for some things
#define MIN_HLP_PANE_WIDTH 271
#define MIN_BOT_HEIGHT 35
#else
#define MIN_HLP_PANE_WIDTH 245
#define MIN_BOT_HEIGHT 30
#endif


#define MAX_IDENTICAL_PIECES 10000 // maximum number of identical pics permitted


#define WITH_EXPORT_OPTION

#endif
