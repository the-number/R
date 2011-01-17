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

#include "cubeview.h"
#include "select.h"
#include "control.h"
#include "ui.h"
#include "drwBlock.h"
#include <GL/glu.h>
#include "glarea.h"
#include <gdk/gdkkeysyms.h>


static void on_realize (GtkWidget *w, gpointer data);
static void resize_viewport (GtkWidget *w, GtkAllocation *alloc, gpointer data);
static gboolean on_expose (GtkWidget *w, GdkEventExpose *event, gpointer data);

static void set_the_colours (GtkWidget *w, const char *progname);
static GLboolean have_accumulation_buffer (void);


/* The move that will take place when the mouse is clicked */
struct move_data the_pending_movement = { -1, -1, -1, 0 };


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


static void
gbk_cubeview_class_init (GbkCubeviewClass *klass)
{

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

static gboolean            
grab_focus  (GtkWidget *widget,
		GdkEventCrossing *event,
		gpointer data) 
{
  gtk_widget_grab_focus (widget);
  return FALSE;
}

static gboolean
cube_orientate_mouse (GtkWidget *w, GdkEventMotion *event, gpointer data)
{
  static gdouble last_mouse_x = -1;
  static gdouble last_mouse_y = -1;

  gint xmotion = 0;
  gint ymotion = 0;

  GbkCubeview *dc = GBK_CUBEVIEW (w);

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
  gbk_redisplay (dc);

  if (xmotion > 0)
    rotate_cube (1, 1);
  if (xmotion < 0)
    rotate_cube (1, 0);

  gbk_redisplay (dc);

  return FALSE;
}


/* orientate the whole cube with the arrow keys */
static gboolean
cube_orientate_keys (GtkWidget *w, GdkEventKey *event, gpointer data)
{
  GbkCubeview *dc = GBK_CUBEVIEW (w);

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
  gbk_redisplay (dc);

  /* We return TRUE here (disabling other event handlers)
     otherwise other widgets can steal the keyboard focus from our
     glwidget */
  return TRUE;
}


/* Rotate the cube about the z axis (relative to the viewer ) */
static gboolean
z_rotate (GtkWidget *w, GdkEventScroll *event, gpointer data)
{
  GbkCubeview *dc = GBK_CUBEVIEW (w);

  rotate_cube (2, !event->direction);

  gbk_redisplay (dc);

  return FALSE;
}

static gboolean
on_crossing  (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
  struct cublet_selection *cs = data;

  if (event->type == GDK_ENTER_NOTIFY)
    select_enable (cs);

  if (event->type == GDK_LEAVE_NOTIFY)
    select_disable (cs);

  return TRUE;
}


/* Disable / Enable the selection based on the state of button 1 */
static gboolean
enable_disable_selection (GtkWidget *w, GdkEventButton *event, gpointer data)
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


static void
gbk_cubeview_init (GbkCubeview *dc)
{
  struct cublet_selection *cs = NULL;

  initialize_gl_capability (GTK_WIDGET (dc));

  dc->glcontext = NULL;
  dc->gldrawable = NULL;
  dc->idle_id = 0;

  gtk_widget_add_events (GTK_WIDGET (dc),
			 GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK
			 | GDK_BUTTON_RELEASE_MASK
			 /* | GDK_BUTTON_MOTION_MASK */
			 | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
			 | GDK_VISIBILITY_NOTIFY_MASK
			 | GDK_POINTER_MOTION_MASK);

  GTK_WIDGET_SET_FLAGS (dc, GTK_CAN_FOCUS);

  cs = Xselect_create (GTK_WIDGET (dc), 50, 1, selection_func, &the_pending_movement );

  g_signal_connect (dc, "realize", G_CALLBACK (on_realize), NULL);
  g_signal_connect (dc, "expose-event", G_CALLBACK (on_expose), NULL);
  g_signal_connect (dc, "size-allocate", G_CALLBACK (resize_viewport), NULL);

  /* Grab the keyboard focus whenever the mouse enters the widget */
  g_signal_connect (dc, "enter-notify-event",
		    G_CALLBACK (grab_focus), NULL);

  g_signal_connect (dc, "key-press-event",
		    G_CALLBACK (cube_orientate_keys), NULL);

  g_signal_connect (dc, "motion-notify-event",
		    G_CALLBACK (cube_orientate_mouse), NULL);

  g_signal_connect (dc, "scroll-event",
		    G_CALLBACK (z_rotate), NULL);



  g_signal_connect (dc, "leave-notify-event",
		    G_CALLBACK (on_crossing), cs);

  g_signal_connect (dc, "enter-notify-event",
		    G_CALLBACK (on_crossing), cs);


  g_signal_connect (dc, "button-press-event",
		    G_CALLBACK (enable_disable_selection), cs);

  g_signal_connect (dc, "button-release-event",
		    G_CALLBACK (enable_disable_selection), cs);

  g_signal_connect (dc, "button-press-event",
		    G_CALLBACK (on_mouse_button), cs);
}

G_DEFINE_TYPE (GbkCubeview, gbk_cubeview, GTK_TYPE_DRAWING_AREA);


GtkWidget*
gbk_cubeview_new (void)
{
  return GTK_WIDGET (g_object_new (gbk_cubeview_get_type (), NULL));
}




static display display_anti_alias;
static display display_raw;

/* Resize callback.  */
static void
resize_viewport (GtkWidget *w, GtkAllocation *alloc, gpointer data)
{
  GbkCubeview *dc = GBK_CUBEVIEW (w);

  GLint min_dim;
  gint height = alloc->height;
  gint width = alloc->width;


  if (!GTK_WIDGET_REALIZED (w))
    return;

  if (!gdk_gl_drawable_gl_begin (dc->gldrawable, dc->glcontext))
    return;

  min_dim = (width < height) ? width : height;

  /* Ensure that cube is always the same proportions */
  glViewport ((width - min_dim) / 2, (height - min_dim) / 2, min_dim,  min_dim);
}

static gboolean
handleRedisplay (gpointer data)
{
  GbkCubeview *disp_ctx = data;
  disp_ctx->display_func (disp_ctx);
  g_source_remove (disp_ctx->idle_id);
  disp_ctx->idle_id = 0;

  return TRUE;
}

void
gbk_redisplay (GbkCubeview *dc)
{
  if (0 == dc->idle_id)
    dc->idle_id = g_idle_add (handleRedisplay, dc);
}

/* Expose callback.  Just redraw the scene */
static gboolean
on_expose (GtkWidget *w, GdkEventExpose *event, gpointer data)
{
  GbkCubeview *dc = GBK_CUBEVIEW (w);
  gbk_redisplay (dc);
  return FALSE;
}



static void
on_realize (GtkWidget *w, gpointer data)
{
  GbkCubeview *dc = GBK_CUBEVIEW (w);
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



void error_check (const char *file, int line_no, const char *string);


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
display_anti_alias (GbkCubeview *dc)
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
display_raw (GbkCubeview *dc)
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



#if !X_DISPLAY_MISSING
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#endif

static void
set_the_colours (GtkWidget *w, const char *progname)
{
#if !X_DISPLAY_MISSING
  int i;

  Display *dpy = GDK_WINDOW_XDISPLAY (gtk_widget_get_parent_window (w));

  for (i = 0; i < 6; ++i)
    {
      char *colour = 0;
      char resname[20];
      GdkColor xcolour;
      g_snprintf (resname, 20, "color%d", i);
      colour = XGetDefault (dpy, progname, resname);

      if (!colour)
	continue;

      if (!gdk_color_parse (colour, &xcolour))
	{
	  g_warning ("colour %s not in database\n", colour);
	}
      else
	{
	  /* convert colours to GLfloat values,  and set them */
	  const unsigned short full = ~0;
	  
	  struct cube_rendering r;

          r.pixbuf = NULL;
          r.texName = 0;
	  r.red = (GLfloat) xcolour.red / full;
	  r.green = (GLfloat) xcolour.green / full;
	  r.blue = (GLfloat) xcolour.blue / full;

       	  setColour (i, &r);
	}
    }
#endif
}
