/*
    User Interface functions for the Cube
    Copyright (C) 1998,  2003  John Darrington
                  2004  John Darrington,  Dale Mellor

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
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "txfm.h"
#include "cube.h"
#include "drwBlock.h"
#include "ui.h"
#include "glarea.h"
#include "select.h"
#include "widget-set.h"

#include "quarternion.h"


#if DEBUG && HAVE_GL_GLUT_H
#include <GL/glut.h>
#endif

static short abort_requested = 0;
static Slice_Blocks *blocks_in_motion;

static gboolean animate (gpointer data);


struct animation animation = {40, 0, false, 2};


static int inverted_rotation = 0;

static void animate_rotation (struct move_data * data);


/* The move that will take place when the mouse is clicked */
static struct move_data pending_movement = { -1, -1, -1, 0 };


static int selectionIsValid = 0;


extern Quarternion cube_view;


int
is_animating (void)
{
  return animation.animation_in_progress;
}


/* Rotate cube about axis (screen relative) */
void
rotate_cube (int axis, int dir)
{
  /* how many degrees to turn the cube with each hit */
  GLdouble step = 2.0;
  Quarternion rot;
  vector v;

  if (dir)
    step = -step;

  switch (axis)
    {
    case 0:
      v[1] = v[2] = 0;
      v[0] = 1;
      break;
    case 1:
      v[0] = v[2] = 0;
      v[1] = 1;
      break;
    case 2:
      v[0] = v[1] = 0;
      v[2] = 1;
      break;
    }

  quarternion_from_rotation (&rot, v, step);
  quarternion_pre_mult (&cube_view, &rot);
  postRedisplay ();
}

/* orientate the whole cube with the arrow keys */
void
arrows (t_keysym keysym, int shifted)
{
  int axis;
  int dir;

  switch (keysym)
    {
    case GDK_Right:
      if (shifted)
	{
	  dir = 0;
	  axis = 2;
	}
      else
	{
	  dir = 1;
	  axis = 1;
	}
      break;
    case GDK_Left:

      if (shifted)
	{
	  dir = 1;
	  axis = 2;
	}
      else
	{
	  dir = 0;
	  axis = 1;
	}
      break;
    case GDK_Up:
      axis = 0;
      dir = 0;
      break;
    case GDK_Down:
      axis = 0;
      dir = 1;
      break;
    default:
      return;
    }

  rotate_cube (axis, dir);
}


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
int
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
getTurnAxis (const struct facet_selection *items, GLfloat * vector)
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
  if (get_block_transform (the_cube, items->block, t) != 0)
    {
      fprintf (stderr, "Attempt to fetch non-existant block transform\n");
      exit (1);
    }

  /* Select the untransformed vector for the selected edge
     and transform it,  so that we go the right way */
  transform (t, axes[items->face][items->quadrant], vector);
}


/* render the cube */
void
drawCube (GLboolean ancilliary)
{
  g_print ("%s %d\n", __FUNCTION__, ancilliary);
  int i;

#if DEBUG
  {
    GLfloat offset = 1.6 * cube_get_size (the_cube, 0);

    /* Show the directions of the axes */
    glColor3f (1, 1, 1);

    /* X axis */
    glPushMatrix ();
    glTranslatef (-offset, -offset, 0);
    glBegin (GL_LINES);
    glVertex3f (0, 0, 0);
    glVertex3f (2 * cube_get_size (the_cube, 0), 0, 0);
    glEnd ();


    glRasterPos3d (offset * 1.1, 0, 0);
    glutBitmapCharacter (GLUT_BITMAP_9_BY_15, '0');

    glPopMatrix ();

    /* Y axis */
    glPushMatrix ();
    glTranslatef (-offset, -offset, 0);
    glBegin (GL_LINES);
    glVertex3f (0, 0, 0);
    glVertex3f (0, 2 * cube_get_size (the_cube, 1), 0);
    glEnd ();

    glRasterPos3d (0.1 * offset, offset, 0);
    glutBitmapCharacter (GLUT_BITMAP_9_BY_15, '1');

    glPopMatrix ();

    /* Z axis */
    glPushMatrix ();
    glTranslatef (-offset, -offset, 0);

    glBegin (GL_LINES);
    glVertex3f (0, 0, 0);
    glVertex3f (0, 0, 2 * cube_get_size (the_cube, 2));
    glEnd ();

    glRasterPos3d (0.1 * offset, 0, offset);
    glutBitmapCharacter (GLUT_BITMAP_9_BY_15, '2');

    glPopMatrix ();

  }
#endif

  for (i = 0; i < cube_get_number_of_blocks (the_cube); i++)
    {
      int j = 0;

      /* Find out if this block is one of those currently being
         turned.  If so,  j will be < turning_block_qty */
      if (blocks_in_motion)
	for (j = 0; j < blocks_in_motion->number_blocks; j++)
	  {
	    if (blocks_in_motion->blocks[j] == i)
	      break;
	  }

      glPushMatrix ();
      if (blocks_in_motion && j != blocks_in_motion->number_blocks)
	{
	  /* Blocks which are in motion,  need to be animated.
	     so we rotate them according to however much the
	     animation angle is */
	  GLdouble angle = animation.animation_angle;

	  int unity = 1;
	  if (!pending_movement.dir)
	    unity = -1;

	  switch (pending_movement.axis)
	    {
	    case 0:
	    case 3:
	      glRotatef (angle, unity, 0, 0);
	      break;
	    case 1:
	    case 4:
	      glRotatef (angle, 0, unity, 0);
	      break;
	    case 2:
	    case 5:
	      glRotatef (angle, 0, 0, unity);
	      break;
	    }
	}
      {
	Matrix M;

	/* place the block in its current position and
	   orientation */
	get_block_transform (the_cube, i, M);
	glPushMatrix ();
	glMultMatrixf (M);

	/* and draw the block */
	draw_block (i, ancilliary);
	glPopMatrix ();

      }

      glPopMatrix ();
    }
}




