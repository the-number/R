/*
    Copyright (C) 2009 John Darrington
    Copyright (c) 2004 Dale Mellor,  based on work which is
    Copyright (C) 1998,  2003  John Darrington

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

#ifndef CUBE_I_H
#define CUBE_I_H


#ifndef CUBE_H
#    error This is a private header file and should not be included directly in any user source code.
#endif



#include "txfm.h"




typedef struct _Face
{
  /* An array of vectors showing the orientation of the blocks. There are four
     vectors per face. These are used to orientate the mouse pointer,  to
     detect when the cube has been solved and to provide feedback to clients
     querying the state. */
  vector quadrants[4];

  /* The normal vector is orthogonal to the face of the block. When all
     normals are in the same direction,  the cube colours are correct,  but not
     necessarily their orientations. */
  vector normal;

  /* The position of the centre of the face,  relative to the centre of the
     block. */
  point centre;

} Face;


typedef struct _Block
{
  /* Bit-field indicating which faces are on the surface of the cube,  and
     should therefore be rendered to the framebuffer. */
  unsigned int visible_faces;

  /* A set of attributes for each face (including internal ones!) */
  Face face[6];

  /* The position from the centre of the cube,  and the rotation from the
     'natural' position (note that the location vector is accessed as
     transformation+12). */
  Matrix transformation;

} Block;



#define BLOCK_ERROR_MESSAGE_BUFFER_LEN 255



struct cube
{
  /* The number of blocks on each side of the cube */
  int size[3];

  /* cube_size ** 3 */
  int number_blocks;

  /* A set of attributes for every block (including internal ones!) */
  Block *blocks;
};



#endif /* Defined CUBE_I_H. */
