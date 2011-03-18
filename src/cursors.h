/*
    Routines to set mouse cursors
    Copyright (C) 2003  John Darrington

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

#ifndef CURSORS_H
#define CURSORS_H

#include <gdk/gdk.h>

#define n_CURSORS 16

extern GdkCursor *cursor[n_CURSORS * 2];

extern const float cursor_interval;

void get_cursor (int index, const unsigned char **data,
		 const unsigned char **mask, int *height, int *width,
		 int *hot_x, int *hot_y, gboolean rev);

#endif
