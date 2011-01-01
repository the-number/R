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

#include <stdlib.h>

#include <gtk/gtk.h>
#include <assert.h>

static gboolean detect_motion (GtkWidget * w,
			       GdkEventMotion * event, gpointer user_data);


static gboolean UnsetMotion (gpointer data);

struct cublet_selection 
{
  guint timer;
  void (*action) (void);
  gint idle_threshold;
  double granularity;
  GtkWidget *glwidget;

  gboolean motion;
  gboolean stop_detected;

  /* A copy of the last selection taken */
  struct facet_selection *current_selection;

  gint mouse_x;
  gint mouse_y;
};

/* Identify the block at screen co-ordinates x,  y */
static struct facet_selection *pickPolygons (struct cublet_selection *);


/* Initialise the selection mechanism.  Holdoff is the time for which
the mouse must stay still,  for anything to happen. Precision is the
minimum distance it must have moved. Do_this is a pointer to a function
to be called when a new block is selected. */
struct cublet_selection *
select_create (GtkWidget *rendering, int holdoff,
	       double precision, void (*do_this) (void))
{
  struct cublet_selection *cs = malloc (sizeof *cs);

  cs->idle_threshold = holdoff;
  cs->granularity = precision;

  g_signal_connect (rendering, "motion-notify-event",
		    G_CALLBACK (detect_motion), cs);

  cs->action = do_this;
  cs->stop_detected = FALSE;

  cs->timer = g_timeout_add (cs->idle_threshold, UnsetMotion, cs);

  cs->glwidget = rendering;

  return cs;
}


void
select_disable (struct cublet_selection *cs)
{
  if (cs->timer)
    g_source_remove (cs->timer);
  cs->timer = 0;
}

void
select_enable (struct cublet_selection *cs)
{
  if (0 == cs->timer)
    cs->timer = g_timeout_add (cs->idle_threshold, UnsetMotion, cs);
}

/* This callback occurs whenever the mouse is moving */
static gboolean
detect_motion (GtkWidget * w, GdkEventMotion * event, gpointer data)
{
  struct cublet_selection *cs = data;

  if (event->type != GDK_MOTION_NOTIFY)
    return FALSE;

  cs->mouse_x = event->x;
  cs->mouse_y = event->y;

  cs->motion = TRUE;
  cs->stop_detected = FALSE;

  return FALSE;
}





/* This callback occurs at regular intervals.   The period is determined by
cs->idle_threshold.  It checks to see if the mouse has moved,  since the last
call of this function.
Post-condition:  motion is FALSE.
*/
static gboolean
UnsetMotion (gpointer data)
{
  struct cublet_selection *cs = data;

  if (cs->motion == FALSE)
    {				/* if not moved since last time */

      if (!cs->stop_detected)
	{
	  /* in here,  things happen upon the mouse stopping */
	  cs->stop_detected = TRUE;
	  select_update (cs);
	}
    }

  cs->motion = FALSE;

  return TRUE;
}  /* end UnsetMotion */



static int
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




struct facet_selection *choose_items (GLint hits, GLuint buffer[]);



/* Identify the block at screen co-ordinates x,  y .  This func determines all
   candidate blocks.  That is,  all blocks which orthogonally project to x,  y.  It
   then calls choose_items,  to determine which of them is closest to the screen.
*/
static struct facet_selection *
pickPolygons (struct cublet_selection *cs)
{
  GLint height;

  GLint viewport[4];
  GLuint selectBuf[BUFSIZE];
  GLint hits;

  assert (cs->granularity > 0);

  height = get_widget_height (cs->glwidget);

  glSelectBuffer (BUFSIZE, selectBuf);

  glRenderMode (GL_SELECT);

  glInitNames ();
  glPushName (0xFFFF);

  glMatrixMode (GL_PROJECTION);
  glPushMatrix ();
  glLoadIdentity ();

  glGetIntegerv (GL_VIEWPORT, viewport);

  gluPickMatrix ((GLdouble) cs->mouse_x, (GLdouble) (height - cs->mouse_y),
		 cs->granularity, cs->granularity, viewport);


  perspectiveSet ();
  modelViewInit ();
  drawCube (TRUE);
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
const struct facet_selection *
select_get (const struct cublet_selection *cs)
{
  return cs->current_selection;
}



/* This func,  determines which block the mouse is pointing at,  and if it
   has changed,  calls the function ptr "cs->action" */
void
select_update (struct cublet_selection *cs)
{
  cs->current_selection = pickPolygons (cs);

  if (cs->action)
    cs->action ();
}

gboolean
select_is_selected (struct cublet_selection *cs)
{
  return (NULL != cs->current_selection);
}
