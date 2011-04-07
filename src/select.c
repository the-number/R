/*
    A system to permit user selection of a block and rotation axis
    of a magic cube.
    Copyright (C) 1998,  2003, 2011  John Darrington

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
#include <gdk/gdkkeysyms.h> 

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

#include <stdlib.h>

#include <gtk/gtk.h>
#include <assert.h>

#include "drwBlock.h"
#include "cubeview.h"
#include "glarea.h"

static gboolean detect_motion (GtkWidget * w,
			       GdkEventMotion * event, gpointer user_data);


static gboolean UnsetMotion (gpointer data);


struct cublet_selection
{
  guint timer;
  select_func *action;
  gpointer data;
  gint idle_threshold;
  double granularity;
  GtkWidget *w;

  gboolean motion;
  gboolean stop_detected;

  /* A copy of the last selection taken */
  struct facet_selection current_selection;

  gint mouse_x;
  gint mouse_y;
};



static gboolean
key_press (GtkWidget *w, GdkEventKey *e,  gpointer data)
{
  struct cublet_selection *cs = data;

  if ( e->keyval != GDK_Shift_L && e->keyval != GDK_Shift_R )
    return FALSE;
    
  selection_func (cs, w);

  return FALSE;
}


static void pickPolygons (GbkCubeview * cv, struct cublet_selection *cs,
			  struct facet_selection *sel);

/* Initialise the selection mechanism.  Holdoff is the time for which
the mouse must stay still,  for anything to happen. Precision is the
minimum distance it must have moved. DO_THIS is a pointer to a function
to be called when a new block is selected. DATA is a data to be passed to DO_THIS*/
struct cublet_selection *
select_create (GtkWidget * w, int holdoff,
	       double precision, select_func * do_this, gpointer data)
{
  struct cublet_selection *cs = g_malloc (sizeof *cs);

  cs->idle_threshold = holdoff;
  cs->granularity = precision;

  g_signal_connect (w, "motion-notify-event", G_CALLBACK (detect_motion), cs);

  g_signal_connect (w, "key-press-event", G_CALLBACK (key_press), cs);
  g_signal_connect (w, "key-release-event", G_CALLBACK (key_press), cs);

  cs->action = do_this;
  cs->data = data;
  cs->stop_detected = FALSE;
  cs->motion = FALSE;

  cs->timer = g_timeout_add (cs->idle_threshold, UnsetMotion, cs);

  cs->w = w;

  cs->current_selection.block = -1;
  cs->current_selection.face = -1;
  cs->current_selection.quadrant = -1;

  return cs;
}


void
select_destroy (struct cublet_selection *cs)
{
  select_disable (cs);
  g_free (cs);
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

  GbkCubeview *cv = GBK_CUBEVIEW (cs->data);

  if (cs->motion == FALSE)
    {				/* if not moved since last time */

      if (!cs->stop_detected)
	{
	  /* in here,  things happen upon the mouse stopping */
	  cs->stop_detected = TRUE;
	  select_update (cv, cs);
	}
    }

  cs->motion = FALSE;

  return TRUE;
}				/* end UnsetMotion */



static int
get_widget_height (GtkWidget * w)
{
  return w->allocation.height;
}



#include "select.h"
#include <float.h>
#include <stdio.h>

#include <assert.h>
#include <string.h>
#include <GL/glu.h>


#define BUFSIZE 512




static void choose_items (GLint hits, GLuint buffer[],
			  struct facet_selection *);



/* Identify the block at screen co-ordinates x,  y .  This func determines all
   candidate blocks.  That is,  all blocks which orthogonally project to x,  y.  It
   then calls choose_items,  to determine which of them is closest to the screen.
*/
static void
pickPolygons (GbkCubeview * cv, struct cublet_selection *cs,
	      struct facet_selection *sel)
{
  GLint height;

  GLint viewport[4];
  GLuint selectBuf[BUFSIZE];
  GLint hits;

  GtkWidget *w;

  if (!gdk_gl_drawable_make_current (cv->gldrawable, cv->glcontext))
    {
      g_critical ("Cannot set gl drawable current\n");
      return;
    }
  assert (cs->granularity > 0);
  w = cs->w;

  height = get_widget_height (w);

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

  perspectiveSet (&cv->scene);
  gbk_cubeview_model_view_init (cv);
  drawCube (cv->cube, TRUE, cv);
  glMatrixMode (GL_PROJECTION);
  glPopMatrix ();

  ERR_CHECK ("");
  hits = glRenderMode (GL_RENDER);

  choose_items (hits, selectBuf, sel);
}





/* Find out which of all the objects in buffer is the one with the
   lowest Z value.  ie. closer in the depth buffer  */
static void
choose_items (GLint hits, GLuint buffer[], struct facet_selection *selection)
{
  unsigned int i, j;
  GLuint names, *ptr;

  GLint closest[3] = { -1, -1, -1 };

  float zvalue = FLT_MAX;
  float z1;

#define SEL_BLOCK 0
#define SEL_FACE 1
#define SEL_QUAD 2

  g_return_if_fail (hits >= 0);

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

  selection->block = closest[SEL_BLOCK];
  selection->face = closest[SEL_FACE];
  selection->quadrant = closest[SEL_QUAD];

#if DEBUG
  fprintf (stderr, "Selected block %d,  face %d,  quadrant %d\n",
	   selection->block, selection->face, selection->quadrant);
#endif

}

/* an accessor func to get the value of the currently selected items */
const struct facet_selection *
select_get (const struct cublet_selection *cs)
{
  return &cs->current_selection;
}


/* This func,  determines which block the mouse is pointing at,  and if it
   has changed,  calls the function ptr "cs->action" */
void
select_update (GbkCubeview * cv, struct cublet_selection *cs)
{
  pickPolygons (cv, cs, &cs->current_selection);
  if (cs->action)
    cs->action (cs, cs->data);
}

gboolean
select_is_selected (const struct cublet_selection *cs)
{
  return cs->current_selection.quadrant != -1;
}


GtkWidget *
cublet_selection_get_widget (const struct cublet_selection * cs)
{
  return cs->w;
}
