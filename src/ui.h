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

#include <GL/gl.h>
#include <gdk/gdkkeysyms.h>

#include "move-queue.h"

extern int frameQty;



typedef guint t_keysym;

#define add_timeout(interval,  proc,  data) g_timeout_add (interval,  proc,  data)

#define TIMEOUT_CALLBACK(X) gboolean X (gpointer data)


void abort_animation (void);

void selection_func (void);
void arrows (t_keysym keysym, int shifted);
void mouse (int button);
int vector2axis (GLfloat * vector);
void drawCube (void);

void request_stop (void);
void request_play (void);
void request_forward (void);
void request_back (void);
void request_truncate_move_queue (void);
void request_delayed_rotation (Move_Data * data);
void request_mark_move_queue (void);
void request_queue_rewind (void);
void request_fast_forward (void);

void request_rotation (Move_Data * data);

/* Rotate the cube about the axis (screen relative) in direction dir */
void rotate_cube (int axis, int dir);

int is_animating (void);


#endif
