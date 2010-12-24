/*
    GNUbik -- A 3 dimensional magic cube game.
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
#ifndef TEXTURES_H
#define TEXTURES_H

#include <gdk-pixbuf/gdk-pixbuf.h>

#define	checkImageWidth 64
#define	checkImageHeight 64


struct pattern_parameters
{
  GLuint texName;
  GLubyte *data;
  GLint texFunc;		/* This is the function to be passed to glTexEnv */
};


extern struct pattern_parameters stock_pattern[6];

void texInit (void);


GLuint create_pattern_from_pixbuf (const GdkPixbuf *pixbuf, GError **gerr);

#endif
