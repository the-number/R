/*
    User Interface functions for the GNUbik Cube
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

#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>
#include <GL/gl.h>

#include "cube.h"

struct animation
{
  /* picture rate in ms. 40ms = 25 frames per sec */
  int picture_rate ;

  /* The current angle through which an animation has turned */
  GLfloat animation_angle ;

  struct move_data *current_move;

  /* the number of frames drawn when a slice of the cube rotates 90 degrees */
  int frameQty ;
};

gboolean on_mouse_button (GtkWidget *w, GdkEventButton *event, gpointer data);

gboolean is_animating (void);

void set_frame_qty (int frames);


#endif
