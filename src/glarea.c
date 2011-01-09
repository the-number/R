/*
  GNUbik -- A 3 dimensional magic cube game.
  Copyright (C) 2003, 2004, 2009, 2011  John Darrington

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


#include <time.h>
#include <getopt.h>
#include <math.h>
#include "drwBlock.h"
#include "version.h"
#include "ui.h"
#include "select.h"
#include "glarea.h"
#include "widget-set.h"
#include "cursors.h"
#include "textures.h"
#include "colour-dialog.h"

#include <gdk/gdk.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <gtk/gtk.h>
#include <gtk/gtkwidget.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdktypes.h>

#include <gtk/gtkgl.h>
#include <gdk/gdkglconfig.h>


struct display_context;

typedef void display (struct display_context *);

static display display_anti_alias;
static display display_raw;


struct display_context 
{
  display *display_func;
  GtkWidget *glwidget;
  guint idle_id;

  GdkGLContext *glcontext;
  GdkGLDrawable *gldrawable;
};

static gboolean handleRedisplay (gpointer);

/* Return true iff there is an accumulation buffer which is
   usable */
static GLboolean
have_accumulation_buffer (void)
{
  GLint x;

  glGetIntegerv (GL_ACCUM_BLUE_BITS, &x);
  if (x < 3)
    return GL_FALSE;

  glGetIntegerv (GL_ACCUM_RED_BITS, &x);
  if (x < 3)
    return GL_FALSE;

  glGetIntegerv (GL_ACCUM_GREEN_BITS, &x);
  if (x < 3)
    return GL_FALSE;

  return GL_TRUE;
}

/* Expose callback.  Just redraw the scene */
static gboolean
on_expose (GtkWidget *w, GdkEventExpose *event, gpointer data)
{
  struct display_context *dc = data;
  postRedisplay (dc);
  return FALSE;
}


static void
on_realize (GtkWidget *w, gpointer data)
{
  struct display_context *dc = data;
  dc->glcontext = gtk_widget_get_gl_context (w);
  dc->gldrawable = gtk_widget_get_gl_drawable (w);

  if (!gdk_gl_drawable_gl_begin (dc->gldrawable, dc->glcontext))
    {
      g_critical ("Cannot initialise gl drawable\n");
      return;
    }

  gtk_widget_set_size_request (w, 300, 300);

  set_the_colours (w, "gnubik");

  if (have_accumulation_buffer ())
    dc->display_func = display_anti_alias;
  else
    dc->display_func = display_raw;
}


/* Resize callback.  */
static void
resize_viewport (GtkWidget *w, GtkAllocation *alloc, gpointer data)
{
  GLint min_dim;
  gint height = alloc->height;
  gint width = alloc->width;

  struct display_context *dc = data;

  if (!GTK_WIDGET_REALIZED (w))
    return;

  if (!gdk_gl_drawable_gl_begin (dc->gldrawable, dc->glcontext))
    return;

  min_dim = (width < height) ? width : height;

  /* Ensure that cube is always the same proportions */
  glViewport ((width - min_dim) / 2, (height - min_dim) / 2, min_dim,
	      min_dim);
}

static void
initialize_gl_capability (GtkWidget *glxarea)
{
  gint i;

  const GdkGLConfigMode mode[] = {
    GDK_GL_MODE_RGB | GDK_GL_MODE_DOUBLE | GDK_GL_MODE_ACCUM | GDK_GL_MODE_DEPTH,

    GDK_GL_MODE_RGB | GDK_GL_MODE_DOUBLE  | GDK_GL_MODE_DEPTH,
  };

  GdkScreen *screen = gtk_widget_get_screen (glxarea);

  static GdkGLConfig *glconfig = NULL;

  if ( glconfig == NULL )
    {
      for (i = 0; i < sizeof (mode) / sizeof (mode[0]); ++i)
	{
	  glconfig = gdk_gl_config_new_by_mode_for_screen (screen, mode[i]);

	  if (glconfig != NULL)
	    break;
	  else
	    g_warning ("Cannot get visual for mode 0x%0x", mode[i]);
	}

      if (!glconfig)
	g_error ("No suitable visual found.");
    }

  gtk_widget_set_gl_capability (glxarea, glconfig, 0, TRUE, GDK_GL_RGBA_TYPE);
}

GtkWidget *
display_context_get_widget (struct display_context *dc)
{
  return dc->glwidget;
}


static gboolean            
grab_focus  (GtkWidget *widget,
		GdkEventCrossing *event,
		gpointer data) 
{
  gtk_widget_grab_focus (widget);
  return FALSE;
}

struct display_context *
display_context_create (void)
{
  struct display_context *dc = malloc (sizeof  *dc);

#if WIDGETS_NOT_DISABLED
  const GtkTargetEntry target[2] = {
    {"text/uri-list", 0, RDRAG_FILELIST},
    {"application/x-color", 0, RDRAG_COLOUR},
  };
#endif

  dc->glwidget = gtk_drawing_area_new ();

  initialize_gl_capability (dc->glwidget);

#if WIDGETS_NOT_DISABLED
  gtk_drag_dest_set (glxarea, GTK_DEST_DEFAULT_ALL,
		     target, 2, GDK_ACTION_COPY);

  g_signal_connect (glxarea, "drag-data-received",
		    G_CALLBACK (drag_data_received), (gpointer) - 1);
#endif

  dc->glcontext = NULL;
  dc->gldrawable = NULL;
  dc->idle_id = 0;

  gtk_widget_add_events (GTK_WIDGET (dc->glwidget),
			 GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK
			 | GDK_BUTTON_RELEASE_MASK
			 /* | GDK_BUTTON_MOTION_MASK */
			 | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
			 | GDK_VISIBILITY_NOTIFY_MASK
			 | GDK_POINTER_MOTION_MASK);

  GTK_WIDGET_SET_FLAGS (dc->glwidget, GTK_CAN_FOCUS);

  g_signal_connect (dc->glwidget, "realize", G_CALLBACK (on_realize), dc);
  g_signal_connect (dc->glwidget, "expose-event", G_CALLBACK (on_expose), dc);
  g_signal_connect (dc->glwidget, "size-allocate", G_CALLBACK (resize_viewport), dc);

  /* Grab the keyboard focus wheneve the mouse enters the widget */
  g_signal_connect (dc->glwidget, "enter-notify-event",
		    G_CALLBACK (grab_focus), 0);

  return dc;
}




