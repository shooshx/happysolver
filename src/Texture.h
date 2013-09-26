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

#ifndef __TEXTURE_H__INCLUDED__
#define __TEXTURE_H__INCLUDED__

//#include <QtOpenGL>
#include <QImage>

/** \file
	Declares the Texture class.
*/

/** Texture contains the data of a texture in the 3D solution viewer.
	It holds the original QImage of the texture and its index in GLWidget tables.
*/
class Texture									
{
public:
	Texture()  {}
	Texture(const QImage& _img) :img(_img) {}

	QImage img;
	//int ind;	///< index of for the textures table in the GLWidget where we should find the proper id.

	/// is this a real texture that is bound in the GL context or not?
	/// for instance, the big Little Genius texture is not bound to a GL context.
	/// only small subsets of it are.
	//bool bound; 
};	


#endif // __TEXTURE_H__INCLUDED__
