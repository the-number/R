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
#include "cubeview.h"
#include <assert.h>
#include <math.h>
#include "widget-set.h"


#if DEBUG && HAVE_GL_GLUT_H
#include <GL/glut.h>
#endif

static gboolean animate_callback (gpointer data);


static gboolean inverted_rotation;

static void animate_rotation (GbkCubeview *);


/* handle mouse clicks */
gboolean
on_mouse_button (GtkWidget *w, GdkEventButton *event, gpointer data)
{
  GbkCubeview *cv = GBK_CUBEVIEW (w);

  struct cublet_selection *cs = data;
  if (event->type != GDK_BUTTON_PRESS)
    return TRUE;

  /* Don't let a user make a move,  whilst one is already in progress,
     otherwise the cube falls to bits. */
  if (gbk_cubeview_is_animating (cv))
    return TRUE;

  switch (event->button)
    {
    case 3:
      /* this is inherently dangerous.  If an event is missed somehow,
         turn direction could get out of sync */

      inverted_rotation = TRUE;
      //      cv->pending_movement->dir = !cv->pending_movement->dir;

      break;
    case 1:
      /* Make a move */
      if (select_is_selected (cs))
	{
	  /* and tell the blocks.c library that a move has taken place */
	  gbk_cube_rotate_slice (cv->cube, cv->pending_movement);

	  animate_rotation (cv);
	}

      break;
    }

  return TRUE;
}


/* Does exactly what it says on the tin :-) */
static void
animate_rotation (GbkCubeview *dc)
{
  struct move_data *data = dc->pending_movement;

  dc->animation.current_move = data;

  //  set_toolbar_state (PLAY_TOOLBAR_STOP);

  dc->animation.animation_angle = 90.0 * move_turns (dc->animation.current_move);

  g_timeout_add (dc->animation.picture_rate, animate_callback, dc);
}


/* a timeout  calls this func,  to animate the cube */
static gboolean 
animate_callback (gpointer data)
{
  GbkCubeview *dc = data;

  /* how many degrees motion per frame */
  GLfloat increment = 90.0 / (dc->animation.frameQty + 1);

  /*  decrement the current angle */
  dc->animation.animation_angle -= increment;

  /* and redraw it */
  gbk_redisplay (dc);

  if (dc->animation.animation_angle > 0)
    {
      /* call this timeout again */
      g_timeout_add (dc->animation.picture_rate, animate_callback, data);
    }
  else
    {
      /* we have finished the animation sequence now */
      enum Cube_Status status;

      dc->animation.animation_angle = 0.0;
      dc->animation.current_move = NULL;

#if 0
      set_toolbar_state ((move_queue_progress (move_queue).current == 0
			  ? 0 : PLAY_TOOLBAR_BACK)
			 |
			 (move_queue_current (move_queue)
			  ? PLAY_TOOLBAR_PLAY : 0));
#endif

      update_statusbar ();

      if (NOT_SOLVED != (status = gbk_cube_get_status (dc->cube)))
	declare_win (dc->cube);
    }

  return FALSE;
}				/* end animate () */

