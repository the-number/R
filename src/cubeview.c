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

#include <config.h>
#include "cubeview.h"
#include "select.h"
#include "control.h"
#include "drwBlock.h"
#include <GL/glu.h>
#include "glarea.h"
#include "textures.h"
#include <gdk/gdkkeysyms.h>

static void realize (GtkWidget * w);
static void size_allocate (GtkWidget * w, GtkAllocation * alloc);
static gboolean expose (GtkWidget * w, GdkEventExpose * event);

static GLboolean have_accumulation_buffer (void);

static gboolean on_mouse_button (GtkWidget * w, GdkEventButton * event,
				 gpointer data);


static void cubeview_generate_texname (GbkCubeview * cubeview, int i);


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

enum
{
  PROP_0 = 0,
  PROP_CUBE,
  PROP_ASPECT,
  PROP_FQ,
  PROP_COL0, PROP_COL1, PROP_COL2, PROP_COL3, PROP_COL4, PROP_COL5,
  PROP_IMG0, PROP_IMG1, PROP_IMG2, PROP_IMG3, PROP_IMG4, PROP_IMG5,
  PROP_SFC0, PROP_SFC1, PROP_SFC2, PROP_SFC3, PROP_SFC4, PROP_SFC5
};

static void animate_move (GbkCubeview * dc, const struct move_data *move);

static void
on_move (GbkCube * cube, struct move_data *move, GbkCubeview * cv)
{
  animate_move (cv, move);
}

static gboolean
unset_rotating_flag (gpointer data)
{
  GbkCubeview *cv = data;

  cv->is_rotating = FALSE;

  gbk_cubeview_redisplay (cv);
  return FALSE;
}


static void
on_rotate (GbkCubeview * cv)
{
  if (cv->rot_timer != 0)
    g_source_remove (cv->rot_timer);

  cv->is_rotating = TRUE;

  cv->rot_timer =
    g_timeout_add (cv->picture_rate * 4, unset_rotating_flag, cv);

  gbk_cubeview_redisplay (cv);
}

