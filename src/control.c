/*
    Control functions for the Cube
    Copyright (C) 1998,  2003  John Darrington
                  2004  John Darrington,  Dale Mellor
		  2011  John Darrington

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


#include <config.h>

#include "control.h"
#include "select.h"

#include "cursors.h"
#include "cubeview.h"
#include "txfm.h"

#include <math.h>

/* return the turn direction,  based upon the turn axis vector */
static int
turnDir (GLfloat * vector)
{
  /* This is a horrendous kludge.  It works only because we know that
     vector is arithemtically very simple */

  return (vector[0] + vector[1] + vector[2] > 0);
}


/* Convert a vector to a axis number.  Obviously this assumes the vector
is orthogonal to the frame of reference ( another nasty kludge ). */
static int
vector2axis (GLfloat * vector)
{
  return vector[0] != 0 ? 0 : vector[1] != 0 ? 1 : 2;
}

#define X {1, 0, 0, 0}
#define _X {-1, 0, 0, 0}
#define Y {0, 1, 0, 0}
#define _Y {0, -1, 0, 0}
#define Z {0, 0, 1, 0}
#define _Z {0, 0, -1, 0}


/* Determine the axis about which to rotate the slice,  from the objects
selected by the cursor position */
static void
getTurnAxis (GbkCube * cube, const struct facet_selection *items,
	     GLfloat * vector)
{
  Matrix t;

  /* Each edge (quadrant) on a block represents a different axis */
  const GLfloat axes[6][4][4] = {
    {_Y, X, Y, _X},
    {Y, X, _Y, _X},
    {Z, X, _Z, _X},
    {_Z, X, Z, _X},
    {_Y, _Z, Y, Z},
    {Y, _Z, _Y, Z},
  };

  /* Fetch the selected block's transformation from its original
     orientation */
  if (gbk_cube_get_block_transform (cube, items->block, t) != 0)
    {
      g_error ("Attempt to fetch non-existant block transform");
    }

  /* Select the untransformed vector for the selected edge
     and transform it,  so that we go the right way */
  vector_transform (vector, axes[items->face][items->quadrant], t);
}

static void
set_mouse_cursor (GtkWidget *w, const struct cublet_selection *cs)
{
  GbkCubeview *cv = GBK_CUBEVIEW (w);
  GdkCursor *selected_cursor = cv->background_cursor;

  if (select_is_selected (cs))
    {
      GdkDisplay *display = gtk_widget_get_display (w);
      GdkModifierType keymask;
      gdk_display_get_pointer (display,
			       NULL, NULL, NULL, &keymask);

      int index;
      float angle = (int) cv->cursor_angle % 360;

      if (angle < 0)
	angle += 360.0;

      index = round (angle / cursor_interval);
      index = index % n_CURSORS;

      if ( keymask & GDK_SHIFT_MASK)
	index += n_CURSORS;

      if (NULL == cursor[index] )
	{
	  const unsigned char *mask_bits;
	  const unsigned char *data_bits;
	  int hot_x, hot_y;
	  int width, height;

	  GdkPixmap *source, *mask;
	  GdkColor fg = { 0, 65535, 65535, 65535 };	/* White. */
	  GdkColor bg = { 0, 0, 0, 0 };	/* Black. */


	  get_cursor (index % n_CURSORS, &data_bits, &mask_bits, &height, &width,
		      &hot_x, &hot_y, keymask & GDK_SHIFT_MASK);

	  source = gdk_bitmap_create_from_data (NULL,
						(const gchar *) data_bits,
						width, height);

	  mask = gdk_bitmap_create_from_data (NULL,
					      (const gchar *) mask_bits,
					      width, height);


	  selected_cursor = gdk_cursor_new_from_pixmap (source, mask, &fg, &bg, hot_x, hot_y);
	  g_object_unref (source);
	  g_object_unref (mask);

	  cursor[index] = selected_cursor;
	}
      else
	selected_cursor = cursor[index];
    }

  gdk_window_set_cursor (w->window, selected_cursor);
}


/* 
   This func is called whenever a new set of polygons have been selected.
*/
void
selection_func (struct cublet_selection *cs, gpointer data)
{
  GbkCubeview *cv = GBK_CUBEVIEW (data);

  if (gbk_cubeview_is_animating (cv))
    return;

  if (select_is_selected (cs))
    {
      const struct facet_selection *selection = select_get (cs);
      GLfloat turn_axis[4];
      vector v;
      short axis;
      short dir;
      int slice;

      getTurnAxis (cv->cube, selection, turn_axis);
      axis = vector2axis (turn_axis);

      dir = turnDir (turn_axis);

      /* !!!!!! We are accessing private cube data. */
      slice = cv->cube->blocks[selection->block].transformation[12 + axis];

      /* Delete the old move and create a new one */
      move_unref (cv->pending_movement);

      /* Reverse the direction when the shift key is down */
      {
	GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET (cv));
	GdkModifierType keymask;
	gdk_display_get_pointer (display,
				 NULL, NULL, NULL, &keymask);

	if ( keymask & GDK_SHIFT_MASK)
	  dir = !dir;
      }

      cv->pending_movement = move_create (slice, axis, dir);

      /* Here we take the orientation of the selected quadrant and multiply it
         by the projection matrix.  The result gives us the angle (on the screen)
         at which the mouse cursor needs to be drawn. */
      {
	Matrix proj;

	glGetFloatv (GL_PROJECTION_MATRIX, proj);

	gbk_cube_get_quadrant_vector (cv->cube, selection->block,
				      selection->face, selection->quadrant,
				      v);

	vector_transform_in_place (v, proj);

	cv->cursor_angle = atan2 (v[0], v[1]) * 180.0 / M_PI;
      }
    }

  GtkWidget *w = cublet_selection_get_widget (cs);
  set_mouse_cursor (w, cs);
  gbk_cubeview_redisplay (GBK_CUBEVIEW (w));
}
