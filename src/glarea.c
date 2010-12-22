/*
  GNUbik -- A 3 dimensional magic cube game.
  Copyright (C) 2003, 2004, 2009  John Darrington

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
#include "colour-sel.h"

#include <GL/gl.h>
#include <GL/glu.h>

#include <gtk/gtk.h>
#include <gtk/gtkwidget.h>
#include <gdk/gdkkeysyms.h>

#include <gtk/gtkgl.h>
#include <gdk/gdkglconfig.h>


typedef void display (GtkWidget *);

static void display_anti_alias (GtkWidget * glxarea);
static void display_raw (GtkWidget * glxarea);

static display *display_func = NULL;


static void graphics_area_init (GtkWidget * w, gpointer data);

static gboolean handleRedisplay (gpointer glxarea);

void projection_init (int jitter);


static void resize (GtkWidget * w, GtkAllocation * alloc, gpointer data);


static gboolean cube_orientate_keys (GtkWidget * w,
				     GdkEventKey * event,
				     gpointer clientData);

static gboolean z_rotate (GtkWidget * w, GdkEventScroll * event,
			  gpointer clientData);


static gboolean cube_orientate_mouse (GtkWidget * w,
				      GdkEventMotion * event,
				      gpointer clientData);


static gboolean buttons (GtkWidget * w,
			 GdkEventButton * event, gpointer clientData);



static gboolean cube_controls (GtkWidget * w,
			       GdkEventButton * event, gpointer clientData);


static void expose (GtkWidget * w, GdkEventExpose * event);


static GtkWidget *glwidget;


void
re_initialize_glarea (void)
{
  graphics_area_init (glwidget, 0);
}


/* Resize callback.  */
static void
resize (GtkWidget * w, GtkAllocation * alloc, gpointer data)
{
  GLint min_dim;
  gint height = alloc->height;
  gint width = alloc->width;

  if (!GTK_WIDGET_REALIZED (w))
    return;

  GdkGLContext *glcontext = gtk_widget_get_gl_context (w);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (w);

  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return;

  min_dim = (width < height) ? width : height;

  /* Ensure that cube is always the same proportions */
  glViewport ((width - min_dim) / 2, (height - min_dim) / 2, min_dim,
	      min_dim);
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

static void register_gl_callbacks (GtkWidget * glxarea);

static void
initialize_gl_capability (GtkWidget * glxarea)
{
  gint i;

  const GdkGLConfigMode mode[] = {
    GDK_GL_MODE_RGB | GDK_GL_MODE_DOUBLE | GDK_GL_MODE_ACCUM | GDK_GL_MODE_DEPTH,

    GDK_GL_MODE_RGB | GDK_GL_MODE_DOUBLE  | GDK_GL_MODE_DEPTH,
  };

  GdkScreen *screen = gtk_widget_get_screen (glxarea);

  GdkGLConfig *glconfig = NULL;

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

  gtk_widget_set_gl_capability (glxarea, glconfig, 0, TRUE, GDK_GL_RGBA_TYPE);
}



GtkWidget *
create_gl_area (void)
{
  const GtkTargetEntry target[2] = {
    {"text/uri-list", 0, RDRAG_FILELIST},
    {"application/x-color", 0, RDRAG_COLOUR},
  };

  GtkWidget *glxarea = gtk_drawing_area_new ();


  initialize_gl_capability (glxarea);

  gtk_drag_dest_set (glxarea, GTK_DEST_DEFAULT_ALL,
		     target, 2, GDK_ACTION_COPY);


  g_signal_connect (glxarea, "drag_data_received",
		    G_CALLBACK (drag_data_received), (gpointer) - 1);


  glwidget = glxarea;

  register_gl_callbacks (glxarea);


  return glxarea;
}


extern float cursorAngle;

static void
graphics_area_init (GtkWidget * w, gpointer data)
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context (w);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (w);

  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    {
      g_critical ("Cannot initialise gl drawable\n");
      return;
    }

  gtk_widget_set_size_request (w, 300, 300);

  gtk_window_set_focus (GTK_WINDOW (gtk_widget_get_toplevel (w)), w);

  lighting_init ();

  texInit ();

  projection_init (0);

  modelViewInit ();

  set_the_colours (w, "gnubik");
}


