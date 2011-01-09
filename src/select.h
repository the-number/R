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
struct display_context;

typedef void select_func (struct cublet_selection *, gpointer data);

/* Initialise the selection library */
struct cublet_selection *
select_create (struct display_context *r, int holdoff,
	       double precision, select_func *do_this, gpointer data);


/* Temporarily enable/disable selection */
void select_disable (struct cublet_selection *);
void select_enable (struct cublet_selection *);

/* Return  a pointer to a structre containing the selected items */
const struct facet_selection *select_get (const struct cublet_selection *cs);

/* Force an update of the selected block */
void select_update (struct cublet_selection *);

/* returns true if a block has been selected */
gboolean select_is_selected (const struct cublet_selection *cs);


struct display_context *cublet_selection_get_display_context (struct cublet_selection *);

#endif