void
postRedisplay (struct display_context *dc)
{
  if (0 == dc->idle_id)
    dc->idle_id = g_idle_add (handleRedisplay, dc);
}


/* Error string display.  This is always called by a macro
   wrapper,  to set the file and line_no opts parameters */
void
error_check (const char *file, int line_no, const char *string)
{
  GLenum err_state;
  if (GL_NO_ERROR != (err_state = glGetError ()))
    g_print ("%s:%d %s:  %s\n", file, line_no, string,
	     gluErrorString (err_state));
}


/* Rotate the cube about the z axis (relative to the viewer ) */
gboolean
z_rotate (GtkWidget *w, GdkEventScroll *event, gpointer data)
{
  struct display_context *dc = data;

  rotate_cube (2, !event->direction);

  postRedisplay (dc);

  return FALSE;
}

/* Record the state of button 1 */
gboolean
on_button_press_release (GtkWidget *w, GdkEventButton *event, gpointer data)
{
  struct cublet_selection *cs = data;

  if (event->button != 1)
    return FALSE;

  if (event->type == GDK_BUTTON_PRESS)
    {
      select_disable (cs);
    }
  else if (event->type == GDK_BUTTON_RELEASE)
    {
      select_enable (cs);
    }

  return FALSE;
}


gboolean
cube_orientate_mouse (GtkWidget *w, GdkEventMotion *event, gpointer data)
{
  static gdouble last_mouse_x = -1;
  static gdouble last_mouse_y = -1;

  gint xmotion = 0;
  gint ymotion = 0;


  struct display_context *dc = data;

  GdkWindow *window = gtk_widget_get_window (w);

  GdkModifierType mm;
  gdk_window_get_pointer (window, NULL, NULL, &mm);

  if (! (GDK_BUTTON1_MASK & mm))
    return FALSE;

  if (last_mouse_x >= 0)
    xmotion = event->x - last_mouse_x;

  if (last_mouse_y >= 0)
    ymotion = event->y - last_mouse_y;

  last_mouse_x = event->x;
  last_mouse_y = event->y;

  if (ymotion > 0)
    rotate_cube (0, 1);
  if (ymotion < 0)
    rotate_cube (0, 0);
  postRedisplay (dc);

  if (xmotion > 0)
    rotate_cube (1, 1);
  if (xmotion < 0)
    rotate_cube (1, 0);

  postRedisplay (dc);

  return FALSE;
}


/* orientate the whole cube with the arrow keys */
gboolean
cube_orientate_keys (GtkWidget *w, GdkEventKey *event, gpointer data)
{
  struct display_context *dc = data;

  const int shifted = event->state & GDK_SHIFT_MASK;
  
  int axis;
  int dir;

  switch (event->keyval)
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
      return FALSE;
    }

  rotate_cube (axis, dir);
  postRedisplay (dc);

  /* We return TRUE here (disabling other event handlers)
     otherwise other widgets can steal the keyboard focus from our
     glwidget */
  return TRUE;
}

/* Pair of mutually co-operating funcs to handle redisplays,
   avoiding unnecessary overhead.  For example when the window
   is covered by another one
*/
static gboolean
handleRedisplay (gpointer data)
{
  struct display_context *disp_ctx = data;
  disp_ctx->display_func (disp_ctx);
  g_source_remove (disp_ctx->idle_id);
  disp_ctx->idle_id = 0;

  return TRUE;
}




static void
render_scene (GLint jitter)
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  projection_init (jitter);
  modelViewInit ();
  ERR_CHECK ("Error in display");

  drawCube (FALSE);
  ERR_CHECK ("Error in display");
}


/* Reset the bit planes, and render the scene using the accumulation
   buffer for antialiasing*/
static void
display_anti_alias (struct display_context *dc)
{
  int jitter;

  if (!gdk_gl_drawable_make_current (dc->gldrawable, dc->glcontext))
    {
      g_critical ("Cannot set gl drawable current\n");
      return;
    }

  glClear (GL_ACCUM_BUFFER_BIT);
  ERR_CHECK ("Error in display");

  for (jitter = 0; jitter < 8; ++jitter)
    {
      render_scene (jitter);
      glAccum (GL_ACCUM, 1.0 / 8.0);
    }

  glAccum (GL_RETURN, 1.0);

  gdk_gl_drawable_swap_buffers (dc->gldrawable);
}



/* Reset the bit planes, and render the scene,
   without anti-aliasing.
 */
static void
display_raw (struct display_context *dc)
{
  if (!gdk_gl_drawable_make_current (dc->gldrawable, dc->glcontext))
    {
      g_critical ("Cannot set gl drawable current\n");
      return;
    }

  ERR_CHECK ("Error in display");

  render_scene (0);

  gdk_gl_drawable_swap_buffers (dc->gldrawable);
}