static void
register_gl_callbacks (GtkWidget * glxarea)
{
  g_signal_connect (glxarea, "realize", G_CALLBACK (graphics_area_init), 0);

  g_signal_connect (glxarea, "expose_event", G_CALLBACK (expose), 0);

  g_signal_connect (glxarea, "size-allocate", G_CALLBACK (resize), 0);


  gtk_widget_add_events (GTK_WIDGET (glxarea),
			 GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK
			 | GDK_BUTTON_RELEASE_MASK
			 /* | GDK_BUTTON_MOTION_MASK */
			 | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
			 | GDK_VISIBILITY_NOTIFY_MASK
			 | GDK_POINTER_MOTION_MASK);


  GTK_WIDGET_SET_FLAGS (glxarea, GTK_CAN_FOCUS);


  g_signal_connect (glxarea, "key_press_event",
		    G_CALLBACK (cube_orientate_keys), 0);

  g_signal_connect (glxarea, "motion_notify_event",
		    G_CALLBACK (cube_orientate_mouse), 0);


  g_signal_connect (glxarea, "scroll_event", G_CALLBACK (z_rotate), 0);


  g_signal_connect (glxarea, "button_press_event", G_CALLBACK (buttons), 0);

  g_signal_connect (glxarea, "button_release_event", G_CALLBACK (buttons), 0);


  g_signal_connect (glxarea, "button_press_event",
		    G_CALLBACK (cube_controls), 0);
}


static gboolean redisplayPending = FALSE;

static guint idle_id;

