/*
    A system to permit user selection of a block and rotation axis
    of a magic cube.
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
#ifndef SELECT_H
#define SELECT_H

#include <GL/gl.h>
#include <gtk/gtk.h>

struct facet_selection
{
  int block;
  int face;
  int quadrant;
};

struct cublet_selection ;


/* Initialise the selection library */
struct cublet_selection * initSelection (GtkWidget *glxarea, int holdoff,
		    GLdouble precision, void (*do_this) (void));

/* Temporarily enable/disable selection */
void disableSelection (struct cublet_selection *);
void enableSelection (struct cublet_selection *);

/* Return  a pointer to a structre containing the selected items */
struct facet_selection *selectedItems (const struct cublet_selection *cs);

/* Force an update of the selected block */
void updateSelection (struct cublet_selection *);


int get_widget_height (GtkWidget *w);


/* Identify the block at screen co-ordinates x,  y */
struct facet_selection *pickPolygons (struct cublet_selection *, int x, int y);


/* returns true if a block has been selected */
gboolean itemIsSelected (struct cublet_selection *cs);


#endif
