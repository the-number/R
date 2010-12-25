/*
    User Interface functions for the GNUbik Cube
    Copyright (C) 1998  John Darrington

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
#include <gdk/gdkkeysyms.h>


/* A structure containing information about a movement taking place. */
struct move_data
{
  int slice;
  short dir;   /* 0 = cw, 1 = ccw */
  short axis;  /* [0,2] */
  short turns; /* Normally 1 or 2 */
};

struct animation
{
  /* picture rate in ms. 40ms = 25 frames per sec */
  int picture_rate ;

  /* The current angle through which an animation has turned */
  GLfloat animation_angle ;

  gboolean animation_in_progress ;

  /* the number of frames drawn when a slice of the cube rotates 90 degrees */
  int frameQty ;
};

void abort_animation (void);

gboolean on_mouse_button (GtkWidget *w, GdkEventButton *event, gpointer data);
void selection_func (void);
void arrows (guint, int shifted);
int vector2axis (GLfloat *vector);
void drawCube (GLboolean anc);

/* 
void request_stop (void);
void request_play (void);
void request_forward (void);
void request_back (void);
void request_truncate_move_queue (void);
void request_delayed_rotation (struct move_data *data);
void request_mark_move_queue (void);
void request_queue_rewind (void);
void request_fast_forward (void);

void request_rotation (struct move_data *data);
*/

/* Rotate the cube about the axis (screen relative) in direction dir */
void rotate_cube (int axis, int dir);

int is_animating (void);


#endif
