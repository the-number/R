/*
    Menu definition code.
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

#ifndef COLOUR_SEL_H
#define COLOUR_SEL_H

#include <GL/gl.h>
#include <gtk/gtk.h>

void colour_select_menu (GtkWidget * w,  gpointer  data );

enum distrib_type { TILED,  MOSAIC };
enum rendering_type { UNDEFINED = 0,  COLORED,  IMAGED };

struct cube_rendering {
  enum rendering_type type;
  GLuint texName;
  enum distrib_type distr;
};


extern struct  cube_rendering *rendering[];

#define RDRAG_COLOUR 0
#define RDRAG_FILELIST 1



void
drag_data_received (GtkWidget *widget,
		   GdkDragContext *dc,
		   gint x,  gint y,
		   GtkSelectionData *selection_data,
		   guint info,
		   guint t,
		   gpointer user_data);

#endif
