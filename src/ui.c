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
static const char RCSID[]="$Id: ui.c,v 1.15 2009/01/18 00:54:56 jmd Exp $";

#include <memory.h>
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
#include "gnubik.h"


#include "quarternion.h"


#if DEBUG && HAVE_LIBGLUT
#include <GL/glut.h>
#endif


static short abort_requested=0;
static Slice_Blocks *blocks_in_motion;


void drawInd (GLfloat location,  int axis,  int dir);

TIMEOUT_CALLBACK (animate);


/* picture rate in ms. 40ms = 25 frames per sec */
static const int picture_rate=40;
static GLfloat animation_angle = 0;
static int animation_in_progress = 0;
int frameQty=2;


static int inverted_rotation  = 0;


Move_Queue *move_queue = 0;


/* The move taking place NOW */
static struct _Current_Movement
{
    Move_Data move_data;
    int stop_requested;
}
current_movement;


/* Toolbar callback. The stop will happen when the current animation
   completes. */
void request_stop ()
{
    current_movement.stop_requested = 1;
}





void animate_rotation (Move_Data *data);






/* The move that will take place when the mouse is clicked */
static Move_Data pending_movement={-1, -1, -1};



static int selectionIsValid = 0;


double radians (int deg);


extern Quarternion cube_view;


int
is_animating (void)
{
  return animation_in_progress;
}


/* Rotate cube about axis (screen relative) */
void
rotate_cube (int axis,  int dir)
{
  /* how many degrees to turn the cube with each hit */
  GLdouble step=2.0;
  Quarternion rot;
  vector v;


  if ( dir )
    step = -step;

  switch (axis) {
  case 0:
    v[1]=v[2]=0; v[0] = 1;
    break ;
  case 1:
    v[0]=v[2]=0; v[1] = 1;
    break ;
  case 2:
    v[0]=v[1]=0; v[2] = 1;
    break ;
  }

  quarternion_from_rotation (&rot,  v,  step) ;
  quarternion_pre_mult (&cube_view, &rot);
  postRedisplay ();
}

/* orientate the whole cube with the arrow keys */
void
arrows (t_keysym keysym,  int shifted)
{
  int axis;
  int dir;

  switch (keysym) {
  case GNUBIK_Right:
    if ( shifted) {
      dir = 0;
      axis = 2;
    }
    else {
      dir =1;
      axis = 1;
    }
    break;		
  case GNUBIK_Left:	

    if ( shifted ) {
      dir = 1;
      axis = 2;
    }
    else {
      dir =0;
      axis = 1;
    }
    break;		
  case GNUBIK_Up:	
    axis = 0;
    dir = 0;
    break;		
  case GNUBIK_Down:	
    axis = 0;
    dir = 1;
    break;
  default:
    return;
  }

  rotate_cube (axis,  dir);
}


/* return the turn direction,  based upon the turn axis vector */
static int
turnDir (GLfloat *vector)
{
  /* This is a horrendous kludge.  It works only because we know that
     vector is arithemtically very simple */

    return (vector [0] + vector [1] + vector [2]  >  0);
}




