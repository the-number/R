/*
    A system to permit user selection of a block and rotation axis
    of a magic cube.
    Copyright (C) 1998,  2003  John Darrington

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

/* This library provides  a means of picking a block using the mouse cursor.

Two mutually co-operative mechanisms are used in this library.  There is a
timer callback,  which occurs at regular intervals.  There is also the mouse
motion callback,  which occurs whenever the mouse cursor is moving.  If two
consecutive timer callbacks occur,  without and intervening mouse motion callback,
then the cursor is assumed to be stationary.

If a stationary mouse is detected,  the program goes on to determine which block
in the cube (if any) the cursor is located upon.
*/

#include "select.h"
#include <float.h>
#include <stdio.h>
#include "ui.h"

#include <gtk/gtk.h>
#include <assert.h>

static int idle_threshold;
static gboolean motion = FALSE;
static gboolean Stop_Detected = FALSE;

static int mouse_x = -1;
static int mouse_y = -1;

static double granularity = -1;
static void (*action) (void);



static gboolean detect_motion (GtkWidget * w,
			       GdkEventMotion * event, gpointer user_data);

static gboolean UnsetMotion (gpointer data);


static gboolean enableDisableSelection (GtkWidget * w,
					GdkEventCrossing * event,
					gpointer data);

static GtkWidget *glxarea;


static guint timer;

/* is the selection mechanism necessary ? */
static gboolean needSelection;





/* Initialise the selection mechanism.  Holdoff is the time for which
the mouse must stay still,  for anything to happen. Precision is the
minimum distance it must have moved. Do_this is a pointer to a function
to be called when a new block is selected. */
void
initSelection (GtkWidget * rendering, int holdoff,
	       double precision, void (*do_this) (void))
{
  needSelection = FALSE;
  idle_threshold = holdoff;
  granularity = precision;

  g_signal_connect (rendering, "motion_notify_event",
		    G_CALLBACK (detect_motion), 0);

  action = do_this;

  timer = g_timeout_add (idle_threshold, UnsetMotion, 0);

  /* Add a handler to for all those occasions when we don't need the
     selection mechanism going */

  g_signal_connect (rendering, "enter-notify-event",
		    G_CALLBACK (enableDisableSelection), 0);

  g_signal_connect (rendering, "leave-notify-event",
		    G_CALLBACK (enableDisableSelection), 0);


  g_signal_connect (rendering, "visibility-notify-event",
		    G_CALLBACK (enableDisableSelection), 0);


  g_signal_connect (rendering, "unmap-event",
		    G_CALLBACK (enableDisableSelection), 0);

  glxarea = rendering;
}


void
disableSelection (void)
{
  g_source_remove (timer);
  timer = 0;
}

void
enableSelection (void)
{
  if (0 == timer)
    timer = g_timeout_add (idle_threshold, UnsetMotion, 0);

  needSelection = TRUE;
}


/* When the window is not mapped,  kill the selection mechanism.  It wastes
processor time */
static gboolean
enableDisableSelection (GtkWidget * w,
			GdkEventCrossing * event, gpointer data)
{
  /* This is a kludge to work around a rather horrible bug;  for some
     reason,  some  platforms emit a EnterNotify and LeaveNotify (in
     that order) when animations occur.  This workaround makes sure
     that the window is not `entered twice' */
  static int entered = 0;


  switch (event->type)
    {

    case GDK_ENTER_NOTIFY:
      entered++;
      needSelection = FALSE;
      if (0 == timer)
	timer = g_timeout_add (idle_threshold, UnsetMotion, 0);

      break;
    case GDK_LEAVE_NOTIFY:
      updateSelection ();
      entered--;
      if (entered > 0)
	break;

      needSelection = TRUE;
      g_source_remove (timer);
      timer = 0;

      break;

    default:
      break;
    }

  return FALSE;
}




/* This callback occurs whenever the mouse is moving */
static gboolean
detect_motion (GtkWidget * w, GdkEventMotion * event, gpointer user_data)
{

  if (event->type != GDK_MOTION_NOTIFY)
    return FALSE;

  mouse_x = event->x;
  mouse_y = event->y;

  motion = TRUE;
  Stop_Detected = FALSE;

  return FALSE;
}