static void
cubeview_set_property (GObject * object,
		       guint prop_id,
		       const GValue * value, GParamSpec * pspec)
{
  GbkCubeview *cubeview = GBK_CUBEVIEW (object);

  switch (prop_id)
    {
    case PROP_CUBE:
      {
	cubeview->cube = g_value_get_object (value);
	scene_init (cubeview);

	if (cubeview->dim_id)
	  g_signal_handler_disconnect (cubeview->cube, cubeview->dim_id);

	cubeview->dim_id =
	  g_signal_connect_swapped (cubeview->cube, "notify::dimensions",
				    G_CALLBACK (scene_init), cubeview);

	if (cubeview->move_id)
	  g_signal_handler_disconnect (cubeview->cube, cubeview->move_id);

	cubeview->move_id =
	  g_signal_connect (cubeview->cube, "move", G_CALLBACK (on_move),
			    cubeview);

	if (cubeview->rotate_id)
	  g_signal_handler_disconnect (cubeview->cube, cubeview->rotate_id);

	cubeview->rotate_id =
	  g_signal_connect_swapped (cubeview->cube, "rotate",
				    G_CALLBACK (on_rotate), cubeview);
      }
      break;
    case PROP_ASPECT:
      {
	gfloat *asp = g_value_get_pointer (value);
	gfloat v[4];
	gfloat angle = asp[0];
	v[0] = asp[1];
	v[1] = asp[2];
	v[2] = asp[3];
	v[3] = 0;

	quarternion_from_rotation (&cubeview->qView, v, angle);
      }
      break;
    case PROP_FQ:
      cubeview->frameQty = g_value_get_int (value);
      break;
    case PROP_COL0:
    case PROP_COL1:
    case PROP_COL2:
    case PROP_COL3:
    case PROP_COL4:
    case PROP_COL5:
      {
	GdkColor *col = g_value_get_boxed (value);

	GLfloat *v = cubeview->colour[prop_id - PROP_COL0];

	v[0] = col->red / (GLfloat) 0xffff;
	v[1] = col->green / (GLfloat) 0xffff;
	v[2] = col->blue / (GLfloat) 0xffff;

	gbk_cubeview_redisplay (cubeview);
      }
      break;
    case PROP_IMG0:
    case PROP_IMG1:
    case PROP_IMG2:
    case PROP_IMG3:
    case PROP_IMG4:
    case PROP_IMG5:
      {
	cubeview->pixbuf[prop_id - PROP_IMG0] = g_value_get_object (value);

	glDeleteTextures (1, &cubeview->texName[prop_id - PROP_IMG0]);
	cubeview->texName[prop_id - PROP_IMG0] = -1;

	if (gtk_widget_get_realized (GTK_WIDGET (cubeview)))
	  cubeview_generate_texname (cubeview, prop_id - PROP_IMG0);

	gbk_cubeview_redisplay (cubeview);
      }
      break;
    case PROP_SFC0:
    case PROP_SFC1:
    case PROP_SFC2:
    case PROP_SFC3:
    case PROP_SFC4:
    case PROP_SFC5:
      {
	cubeview->surface[prop_id - PROP_SFC0] = g_value_get_enum (value);

	if (cubeview->surface[prop_id - PROP_SFC0] == SURFACE_COLOURED)
	  {
	    cubeview->pixbuf[prop_id - PROP_SFC0] = NULL;
	    glDeleteTextures (1, &cubeview->texName[prop_id - PROP_SFC0]);
	    cubeview->texName[prop_id - PROP_SFC0] = -1;
	  }

	gbk_cubeview_redisplay (cubeview);
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    };
}


static void
cubeview_generate_texname (GbkCubeview * cubeview, int i)
{
  GError *gerr = NULL;

  g_return_if_fail (gtk_widget_get_realized (GTK_WIDGET (cubeview)));

  if (cubeview->pixbuf[i] == NULL)
    return;

  cubeview->texName[i] =
    create_pattern_from_pixbuf (cubeview->pixbuf[i], &gerr);

  if (gerr)
    g_warning ("Cannot generate texture for face %d: %s", i, gerr->message);
}

static void
cubeview_get_property (GObject * object,
		       guint prop_id, GValue * value, GParamSpec * pspec)
{
  GbkCubeview *cubeview = GBK_CUBEVIEW (object);

  switch (prop_id)
    {
    case PROP_CUBE:
      g_value_set_object (value, cubeview->cube);
      break;
    case PROP_COL0:
    case PROP_COL1:
    case PROP_COL2:
    case PROP_COL3:
    case PROP_COL4:
    case PROP_COL5:
      {
	GLfloat *v = cubeview->colour[prop_id - PROP_COL0];
	GdkColor col;
	col.red = v[0] * 0xffff;
	col.green = v[1] * 0xffff;
	col.blue = v[2] * 0xffff;
	g_value_set_boxed (value, &col);
      }
      break;
    case PROP_FQ:
      g_value_set_int (value, cubeview->frameQty);
      break;
    case PROP_IMG0:
    case PROP_IMG1:
    case PROP_IMG2:
    case PROP_IMG3:
    case PROP_IMG4:
    case PROP_IMG5:
      g_value_set_object (value, cubeview->pixbuf[prop_id - PROP_IMG0]);
      break;
    case PROP_SFC0:
    case PROP_SFC1:
    case PROP_SFC2:
    case PROP_SFC3:
    case PROP_SFC4:
    case PROP_SFC5:
      g_value_set_enum (value, cubeview->surface[prop_id - PROP_SFC0]);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    };
}

enum
{
  ANIMATION_COMPLETE,
  n_SIGNALS
};

static guint signals[n_SIGNALS];


static GtkWidgetClass *parent_class = NULL;

static void gbk_cubeview_finalize (GObject * cv);

static void
gbk_cubeview_class_init (GbkCubeviewClass * klass)
{
  gint i;
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  GParamSpec *cube_param_spec;
  GParamSpec *aspect_param_spec;
  GParamSpec *fq_param_spec;


  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gbk_cubeview_finalize;

  gobject_class->set_property = cubeview_set_property;
  gobject_class->get_property = cubeview_get_property;

  fq_param_spec = g_param_spec_int ("animation-frames",
				    "Animation Frames",
				    "How many frames to display per animation",
				    0, 255, 2, G_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, PROP_FQ, fq_param_spec);


  cube_param_spec = g_param_spec_object ("cube",
					 "Cube",
					 "The cube which this widget views",
					 GBK_TYPE_CUBE, G_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, PROP_CUBE, cube_param_spec);

  aspect_param_spec = g_param_spec_pointer ("aspect",
					    "Aspect",
					    "The aspect from which this view sees the cube",
					    G_PARAM_WRITABLE);

  for (i = 0; i < 6; ++i)
    {
      gchar *name = g_strdup_printf ("color%d", i);
      gchar *nick = g_strdup_printf ("The colour for face %d", i);
      gchar *blurb =
	g_strdup_printf ("The GdkColor describing the colour for face %d", i);

      GParamSpec *colour = g_param_spec_boxed (name, nick, blurb,
					       GDK_TYPE_COLOR,
					       G_PARAM_READWRITE);

      g_object_class_install_property (gobject_class, PROP_COL0 + i, colour);

      g_free (name);
      g_free (nick);
      g_free (blurb);
    }


  for (i = 0; i < 6; ++i)
    {
      gchar *name = g_strdup_printf ("image%d", i);
      gchar *nick = g_strdup_printf ("The image for face %d", i);
      gchar *blurb =
	g_strdup_printf ("A GdkPixbuf to be rendered on face %d", i);

      GParamSpec *pixbuf = g_param_spec_object (name, nick, blurb,
						GDK_TYPE_PIXBUF,
						G_PARAM_READWRITE);

      g_object_class_install_property (gobject_class, PROP_IMG0 + i, pixbuf);

      g_free (name);
      g_free (nick);
      g_free (blurb);
    }

  for (i = 0; i < 6; ++i)
    {
      gchar *name = g_strdup_printf ("surface%d", i);
      gchar *nick = g_strdup_printf ("The surface for face %d", i);
      gchar *blurb =
	g_strdup_printf
	("How the image (if any) should be rendered to face %d", i);

      GParamSpec *sfc = g_param_spec_enum (name, nick, blurb,
					   GBK_TYPE_SURFACE,
					   SURFACE_COLOURED,
					   G_PARAM_READWRITE);

      g_object_class_install_property (gobject_class, PROP_SFC0 + i, sfc);

      g_free (name);
      g_free (nick);
      g_free (blurb);
    }



  g_object_class_install_property (gobject_class,
				   PROP_ASPECT, aspect_param_spec);

  signals[ANIMATION_COMPLETE] =
    g_signal_new ("animation-complete",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  0,
		  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  widget_class->realize = realize;
  widget_class->size_allocate = size_allocate;
  widget_class->expose_event = expose;

  GdkScreen *screen = gdk_screen_get_default ();
  GdkWindow *root = gdk_screen_get_root_window (screen);

  const GdkGLConfigMode mode[] = {
    GDK_GL_MODE_RGB | GDK_GL_MODE_DOUBLE | GDK_GL_MODE_DEPTH |
      GDK_GL_MODE_ACCUM,
    GDK_GL_MODE_RGB | GDK_GL_MODE_DOUBLE | GDK_GL_MODE_DEPTH,
  };

  for (i = 0; i < sizeof (mode) / sizeof (mode[0]); ++i)
    {
      klass->glconfig =
	gdk_gl_config_new_by_mode_for_screen (screen, mode[i]);

      if (klass->glconfig != NULL)
	break;
      else
	g_warning ("Cannot get visual for mode 0x%0x", mode[i]);
    }

  if (!klass->glconfig)
    g_error ("No suitable visual found.");


  GdkGLWindow *rootglwin = gdk_gl_window_new (klass->glconfig, root, 0);
  klass->master_ctx =
    gdk_gl_context_new (GDK_GL_DRAWABLE (rootglwin), 0, TRUE,
			GDK_GL_RGBA_TYPE);
  g_object_unref (rootglwin);
}

static void
initialize_gl_capability (GtkWidget * glxarea)
{
  GbkCubeviewClass *klass = GBK_CUBEVIEW_CLASS (G_OBJECT_GET_CLASS (glxarea));

  gtk_widget_set_gl_capability (glxarea,
				klass->glconfig,
				klass->master_ctx, TRUE, GDK_GL_RGBA_TYPE);
}

static gboolean
grab_focus (GtkWidget * widget, GdkEventCrossing * event, gpointer data)
{
  gtk_widget_grab_focus (widget);
  return FALSE;
}

static gboolean
cube_orientate_mouse (GtkWidget * w, GdkEventMotion * event, gpointer data)
{
  gint xmotion = 0;
  gint ymotion = 0;

  GbkCubeview *dc = GBK_CUBEVIEW (w);

  GdkWindow *window = gtk_widget_get_window (w);

  GdkModifierType mm;
  gdk_window_get_pointer (window, NULL, NULL, &mm);

  if (!(GDK_BUTTON1_MASK & mm))
    return FALSE;

  if (dc->last_mouse_x >= 0)
    xmotion = event->x - dc->last_mouse_x;

  if (dc->last_mouse_y >= 0)
    ymotion = event->y - dc->last_mouse_y;

  dc->last_mouse_x = event->x;
  dc->last_mouse_y = event->y;

  if (ymotion > 0)
    gbk_cubeview_rotate_cube (dc, 0, 1);
  if (ymotion < 0)
    gbk_cubeview_rotate_cube (dc, 0, 0);

  if (xmotion > 0)
    gbk_cubeview_rotate_cube (dc, 1, 1);
  if (xmotion < 0)
    gbk_cubeview_rotate_cube (dc, 1, 0);

  return FALSE;
}


/* orientate the whole cube with the arrow keys */
static gboolean
cube_orientate_keys (GtkWidget * w, GdkEventKey * event, gpointer data)
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

  gbk_cubeview_rotate_cube (dc, axis, dir);

  /* We return TRUE here (disabling other event handlers)
     otherwise other widgets can steal the keyboard focus from our
     glwidget */
  return TRUE;
}


/* Rotate the cube about the z axis (relative to the viewer ) */
static gboolean
z_rotate (GtkWidget * w, GdkEventScroll * event, gpointer data)
{
  GbkCubeview *dc = GBK_CUBEVIEW (w);

  gbk_cubeview_rotate_cube (dc, 2, !event->direction);

  gbk_cubeview_redisplay (dc);

  return FALSE;
}

static gboolean
on_crossing (GtkWidget * widget, GdkEventCrossing * event, gpointer data)
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
enable_disable_selection (GtkWidget * w, GdkEventButton * event,
			  gpointer data)
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
gbk_cubeview_finalize (GObject * o)
{
  GbkCubeview *cv = GBK_CUBEVIEW (o);
  select_destroy (cv->cs);

  if (cv->dim_id)
    g_signal_handler_disconnect (cv->cube, cv->dim_id);

  if (cv->rotate_id)
    g_signal_handler_disconnect (cv->cube, cv->rotate_id);

  if (cv->move_id)
    g_signal_handler_disconnect (cv->cube, cv->move_id);

  if (cv->idle_id)
    g_source_remove (cv->idle_id);

  if (cv->animation_timeout)
    g_source_remove (cv->animation_timeout);
}


static void
gbk_cubeview_init (GbkCubeview * dc)
{
  dc->cs = NULL;
  dc->pending_movement = NULL;

  quarternion_set_to_unit (&dc->qView);

  initialize_gl_capability (GTK_WIDGET (dc));

  dc->glcontext = NULL;
  dc->gldrawable = NULL;
  dc->idle_id = 0;
  dc->rotate_id = 0;
  dc->move_id = 0;
  dc->dim_id = 0;

  dc->picture_rate = 40;
  dc->frameQty = 2;
  dc->animation_angle = 0;
  dc->animation_timeout = 0;


  gtk_widget_add_events (GTK_WIDGET (dc),
			 GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK
			 | GDK_BUTTON_RELEASE_MASK
			 /* | GDK_BUTTON_MOTION_MASK */
			 | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
			 | GDK_VISIBILITY_NOTIFY_MASK
			 | GDK_POINTER_MOTION_MASK);

  gtk_widget_set_can_focus (GTK_WIDGET (dc), TRUE);

  dc->cs = select_create (GTK_WIDGET (dc), 50, 1, selection_func, dc);

  /* Grab the keyboard focus whenever the mouse enters the widget */
  g_signal_connect (dc, "enter-notify-event", G_CALLBACK (grab_focus), NULL);

  g_signal_connect (dc, "key-press-event",
		    G_CALLBACK (cube_orientate_keys), NULL);

  g_signal_connect (dc, "motion-notify-event",
		    G_CALLBACK (cube_orientate_mouse), NULL);

  g_signal_connect (dc, "scroll-event", G_CALLBACK (z_rotate), NULL);

  g_signal_connect (dc, "leave-notify-event",
		    G_CALLBACK (on_crossing), dc->cs);

  g_signal_connect (dc, "enter-notify-event",
		    G_CALLBACK (on_crossing), dc->cs);

  g_signal_connect (dc, "button-press-event",
		    G_CALLBACK (enable_disable_selection), dc->cs);

  g_signal_connect (dc, "button-release-event",
		    G_CALLBACK (enable_disable_selection), dc->cs);

  g_signal_connect (dc, "button-press-event",
		    G_CALLBACK (on_mouse_button), dc->cs);

  dc->colour[0][0] = 1.0;
  dc->colour[1][1] = 1.0;
  dc->colour[2][2] = 1.0;
  dc->colour[3][1] = 1.0;
  dc->colour[3][2] = 1.0;
  dc->colour[4][0] = 1.0;
  dc->colour[4][2] = 1.0;
  dc->colour[5][0] = 1.0;
  dc->colour[5][1] = 1.0;

  dc->pixbuf[0] = NULL;
  dc->pixbuf[1] = NULL;
  dc->pixbuf[2] = NULL;
  dc->pixbuf[3] = NULL;
  dc->pixbuf[4] = NULL;
  dc->pixbuf[5] = NULL;

  dc->texName[0] = -1;
  dc->texName[1] = -1;
  dc->texName[2] = -1;
  dc->texName[3] = -1;
  dc->texName[4] = -1;
  dc->texName[5] = -1;

  dc->is_rotating = FALSE;
  dc->rot_timer = 0;
}

G_DEFINE_TYPE (GbkCubeview, gbk_cubeview, GTK_TYPE_DRAWING_AREA);


GtkWidget *
gbk_cubeview_new (GbkCube * cube)
{
  return
    GTK_WIDGET (g_object_new (gbk_cubeview_get_type (), "cube", cube, NULL));
}



static display display_anti_alias;
static display display_raw;

/* Resize callback.  */
static void
size_allocate (GtkWidget * w, GtkAllocation * alloc)
{
  GbkCubeview *dc = GBK_CUBEVIEW (w);

  GLint min_dim;
  gint height = alloc->height;
  gint width = alloc->width;


  if (!gtk_widget_get_realized (w))
    return;

  if (GTK_WIDGET_CLASS (parent_class)->size_allocate)
    GTK_WIDGET_CLASS (parent_class)->size_allocate (w, alloc);

  if (!gdk_gl_drawable_gl_begin (dc->gldrawable, dc->glcontext))
    return;

  min_dim = (width < height) ? width : height;

  /* Ensure that cube is always the same proportions */
  glViewport ((width - min_dim) / 2, (height - min_dim) / 2, min_dim,
	      min_dim);
}

static gboolean
handleRedisplay (gpointer data)
{
  GbkCubeview *cv = data;

  if (cv->is_rotating || !have_accumulation_buffer ())
    display_raw (cv);
  else
    display_anti_alias (cv);

  g_source_remove (cv->idle_id);
  cv->idle_id = 0;

  return TRUE;
}

void
gbk_cubeview_redisplay (GbkCubeview * cv)
{
  if (0 == cv->idle_id)
    cv->idle_id = g_idle_add (handleRedisplay, cv);
}

/* Expose callback.  Just redraw the scene */
static gboolean
expose (GtkWidget * w, GdkEventExpose * event)
{
  GbkCubeview *dc = GBK_CUBEVIEW (w);

  if (GTK_WIDGET_CLASS (parent_class)->expose_event)
    if (GTK_WIDGET_CLASS (parent_class)->expose_event (w, event))
      return TRUE;

  gbk_cubeview_redisplay (dc);
  return FALSE;
}


static void
realize (GtkWidget * w)
{
  GbkCubeview *dc = GBK_CUBEVIEW (w);
  int i;

  if (GTK_WIDGET_CLASS (parent_class)->realize)
    GTK_WIDGET_CLASS (parent_class)->realize (w);

  dc->glcontext = gtk_widget_get_gl_context (w);
  dc->gldrawable = gtk_widget_get_gl_drawable (w);

  if (!gdk_gl_drawable_gl_begin (dc->gldrawable, dc->glcontext))
    {
      g_critical ("Cannot initialise gl drawable\n");
      return;
    }

  for (i = 0; i < 6; ++i)
    cubeview_generate_texname (dc, i);

  gtk_widget_set_size_request (w, 300, 300);

  dc->last_mouse_x = -1;
  dc->last_mouse_y = -1;

  gdk_gl_drawable_gl_end (dc->gldrawable);

  dc->background_cursor = gdk_cursor_new_for_display (gtk_widget_get_display (w), GDK_CROSSHAIR);
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
render_scene (GbkCubeview * cv, GLint jitter)
{
  projection_init (&cv->scene, jitter);

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gbk_cubeview_model_view_init (cv);
  ERR_CHECK ("Error in display");

  drawCube (cv->cube, FALSE, cv);
  ERR_CHECK ("Error in display");
}


/* Reset the bit planes, and render the scene using the accumulation
   buffer for antialiasing*/
static void
display_anti_alias (GbkCubeview * dc)
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
      render_scene (dc, jitter);
      glAccum (GL_ACCUM, 1.0 / 8.0);
    }

  glAccum (GL_RETURN, 1.0);

  gdk_gl_drawable_swap_buffers (dc->gldrawable);
}