/* Convert a vector to a axis number.  Obviously this assumes the vector
is orthogonal to the frame of reference ( another nasty kludge ). */
int
vector2axis (GLfloat *vector)
{
    return vector [0] != 0 ? 0 : vector [1] != 0 ? 1 : 2;
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
getTurnAxis (struct facet_selection* items,  GLfloat *vector)
{
  Matrix t;
  int i;

  /* Each edge (quadrant) on a block represents a different axis */
  const GLfloat axes[6][4][4]={
    {_Y,  X,  Y, _X},
    {Y,  X, _Y, _X},
    {Z,  X, _Z, _X},
    {_Z,  X,  Z, _X},
    {_Y, _Z,  Y,  Z},
    {Y, _Z, _Y,  Z},
  };

  /* Fetch the selected block's transformation from its original
     orientation */
  if ( get_block_transform (the_cube,  items->block,  t) != 0 ) {
    fprintf (stderr, "Attempt to fetch non-existant block transform\n");
    exit (1);
  }

  /* Select the untransformed vector for the selected edge */
  for (i=0; i < 4 ; i++ )
    vector[i]  =  axes[items->face][items->quadrant][i];
	
  /* transform it,  so that we go the right way */
  transform (t,  vector);

  /*
    printf ("Post transform Turn vector is : (");
    for (i=0; i < 3 ; i++ )
    printf ("%d, ", (int) vector[i]);
    printf ("%d)\n", (int) vector[3]);
  */


}


/* render the cube */
void
drawCube (void)
{
  int i;

#if DEBUG
  {
  GLfloat offset= 1.6 * cube_dimension ;

  /* Show the directions of the axes */
  glColor3f (1, 1, 1);

  /* X axis */
  glPushMatrix ();
  glTranslatef (-offset, -offset, 0);
  glBegin (GL_LINES);
  glVertex3f (0, 0, 0);
  glVertex3f (2*cube_dimension, 0, 0);
  glEnd ();


  glRasterPos3d (offset*1.1, 0, 0);
  glutBitmapCharacter (GLUT_BITMAP_9_BY_15, '0');

  glPopMatrix ();

  /* Y axis */
  glPushMatrix ();
  glTranslatef (-offset, -offset, 0);
  glBegin (GL_LINES);
  glVertex3f (0, 0, 0);
  glVertex3f (0, 2*cube_dimension, 0);
  glEnd ();

  glRasterPos3d (0.1*offset,  offset, 0);
  glutBitmapCharacter (GLUT_BITMAP_9_BY_15, '1');

  glPopMatrix ();

  /* Z axis */
  glPushMatrix ();
  glTranslatef (-offset, -offset, 0);

  glBegin (GL_LINES);
  glVertex3f (0, 0, 0);
  glVertex3f (0, 0, 2*cube_dimension);
  glEnd ();

  glRasterPos3d (0.1*offset, 0,  offset);
  glutBitmapCharacter (GLUT_BITMAP_9_BY_15, '2');

  glPopMatrix ();

  }
#endif


  for ( i = 0 ; i < number_of_blocks ; i++) {

    int j = 0;

    /* Find out if this block is one of those currently being
       turned.  If so,  j will be < turning_block_qty */
    if (blocks_in_motion)
      for (j = 0 ; j < blocks_in_motion->number_blocks; j++ ) {
        if ( blocks_in_motion->blocks [j] == i )
	      break;
    }

    glPushMatrix ();
    if ( blocks_in_motion  &&  j != blocks_in_motion->number_blocks  ) {

      /* Blocks which are in motion,  need to be animated.
	 so we rotate them according to however much the
	 animation angle is */
      GLdouble angle = animation_angle;


      {
	int unity=1;
	if ( ! current_movement.move_data.dir)
	  unity = -1;

	switch (current_movement.move_data.axis) {
	case 0: case 3:
	  glRotatef (angle,  unity, 0, 0);
	  break ;
	case 1: case 4:
	  glRotatef (angle, 0,  unity, 0);
	  break ;
	case 2: case 5:
	  glRotatef (angle, 0, 0,  unity);
	  break ;

	}

      }
    }
    {
      Matrix M;


      /* place the block in its current position and
	 orientation */
      get_block_transform (the_cube,  i,  M);
      glPushMatrix ();
      glMultMatrixf (M);

      /* and draw the block */
      draw_block (0,  i);
      glPopMatrix ();

    }

    glPopMatrix ();

  }
}






/* degrees to radians convertion */
double
radians (int deg)
{
	return ((double)deg * M_PI / 180.0 );
}




/* handle mouse clicks */
void
mouse (int button)
{

  /* Don't let a user make a move,  whilst one is already in progress,
     otherwise the cube falls to bits. */
  if ( animation_in_progress)
    return;


  switch ( button ) {
  case 3:
    /* this is inherently dangerous.  If an event is missed somehow,
       turn direction could get out of sync */


    inverted_rotation=1;
    pending_movement.dir = ! pending_movement.dir ;

    postRedisplay ();
    break;

  case 1:

    /* Make a move */
    if (  itemIsSelected () ) {
      request_rotation (&pending_movement);
    }

    postRedisplay ();

    break;

  }
}



/* Toolbar callback. Use stop_requested to ensure that only one step occurs. */

void
request_back ()
{
    if (! animation_in_progress  &&  move_queue  &&
                                               move_queue_retard (move_queue))
    {
        current_movement.stop_requested = 1;

        Move_Data next_move;

        memcpy (&next_move,
                move_queue_current (move_queue),
                sizeof (Move_Data));

        next_move.dir = ! next_move.dir;

        animate_rotation (&next_move);
    }
}



/* Internal functionality for the following two functions. */

static void
_request_play (const int one_move)
{
    if (! animation_in_progress  &&  move_queue  &&
                                               move_queue_current (move_queue))
    {
        current_movement.stop_requested = one_move;

        Move_Data next_move;

        memcpy (&next_move,
                move_queue_current (move_queue),
                sizeof (next_move));

        move_queue_advance (move_queue);

        animate_rotation (&next_move);
    }
}


/* Toolbar `play' callback. */
void
request_play ()
{
    _request_play (0);
}

/* Toolbar `forward' callback. */
void
request_forward ()
{
    _request_play (1);
}



/* The usual rotation request,  called when user uses mouse to rotate the cube by
   hand. The move is put into the `current' place,  and all the moves in the
   move_queue afterwards are zapped. */

void
request_rotation (Move_Data *data)
{
    if (move_queue == NULL)
        move_queue = new_move_queue ();

    move_queue_push_current (move_queue,  data);

    request_play ();
}




/* The rotation request called from script-fu. In this case the moves are
   appended to the move_queue,  so the script can race ahead and work out the
   moves,  leaving the graphics to catch up later. */

void
request_delayed_rotation (Move_Data *data)
{
    if (move_queue == NULL)
        move_queue = new_move_queue ();

    move_queue_push (move_queue,  data);
}




/* Script-fu calls this function on startup,  and then the above function as each
   move is computed. This way,  the moves computed are appended to the move_queue
   from the current point forwards (and forward data that was on the queue is
   discarded). */

void
request_truncate_move_queue ()
{
    if (move_queue)
        move_queue_truncate (move_queue);
}



/* Toolbar callback. */

void
request_mark_move_queue ()
{
    if (move_queue)
        move_queue_mark_current (move_queue);
}




/* Toolbar callback. All the moves take place on the_cube,  not via the animation
   routines,  so the effect is to unwind the queue instantaneously. */

void
request_queue_rewind ()
{
    int mark_result;
    Slice_Blocks *blocks;
    const Move_Data *move_data;

    if (! move_queue)
        return;

    do
    {
        if ((mark_result = move_queue_retard (move_queue)) == 0)
            break;

        if ((move_data = move_queue_current (move_queue)) != NULL)
        {
            blocks = identify_blocks_2 (the_cube,
                                        move_data->slice,
                                        move_data->axis);

            rotate_slice (the_cube,
                          move_data->dir == 0 ? 1 : 3,   /* This is backwards. */
                          blocks);

            free_slice_blocks (blocks);
        }
    }
    while (mark_result == 1);


    postRedisplay ();
    set_toolbar_state ((move_queue_progress (move_queue).current == 0
                                                      ? 0 : PLAY_TOOLBAR_BACK)
                       |
                       (move_queue_current (move_queue)
                                                      ? PLAY_TOOLBAR_PLAY : 0));

    update_statusbar ();
}




/* Script callback. */

void
request_fast_forward ()
{
    const Move_Data *move_data;
    Slice_Blocks *blocks;

    if (! move_queue)
        return;

    for (move_data = move_queue_current (move_queue);
         move_data != NULL;
         move_data = (move_queue_advance (move_queue),
                      move_queue_current (move_queue)))
    {
        blocks = identify_blocks_2 (the_cube,
                                    move_data->slice,
                                    move_data->axis);

        rotate_slice (the_cube,
                      move_data->dir == 0 ? 3 : 1,
                      blocks);

        free_slice_blocks (blocks);
    }

    postRedisplay ();
    set_toolbar_state (PLAY_TOOLBAR_BACK);
    update_statusbar ();
}



/* Does exactly what it says on the tin :-) */

void
animate_rotation (Move_Data *data)
{
    memcpy (&current_movement.move_data,  data,  sizeof (Move_Data));

    blocks_in_motion = identify_blocks_2 (the_cube,  data->slice,  data->axis);

    if (blocks_in_motion == NULL)
    {
        print_cube_error (the_cube,  "Error rotating block");
        exit (-1);
    }

    animation_in_progress = 1;

    set_toolbar_state (PLAY_TOOLBAR_STOP);

    add_timeout (picture_rate,  animate,  &current_movement);
}


void
abort_animation (void)
{
  abort_requested=1;
}


/* a timeout  calls this func,  to animate the cube */
TIMEOUT_CALLBACK (animate)
{

  struct _Current_Movement *md = (struct _Current_Movement *) data;

  /* how many degrees motion per frame */
  GLfloat increment = 90.0 / (frameQty +1 ) ;


  /*  decrement the current angle */
  animation_angle-= increment ;


  /* and redraw it */
  postRedisplay ();


  if ( fabs (animation_angle) < 90.0 && !abort_requested)
    {
      /* call this timeout again */
      add_timeout (picture_rate,  animate,  data);
    }
  else
    {
      /* we have finished the animation sequence now */
      int turn_qty = 0;
      enum Cube_Status status;

      animation_angle = 0.0;
      /* -90 degrees equals +270 degrees */
      switch ( md->move_data.dir ) {
      case 1:
	turn_qty=1;
	break;
      case 0:
	turn_qty=3;
	break;
      }

      /* and tell the blocks.c library that a move has taken place */

      if (rotate_slice (the_cube,  turn_qty,  blocks_in_motion) != 0) {
	print_cube_error (the_cube,  "Error rotating block");
	exit (-1);
      }

      free_slice_blocks (blocks_in_motion);
      blocks_in_motion = NULL;

      animation_in_progress = 0;

      set_toolbar_state ((move_queue_progress (move_queue).current == 0
			  ? 0 : PLAY_TOOLBAR_BACK)
			 |
			 (move_queue_current (move_queue)
			  ? PLAY_TOOLBAR_PLAY : 0));

      update_statusbar ();
      updateSelection ();

      if (  NOT_SOLVED != (status =  cube_get_status (the_cube) ) )
	declare_win (the_cube);

      if ( abort_requested )
	abort_requested = 0;

      /* If there are more animations in the queue,  go again unless a stop has
	 been requested. */

      if (md->stop_requested)
        md->stop_requested = 0;


      else if (move_queue_current (move_queue))
	{
	  memcpy (&md->move_data,
		  move_queue_current (move_queue),
		  sizeof (Move_Data));
	  move_queue_advance (move_queue);
	  animate_rotation (&md->move_data);
	}


    }

  return FALSE;

} /* end animate () */


/* Render the turn indicators on the scene.
Axis is the axis.  */
void
turn_indicator (int axis,  int dir)
{
  int i;

  if ( ! itemIsSelected () )
    return ;


  /* Don't  render invisible faces */
  glEnable (GL_CULL_FACE);


  for ( i = 0 ; i < 2 ; i++) {
    /* one each side of the cube */
    GLfloat location = (i==0) ? cube_dimension *2.0 : -(cube_dimension * 2.0 );

    /* Inside is dark grey */
    glFrontFace (GL_CCW);
    glColor3f (0.7, 0.7, 0.7);
    drawInd (location,  axis,  dir);


    /* Outside is light grey */
    glFrontFace (GL_CW);
    glColor3f (0.3, 0.3, 0.3);
    drawInd (location,  axis,  dir);

  }

  /* We reset this to avoid trouble later ! */
  glFrontFace (GL_CCW);
}




/* Draw the little indicators to show which way the blocks turn */
void
drawInd (GLfloat location,  int axis,  int dir)
{
  int j;
  int theta;

  const GLfloat radius= cube_dimension / 2.0 ;
  const GLfloat strip_width = 0.1 * cube_dimension;
  const GLint segments = 3;
  const GLfloat arrowlength = 0.1;

  /* define a point data type */
  typedef GLfloat point3[3];

  point3 circ ;

  for ( j = 0 ; j < segments ; j ++) {

    const GLfloat stop = 100 + (j * 360 / segments );
    const GLfloat start = 0 + (j * 360 / segments );

    GLfloat width = strip_width;
    GLfloat terminal;
    const GLfloat crit = dir ? (stop -start ) * (1 - arrowlength) + start :(stop -start ) * arrowlength + start;


    glBegin (GL_TRIANGLE_STRIP);
    for ( theta=start; theta <= stop; theta++) {
			
      terminal  = dir ? stop : start;
      if ( (theta > crit  && dir ) || ( theta < crit && !dir ) ) {
	width = strip_width * fabs ( ( terminal - theta ) / ( terminal - crit ) );
      }
      else
	width = strip_width ;
      circ[axis]  = location + (width * ((theta %2 )*2 -1 ));
      circ[(axis + 1)%3] = radius*sin (radians (theta));
      circ[(axis + 2)%3] = radius*cos (radians (theta));
      glVertex3fv (circ);
    }
    glEnd ();
  }

}



float cursorAngle;


/*
This func is called whenever a new set of polygons have been selected.
Call redisplay */
void
selection_func (void)
{

  GLfloat turn_axis[4];

  struct facet_selection *selection = 0;


  if ( animation_in_progress )
    return ;


  if ( 0 != (selection = selectedItems ()) ) {

    vector v;

    getTurnAxis (selection,  turn_axis);

    pending_movement.dir  = turnDir (turn_axis);
    if ( inverted_rotation )
      pending_movement.dir = ! pending_movement.dir ;

    pending_movement.axis = vector2axis (turn_axis);

    /* !!!!!! We are accessing private cube data. */
    pending_movement.slice
        = the_cube->blocks [selection->block]
                                 .transformation [12 + pending_movement.axis];


    /* Here we take the orientation of the selected quadrant and multiply it
       by the projection matrix.  The result gives us the angle (on the screen)       at which the mouse cursor needs to be drawn. */
    {

      Matrix proj;

      glGetFloatv (GL_PROJECTION_MATRIX,  proj);

      get_quadrant_vector (the_cube,  selection->block,
		      selection->face,  selection->quadrant,  v);


      transform (proj,  v);

      cursorAngle = atan2 (v[0],  v[1]) * 180.0 / M_PI;

    }


    selectionIsValid = 1;

  }
  else {
    pending_movement.dir = -1;
    pending_movement.axis = -1;
    selectionIsValid = 0;
  }

  inverted_rotation = 0;

  postRedisplay ();

}
