/*
    routines which actually draw the blocks of the cube.
    Copyright (C) 1998, 2011  John Darrington

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License,  or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DRW_BLOCKS_H
#define DRW_BLOCKS_H

#include <GL/gl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "cube.h"
#include "cubeview.h"



/* The texture names for each face */
void drawCube (GbkCube * cube, GLboolean ancilliary, GbkCubeview * cv);

#endif
