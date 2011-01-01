/*
  Colour Menu code.
  Copyright (C) 2003, 2011  John Darrington

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

#ifndef COLOUR_DIALOG_H
#define COLOUR_DIALOG_H

#include <GL/gl.h>
#include <gtk/gtk.h>

void colour_select_menu (GtkWidget *w, GtkWindow *window);

enum distrib_type
{
  TILED, MOSAIC
};

enum rendering_type
{
  UNDEFINED = 0,
  COLORED,
  IMAGED
};

struct cube_rendering
{
  enum rendering_type type;
  GLuint texName;
  enum distrib_type distr;
};


#endif