/* Reset the bit planes, and render the scene,
   without anti-aliasing.
 */
static void
display_raw (GbkCubeview * dc)
{
  if (!gdk_gl_drawable_make_current (dc->gldrawable, dc->glcontext))
    {
      g_critical ("Cannot set gl drawable current\n");
      return;
    }

  ERR_CHECK ("Error in display");

  render_scene (dc, 0);

  gdk_gl_drawable_swap_buffers (dc->gldrawable);
}


void
gbk_cubeview_set_frame_qty (GbkCubeview * dc, int frames)
{
  g_object_set (dc, "animation-frames", frames, NULL);
}

gboolean
gbk_cubeview_is_animating (GbkCubeview * dc)
{
  return dc->current_move != NULL;
}


/* Wrapper to set the modelview matrix */
void
gbk_cubeview_model_view_init (GbkCubeview * cv)
{
  struct scene_view *scene = &cv->scene;
  GbkCube *cube = cv->cube;
  /* start with the cube slightly skew,  so we can see all its aspects */
  GLdouble cube_orientation[2] = { 15.0, 15.0 };

  Matrix m;

  /* Update viewer position in modelview matrix */

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  GLdouble eye = scene->bounding_sphere_radius + scene->cp_near;

  glTranslatef (0, 0, -eye);

  Quarternion o;
  quarternion_set_to_unit (&o);
  quarternion_pre_mult (&o, &cube->orientation);
  quarternion_pre_mult (&o, &cv->qView);

  quarternion_to_matrix (m, &o);

  glMultMatrixf (m);


  /* skew the cube */
  glRotatef (cube_orientation[1], 1, 0, 0);	/* horizontally */
  glRotatef (cube_orientation[0], 0, 1, 0);	/* vertically */

#if 0
  /*
   * DM 3-Jan-2004
   *
   * Add a couple of 90 degree turns to get the top and right faces in their
   * logical positions when the program starts up.
   */
  glRotatef (90.0, 1, 0, 0);
  glRotatef (-90.0, 0, 0, 1);
#endif
}

