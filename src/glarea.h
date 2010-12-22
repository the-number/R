/*
    GNUbik -- A 3 dimensional magic cube game.
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

#ifndef GLAREA_H
#define GLAREA_H

#include <gtk/gtk.h>


#define ERR_CHECK(string)  error_check (__FILE__,__LINE__,string)

void error_check (const char *file, int line_no, const char *string);


void re_initialize_glarea (void);

GtkWidget *create_gl_area (void);

void perspectiveSet (void);
void modelViewInit (void);
void postRedisplay (void);
void lighting_init (void);
void projection_init (int jitter);

/* Callbacks for signals on the glarea widget */
void on_realize (GtkWidget *w, gpointer data);
void resize_viewport (GtkWidget *w, GtkAllocation *alloc, gpointer data);
gboolean cube_orientate_keys (GtkWidget *w, GdkEventKey *event, gpointer data);
void graphics_area_init (GtkWidget *w, gpointer data);
gboolean z_rotate (GtkWidget * w, GdkEventScroll * event, gpointer data);
void on_expose (GtkWidget * w, GdkEventExpose * event);
gboolean cube_orientate_mouse (GtkWidget *w, GdkEventMotion * event, gpointer data);
gboolean on_button_press_release (GtkWidget *w, GdkEventButton * event, gpointer data);
gboolean cube_controls (GtkWidget *w, GdkEventButton * event, gpointer data);

#endif
