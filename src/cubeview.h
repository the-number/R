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


#ifndef __GBK_CUBEVIEW_H__
#define __GBK_CUBEVIEW_H__

#include <glib-object.h>
#include <gtk/gtkdrawingarea.h>

#include <gtk/gtkgl.h>
#include <gdk/gdkglconfig.h>
#include "cube.h"

struct animation;

enum surface
{
  SURFACE_COLOURED,
  SURFACE_TILED,
  SURFACE_MOSAIC
};

struct scene_view
{
  GLdouble fovy;
  GLdouble cp_near;
  GLdouble cp_far;
  GLdouble bounding_sphere_radius;
};

/*
 * Type macros.
 */
#define GBK_TYPE_CUBEVIEW                  (gbk_cubeview_get_type ())
#define GBK_CUBEVIEW(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GBK_TYPE_CUBEVIEW, GbkCubeview))
#define GBK_IS_CUBEVIEW(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GBK_TYPE_CUBEVIEW))
#define GBK_CUBEVIEW_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GBK_TYPE_CUBEVIEW, GbkCubeviewClass))
#define GBK_IS_CUBEVIEW_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GBK_TYPE_CUBEVIEW))
#define GBK_CUBEVIEW_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GBK_TYPE_CUBEVIEW, GbkCubeviewClass))

typedef struct _GbkCubeview GbkCubeview;
typedef struct _GbkCubeviewClass GbkCubeviewClass;


typedef void display (GbkCubeview *);

struct _GbkCubeview
{
  GtkDrawingArea parent_instance;

  GbkCube *cube;

  struct cublet_selection *cs;
  struct scene_view scene;

  /* instance members */
  guint idle_id;

  guint rotate_id;
  guint move_id;
  guint dim_id;
  guint animation_timeout;

  GdkGLContext *glcontext;
  GdkGLDrawable *gldrawable;

  /* The move that will take place when the mouse is clicked */
  struct move_data *pending_movement;

  /* Position of the mouse cursor */
  gdouble last_mouse_x;
  gdouble last_mouse_y;

  /* Angle of the mouse cursor */
  float cursor_angle;
  /* The current angle through which an animation has turned */
  GLfloat animation_angle;

  /* the number of frames drawn when a slice of the cube rotates 90 degrees */
  int frameQty;
  /* picture rate in ms. 40ms = 25 frames per sec */
  int picture_rate;

  const struct move_data *current_move;

  Quarternion qView;

  GLfloat colour[6][3];
  GdkPixbuf *pixbuf[6];
  GLuint texName[6];
  enum surface surface[6];

  gboolean is_rotating;
  guint rot_timer;

  /* The cursor to use when nothing is selected */
  GdkCursor *background_cursor ;
};

struct _GbkCubeviewClass
{
  GtkDrawingAreaClass parent_class;

  /* class members */
  GdkGLContext *master_ctx;
  GdkGLConfig *glconfig;
};

/* used by GBK_TYPE_CUBEVIEW */
GType gbk_cubeview_get_type (void);

/*
 * Method definitions.
 */
GtkWidget *gbk_cubeview_new (GbkCube * c);


/* Rotate the cube about the axis (screen relative) in direction dir */
void gbk_cubeview_redisplay (GbkCubeview * dc);

void error_check (const char *file, int line_no, const char *string);
#define ERR_CHECK(string)  error_check (__FILE__,__LINE__,string)

void gbk_cubeview_set_frame_qty (GbkCubeview * dc, int frames);

gboolean gbk_cubeview_is_animating (GbkCubeview * dc);

void gbk_cubeview_model_view_init (GbkCubeview * cv);

void gbk_cubeview_rotate_cube (GbkCubeview * cv, int axis, int dir);


GType gbk_cubeview_surface_get_type (void);


#define GBK_TYPE_SURFACE (gbk_cubeview_surface_get_type ())


#endif /* __GBK_CUBEVIEW_H__ */