/* Rotate cube about axis (screen relative) */
void
gbk_cubeview_rotate_cube (GbkCubeview * cv, int axis, int dir)
{
  GbkCube *cube = cv->cube;
  /* how many degrees to turn the cube with each hit */
  const GLdouble step = 2.0;

  vector v = { 0, 0, 0, 0 };

  g_assert (axis >= 0);
  g_assert (axis < 3);

  if (dir)
    v[axis] = -1;
  else
    v[axis] = 1;

  /* We need to Transform v by the aspect of cv */
  Matrix m;
  Quarternion q;
  quarternion_get_inverse (&q, &cv->qView);

  quarternion_to_matrix (m, &q);
  vector_transform_in_place (v, m);

  gbk_cube_rotate (cube, v, step);
}




/* Animation related stuff */

/*
    User Interface functions for the Cube
    Copyright (C) 1998,  2003  John Darrington
                  2004  John Darrington,  Dale Mellor
		  2011  John Darrington
*/

static gboolean animate_callback (gpointer data);


/* handle mouse clicks */
static gboolean
on_mouse_button (GtkWidget * w, GdkEventButton * event, gpointer data)
{
  GbkCubeview *cv = GBK_CUBEVIEW (w);

  struct cublet_selection *cs = data;
  if (event->type != GDK_BUTTON_PRESS)
    return FALSE;
 
  if  (event->button != 1)
    return FALSE;


  /* Don't let a user make a move,  whilst one is already in progress,
     otherwise the cube falls to bits. */
  if (gbk_cubeview_is_animating (cv))
    return FALSE;

  /* Force an update on the selection mechanism.  This avoids
     annoying instances of moves occuring  when a rotation was 
     intended. (Happens when experienced users become too fast). */
  select_update (cv, cs);

  /* Make a move */
  if (select_is_selected (cs))
    /* and tell the blocks.c library that a move has taken place */
    gbk_cube_rotate_slice (cv->cube, cv->pending_movement);

  return FALSE;
}