void
postRedisplay (void)
{
  if (display_func == NULL)
    {
      GdkGLContext *glcontext = gtk_widget_get_gl_context (glwidget);
      GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (glwidget);

      if (!gdk_gl_drawable_make_current (gldrawable, glcontext))
	{
	  g_critical ("Cannot set gl drawable current\n");
	  return;
	}

      if (have_accumulation_buffer ())
	display_func = display_anti_alias;
      else
	display_func = display_raw;
    }

  if (!redisplayPending)
    {
      idle_id = g_idle_add (handleRedisplay, glwidget);

      redisplayPending = TRUE;
    }
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




/* Expose callback.  Just redraw the scene */
static void
expose (GtkWidget * w, GdkEventExpose * event)
{
  postRedisplay ();
}




static gboolean button_down = FALSE;

/* Rotate the cube about the z axis (relative to the viewer ) */
gboolean
z_rotate (GtkWidget * w, GdkEventScroll * event, gpointer clientData)
{

  rotate_cube (2, !event->direction);

  return FALSE;

}

/* Record the state of button 1 */
gboolean
buttons (GtkWidget * w, GdkEventButton * event, gpointer clientData)
{

  /* In GTK1-2,  buttons 4 and 5 mean mouse wheel scrolling */
  if (event->button == 4)
    rotate_cube (2, 1);

  if (event->button == 5)
    rotate_cube (2, 0);



  if (event->button != 1)
    return FALSE;


  if (event->type == GDK_BUTTON_PRESS)
    {
      button_down = TRUE;
      disableSelection ();
    }
  else if (event->type == GDK_BUTTON_RELEASE)
    {
      enableSelection ();
      button_down = FALSE;
    }


  return FALSE;
}


gboolean
cube_orientate_mouse (GtkWidget * w,
		      GdkEventMotion * event, gpointer clientData)
{

  static gdouble last_mouse_x = -1;
  static gdouble last_mouse_y = -1;

  gint xmotion = 0;
  gint ymotion = 0;



  if (!button_down)
    return FALSE;

  if (itemIsSelected ())
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

  if (xmotion > 0)
    rotate_cube (1, 1);
  if (xmotion < 0)
    rotate_cube (1, 0);


  return FALSE;
}

gboolean
cube_orientate_keys (GtkWidget * w, GdkEventKey * event, gpointer clientData)
{

  int shifted = 0;

  if (event->state & GDK_SHIFT_MASK)
    shifted = 1;

  arrows (event->keyval, shifted);

  return TRUE;
}





/* Input callback.  Despatch input events to approprite handlers */
gboolean
cube_controls (GtkWidget * w, GdkEventButton * event, gpointer clientData)
{
  if (event->type != GDK_BUTTON_PRESS)
    return TRUE;


  mouse (event->button);

  return TRUE;
}





/* Pair of mutually co-operating funcs to handle redisplays,
   avoiding unnecessary overhead.  For example when the window
   is covered by another one
*/

static gboolean
handleRedisplay (gpointer glxarea)
{
  display_func (GTK_WIDGET (glxarea));
  redisplayPending = FALSE;

  g_source_remove (idle_id);

  return TRUE;
}


static void
set_mouse_cursor (GtkWidget * glxarea)
{
  const unsigned char *mask_bits;
  const unsigned char *data_bits;
  int hot_x, hot_y;
  int width, height;


  GdkCursor *cursor;
  GdkPixmap *source, *mask;
  GdkColor fg = { 0, 65535, 65535, 65535 };	/* White. */
  GdkColor bg = { 0, 0, 0, 0 };	/* Black. */

  if (itemIsSelected ())
    {
      get_cursor (cursorAngle, &data_bits, &mask_bits, &height, &width,
		  &hot_x, &hot_y);


      source = gdk_bitmap_create_from_data (NULL, (const gchar *) data_bits,
					    width, height);
      mask = gdk_bitmap_create_from_data (NULL, (const gchar *) mask_bits,
					  width, height);

      cursor =
	gdk_cursor_new_from_pixmap (source, mask, &fg, &bg, hot_x, hot_y);
      g_object_unref (source);
      g_object_unref (mask);
    }
  else
    {
      GdkDisplay *display = gtk_widget_get_display (glxarea);
      cursor = gdk_cursor_new_for_display (display, GDK_CROSSHAIR);
    }

  gdk_window_set_cursor (glxarea->window, cursor);
  gdk_cursor_unref (cursor);

}



static void
render_scene (GLint jitter)
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  projection_init (jitter);
  modelViewInit ();
  ERR_CHECK ("Error in display");

  drawCube ();
  ERR_CHECK ("Error in display");
}


/* Reset the bit planes, and render the scene using the accumulation
   buffer for antialiasing*/
static void
display_anti_alias (GtkWidget * glxarea)
{
  int jitter;

  GdkGLContext *glcontext = gtk_widget_get_gl_context (glxarea);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (glxarea);

  if (!gdk_gl_drawable_make_current (gldrawable, glcontext))
    {
      g_critical ("Cannot set gl drawable current\n");
      return;
    }

  projection_init (0);
  modelViewInit ();
  ERR_CHECK ("Error in display");

  set_mouse_cursor (glxarea);

  glClear (GL_ACCUM_BUFFER_BIT);
  ERR_CHECK ("Error in display");

  for (jitter = 0; jitter < 8; ++jitter)
    {
      render_scene (jitter);
      glAccum (GL_ACCUM, 1.0 / 8.0);
    }

  glAccum (GL_RETURN, 1.0);

  gdk_gl_drawable_swap_buffers (gldrawable);
}



/* Reset the bit planes, and render the scene,
   without anti-aliasing.
 */
static void
display_raw (GtkWidget * glxarea)
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context (glxarea);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (glxarea);

  if (!gdk_gl_drawable_make_current (gldrawable, glcontext))
    {
      g_critical ("Cannot set gl drawable current\n");
      return;
    }

  ERR_CHECK ("Error in display");

  set_mouse_cursor (glxarea);

  render_scene (0);

  gdk_gl_drawable_swap_buffers (gldrawable);
}