/* handle mouse clicks */
void
mouse (int button)
{
  /* Don't let a user make a move,  whilst one is already in progress,
     otherwise the cube falls to bits. */
  if (animation.animation_in_progress)
    return;

  switch (button)
    {
    case 3:
      /* this is inherently dangerous.  If an event is missed somehow,
         turn direction could get out of sync */

      inverted_rotation = 1;
      pending_movement.dir = !pending_movement.dir;

      postRedisplay ();
      break;
    case 1:
      /* Make a move */
      if (select_is_selected (the_cublet_selection))
	{
	  g_print ("%s:%d Axis: %d; Direction: %d\n", __FILE__, __LINE__,
		   pending_movement.axis,
		   pending_movement.dir);

	  /* Insist upon 180 degree turns if the section is non-square */
	  if ( !cube_square_axis (the_cube, pending_movement.axis))
	    pending_movement.turns = 2;

	  animate_rotation (&pending_movement);
	}

      postRedisplay ();

      break;
    }
}


/* Does exactly what it says on the tin :-) */
static void
animate_rotation (struct move_data *data)
{
  blocks_in_motion = identify_blocks_2 (the_cube, data->slice, data->axis);

  g_print ("%s:%d Axis %d\n", __FILE__, __LINE__, data->axis);

  assert (blocks_in_motion);

  animation.animation_in_progress = 1;

  set_toolbar_state (PLAY_TOOLBAR_STOP);

  g_timeout_add (animation.picture_rate, animate, data);
}


void
abort_animation (void)
{
  abort_requested = 1;
}


/* a timeout  calls this func,  to animate the cube */
static gboolean 
animate (gpointer data)
{
  struct move_data *md = data;

  g_print ("%s:%d Axis %d\n", __FILE__, __LINE__, md->axis);

  /* how many degrees motion per frame */
  GLfloat increment = 90.0 / (animation.frameQty + 1);

  /*  decrement the current angle */
  animation.animation_angle -= increment;

  /* and redraw it */
  postRedisplay ();

  if (fabs (animation.animation_angle) < 90.0 * md->turns && !abort_requested)
    {
      /* call this timeout again */
      g_timeout_add (animation.picture_rate, animate, data);
    }
  else
    {
      /* we have finished the animation sequence now */
      enum Cube_Status status;

      animation.animation_angle = 0.0;

      g_print ("Finished Dir: %d\n", md->dir);

      /* and tell the blocks.c library that a move has taken place */
      rotate_slice (the_cube, md->turns, md->dir, blocks_in_motion);

      free_slice_blocks (blocks_in_motion);
      blocks_in_motion = NULL;

      animation.animation_in_progress = 0;

#if 0
      set_toolbar_state ((move_queue_progress (move_queue).current == 0
			  ? 0 : PLAY_TOOLBAR_BACK)
			 |
			 (move_queue_current (move_queue)
			  ? PLAY_TOOLBAR_PLAY : 0));
#endif

      update_statusbar ();
      select_update (the_cublet_selection);

      if (NOT_SOLVED != (status = cube_get_status (the_cube)))
	declare_win (the_cube);

      if (abort_requested)
	abort_requested = 0;

      /* If there are more animations in the queue,  go again unless a stop has
         been requested. */

#if 0
      if (md->stop_requested)
	{
	  md->stop_requested = 0;
	}
      else 
	if (move_queue_current (move_queue))
	{
	  memmove (&md->move_data,
		  move_queue_current (move_queue), sizeof (md->move_data));
	  move_queue_advance (move_queue);
	  animate_rotation (&md->move_data);
	}
#endif
    }

  return FALSE;
}				/* end animate () */


float cursorAngle;

/* 
   This func is called whenever a new set of polygons have been selected.
 */
void
selection_func (void)
{
  const struct facet_selection *selection = 0;

  if (animation.animation_in_progress)
    return;

  if (0 != (selection = select_get (the_cublet_selection)))
    {
      GLfloat turn_axis[4];
      vector v;

      getTurnAxis (selection, turn_axis);
      pending_movement.axis = vector2axis (turn_axis);


      pending_movement.dir = turnDir (turn_axis);
      if (inverted_rotation)
	pending_movement.dir = !pending_movement.dir;

      g_print ("%s:%d Axis %d; Dir %d\n", __FILE__, __LINE__,
	       pending_movement.axis,
	       pending_movement.dir);

      /* !!!!!! We are accessing private cube data. */
      pending_movement.slice
	=
	the_cube->blocks[selection->block].transformation[12 +
							  pending_movement.axis];
      
      /* Default to a turn of 90 degrees */
      pending_movement.turns = 1;


      /* Here we take the orientation of the selected quadrant and multiply it
         by the projection matrix.  The result gives us the angle (on the screen)
	 at which the mouse cursor needs to be drawn. */
      {

	Matrix proj;

	glGetFloatv (GL_PROJECTION_MATRIX, proj);

	get_quadrant_vector (the_cube, selection->block,
			     selection->face, selection->quadrant, v);

	transform_in_place (proj, v);

	cursorAngle = atan2 (v[0], v[1]) * 180.0 / M_PI;
      }

      selectionIsValid = 1;
    }
  else
    {
      pending_movement.dir = -1;
      pending_movement.axis = -1;
      selectionIsValid = 0;
    }

  inverted_rotation = 0;

  postRedisplay ();
}