/* This callback occurs at regular intervals.   The period is determined by
idle_threshold.  It checks to see if the mouse has moved,  since the last
call of this function.
Post-condition:  motion is FALSE.
*/
gboolean
UnsetMotion (gpointer data)
{

  if (motion == FALSE)
    {				/* if not moved since last time */

      if (!Stop_Detected)
	{
	  /* in here,  things happen upon the mouse stopping */
	  Stop_Detected = TRUE;
	  updateSelection ();
	}
    }

  motion = FALSE;

  return TRUE;

}				/* end UnsetMotion */



int
get_widget_height (GtkWidget * w)
{
  return w->allocation.height;
}



#include "select.h"
#include <float.h>
#include <stdio.h>
#include "ui.h"
#include "glarea.h"

#include <assert.h>
#include <string.h>
#include <GL/glu.h>


#define BUFSIZE 512




static int noItemSelected = 1;


struct facet_selection *choose_items (GLint hits, GLuint buffer[]);


/* Structure to hold copy of the last selection taken */
static struct facet_selection current_selection = { -1, -1, -1 };


/* Identify the block at screen co-ordinates x,  y .  This func determines all
   candidate blocks.  That is,  all blocks which orthogonally project to x,  y.  It
   then calls choose_items,  to determine which of them is closest to the screen.
*/
struct facet_selection *
pickPolygons (int x, int y)
{
  GLint height;

  GLint viewport[4];
  GLuint selectBuf[BUFSIZE];
  GLint hits;

  assert (granularity > 0);

  height = get_widget_height (glxarea);

  glSelectBuffer (BUFSIZE, selectBuf);

  glRenderMode (GL_SELECT);

  glInitNames ();
  glPushName (0xFFFF);

  glMatrixMode (GL_PROJECTION);
  glPushMatrix ();
  glLoadIdentity ();

  glGetIntegerv (GL_VIEWPORT, viewport);

  gluPickMatrix ((GLdouble) x, (GLdouble) (height - y),
		 granularity, granularity, viewport);


  perspectiveSet ();
  modelViewInit ();
  drawCube ();
  glMatrixMode (GL_PROJECTION);
  glPopMatrix ();

  ERR_CHECK ("");
  hits = glRenderMode (GL_RENDER);
  if (hits == 0)
    {
      ERR_CHECK ("Error Selecting");
      return (NULL);
    }

  return choose_items (hits, selectBuf);
}





/* Find out which of all the objects in buffer is the one with the
   lowest Z value.  ie. closer in the depth buffer  */
struct facet_selection *
choose_items (GLint hits, GLuint buffer[])
{
  static struct facet_selection selection;
  unsigned int i, j;
  GLuint names, *ptr;

  GLint closest[3] = { -1, -1, -1 };

  float zvalue = FLT_MAX;
  float z1;

#define SEL_BLOCK 0
#define SEL_FACE 1
#define SEL_QUAD 2

  ptr = (GLuint *) buffer;

  for (i = 0; i < hits; i++)
    {
      names = *ptr++;

      z1 = (float) *ptr++ / 0x7fffffff;

      ptr++;			/* we're not interested in the minimum zvalue */

      if (z1 < zvalue)
	{
	  zvalue = z1;
	  for (j = 0; j < names; j++)
	    {
	      closest[j] = *ptr++;
	    }

	}
      else
	{
	  ptr += names;
	}
    }

  if (closest[SEL_QUAD] == -1)
    return NULL;

  selection.block = closest[SEL_BLOCK];
  selection.face = closest[SEL_FACE];
  selection.quadrant = closest[SEL_QUAD];

#if DEBUG
  fprintf (stderr, "Selected block %d,  face %d,  quadrant %d\n",
	   selection.block, selection.face, selection.quadrant);
#endif

  return &selection;
}

/* an accessor func to get the value of the currently selected items */
struct facet_selection *
selectedItems (void)
{
  if (noItemSelected)
    return NULL;

  return &current_selection;
}



/* This func,  determines which block the mouse is pointing at,  and if it
   has changed,  calls the function ptr "action" */
void
updateSelection (void)
{
  struct facet_selection *selected_polygons = pickPolygons (mouse_x, mouse_y);

  if (selected_polygons == NULL)
    {
      noItemSelected = 1;
    }
  else
    {
      noItemSelected = 0;

      current_selection = *selected_polygons;
    }

  if (action)
    action ();
}

int
itemIsSelected (void)
{
  return !noItemSelected;
}
