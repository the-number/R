/*
    GNUbik -- A 3 dimensional magic cube game.
    Copyright (C) 2003, 2011  John Darrington

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
#include "quarternion.h"
#include <GL/glu.h>
#include <math.h>
#include "glarea.h"
#include "cube.h"

#include <gdk/gdkkeysyms.h>



struct jitter_v
{
  GLdouble x;
  GLdouble y;
};

static const struct jitter_v j8[8] = {
  {0.5625, 0.4375}, {0.0625, 0.9375}, {0.3125, 0.6875}, {0.6875, 0.8125},
  {0.8125, 0.1875}, {0.9375, 0.5625}, {0.4375, 0.0625}, {0.1875, 0.3125}
};

void
perspectiveSet (struct scene_view *scene)
{
  gluPerspective (scene->fovy, 1, scene->cp_near, scene->cp_far);
}



/* accFrustum and accPerspective are taken from p397 of the Red Book */

static void
accFrustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
	    GLdouble zNear, GLdouble zFar, GLdouble pixdx, GLdouble pixdy,
	    GLdouble eyedx, GLdouble eyedy, GLdouble focus)
{
  GLdouble dx = 0;
  GLdouble dy = 0;
  GLdouble xwsize, ywsize;
  GLint viewport[4];

  glGetIntegerv (GL_VIEWPORT, viewport);

  xwsize = right - left;
  ywsize = top - bottom;

  dx = -(pixdx * xwsize / (GLdouble) viewport[2] + eyedx * zNear / focus);
  dy = -(pixdy * ywsize / (GLdouble) viewport[3] + eyedy * zNear / focus);


  glFrustum (left + dx, right + dy, bottom + dy, top + dy, zNear, zFar);
}


static void
accPerspective (const struct scene_view *sv,
		GLdouble aspect,
		GLdouble pixdx, GLdouble pixdy,
		GLdouble eyedx, GLdouble eyedy, GLdouble focus)
{
  GLdouble fov2, left, right, top, bottom;

  fov2 = (sv->fovy * M_PI / 180.0) / 2.0;
  top = sv->cp_near / (cos (fov2) / sin (fov2));
  bottom = -top;
  right = top * aspect;
  left = -right;
  accFrustum (left, right, bottom, top,
	      sv->cp_near, sv->cp_far, pixdx, pixdy, eyedx, eyedy, focus);
}


void
scene_init (GbkCubeview * cv)
{
  GbkCube *cube = cv->cube;
  cv->scene.bounding_sphere_radius =
    gbk_cube_get_size (cube, 0) * gbk_cube_get_size (cube, 0);
  cv->scene.bounding_sphere_radius +=
    gbk_cube_get_size (cube, 1) * gbk_cube_get_size (cube, 1);
  cv->scene.bounding_sphere_radius +=
    gbk_cube_get_size (cube, 2) * gbk_cube_get_size (cube, 2);
  cv->scene.bounding_sphere_radius = sqrt (cv->scene.bounding_sphere_radius);

  cv->scene.fovy = 33.0;
  cv->scene.cp_near =
    cv->scene.bounding_sphere_radius / (tan (cv->scene.fovy * M_PI / 360.0));
  cv->scene.cp_far = cv->scene.cp_near + 2 * cv->scene.bounding_sphere_radius;
}

void
projection_init (struct scene_view *scene, int jitter)
{
  glHint (GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);

  glEnable (GL_DEPTH_TEST);
  glClearColor (0, 0, 0, 0);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  accPerspective (scene, 1, j8[jitter].x, j8[jitter].y, 0.0, 0.0, 1.0);
}