/* Does exactly what it says on the tin :-) */
static void
animate_move (GbkCubeview * cv, const struct move_data *move)
{
  /* Abort any current animation */
  if (cv->current_move)
    move_unref (cv->current_move);

  cv->current_move = move_ref (move);

  cv->animation_angle = 90.0 * move_turns (cv->current_move);

  cv->animation_timeout =
    g_timeout_add (cv->picture_rate, animate_callback, cv);
}


/* a timeout  calls this func,  to animate the cube */
static gboolean
animate_callback (gpointer data)
{
  GbkCubeview *cv = data;

  /* how many degrees motion per frame */
  GLfloat increment = 90.0 / (cv->frameQty + 1);

  /*  decrement the current angle */
  cv->animation_angle -= increment;

  /* and redraw it */
  gbk_cubeview_redisplay (cv);

  if (cv->animation_angle > 0)
    {
      /* call this timeout again */
      cv->animation_timeout =
	g_timeout_add (cv->picture_rate, animate_callback, data);
    }
  else
    {
      /* we have finished the animation sequence now */
      move_unref (cv->current_move);
      cv->current_move = NULL;

      g_signal_emit (cv, signals[ANIMATION_COMPLETE], 0);
      select_update (cv, cv->cs);
      cv->animation_timeout = 0;
    }

  return FALSE;
}				/* end animate () */




GType
gbk_cubeview_surface_get_type (void)
{
  static GType etype = 0;
  if (etype == 0)
    {
      static const GEnumValue values[] = {
	{SURFACE_COLOURED, "SURFACE_COLOURED", "Plain Surface"},
	{SURFACE_TILED, "SURFACE_TILED", "Tiled Surface"},
	{SURFACE_MOSAIC, "SURFACE_MOSAIC", "Mosaic Surface"},
	{0, NULL, NULL}
      };

      etype = g_enum_register_static
	(g_intern_static_string ("CubeviewSurface"), values);

    }
  return etype;
}
