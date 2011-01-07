/*
    User Interface functions for the Cube
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

#include "ui.h"
#include "select.h"
#include "drwBlock.h"
#include "glarea.h"
#include <assert.h>
#include <math.h>
#include "widget-set.h"


#if DEBUG && HAVE_GL_GLUT_H
#include <GL/glut.h>
#endif

static gboolean animate (gpointer data);


struct animation animation = {40, 0, false, 2};


static gboolean inverted_rotation;

static void animate_rotation (struct move_data * data);


gboolean
is_animating (void)
{
  return (animation.current_move != NULL);
}


/* handle mouse clicks */
gboolean
on_mouse_button (GtkWidget *w, GdkEventButton *event, gpointer data)
{
  struct move_data *pending_movement = data;
  if (event->type != GDK_BUTTON_PRESS)
    return TRUE;

  /* Don't let a user make a move,  whilst one is already in progress,
     otherwise the cube falls to bits. */
  if (is_animating ())
    return TRUE;

  switch (event->button)
    {
    case 3:
      /* this is inherently dangerous.  If an event is missed somehow,
         turn direction could get out of sync */

      inverted_rotation = TRUE;
      pending_movement->dir = !pending_movement->dir;

      //postRedisplay (the_display_context);
      break;
    case 1:
      /* Make a move */
      if (select_is_selected (the_cublet_selection))
	{

	  /* Insist upon 180 degree turns if the section is non-square */
	  if ( !cube_square_axis (the_cube, pending_movement->axis))
	    pending_movement->turns = 2;

	  animate_rotation (pending_movement);
	}

      break;
    }

  return TRUE;
}


/* Does exactly what it says on the tin :-) */
static void
animate_rotation (struct move_data *data)
{
  data->blocks_in_motion = identify_blocks_2 (the_cube, data->slice, data->axis);

  assert (data->blocks_in_motion);

  animation.current_move = data;

  //  set_toolbar_state (PLAY_TOOLBAR_STOP);

  g_timeout_add (animation.picture_rate, animate, data);
}


/* a timeout  calls this func,  to animate the cube */
static gboolean 
animate (gpointer data)
{
  struct move_data *md = data;

  /* how many degrees motion per frame */
  GLfloat increment = 90.0 / (animation.frameQty + 1);

  /*  decrement the current angle */
  animation.animation_angle -= increment;

  /* and redraw it */
  // postRedisplay (the_display_context);

  if (fabs (animation.animation_angle) < 90.0 * md->turns )
    {
      /* call this timeout again */
      g_timeout_add (animation.picture_rate, animate, data);
    }
  else
    {
      /* we have finished the animation sequence now */
      enum Cube_Status status;

      animation.animation_angle = 0.0;

      /* and tell the blocks.c library that a move has taken place */
      rotate_slice (the_cube, md);

      free_slice_blocks (md->blocks_in_motion);
      md->blocks_in_motion = NULL;

      animation.current_move = NULL;

#if 0
      set_toolbar_state ((move_queue_progress (move_queue).current == 0
			  ? 0 : PLAY_TOOLBAR_BACK)
			 |
			 (move_queue_current (move_queue)
			  ? PLAY_TOOLBAR_PLAY : 0));
#endif

      update_statusbar ();

      if (NOT_SOLVED != (status = cube_get_status (the_cube)))
	declare_win (the_cube);
    }

  return FALSE;
}				/* end animate () */

