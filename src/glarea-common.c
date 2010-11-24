/*
    GNUbik -- A 3 dimensional magic cube game.
    Copyright (C) 2003  John Darrington

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
#include "gnubik.h"

GLdouble fovy = -1;
GLdouble cp_near = -1;
GLdouble cp_far = -1;


/* Start with the unit quarternion */
Quarternion cube_view = { 1, 0, 0, 0 };

static GLint bounding_sphere_radius = 0;

struct jitter_v
{
  GLdouble x;
  GLdouble y;
};

static struct jitter_v j8[8] = {
  {0.5625, 0.4375}, {0.0625, 0.9375}, {0.3125, 0.6875}, {0.6875, 0.8125},
  {0.8125, 0.1875}, {0.9375, 0.5625}, {0.4375, 0.0625}, {0.1875, 0.3125}
};



/* Wrapper to set the modelview matrix */
void
modelViewInit (void)
{
  /* start with the cube slightly skew,  so we can see all its aspects */
  GLdouble cube_orientation[2] = { 15.0, 15.0 };

  const GLint origin[] = { 0, 0, 0 };
  Matrix m;

  /* Update viewer position in modelview matrix */

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  gluLookAt (0, 0, bounding_sphere_radius + cp_near,
	     origin[0], origin[1], origin[2], 0.0, 1.0, 0.0);



  quarternion_to_matrix (m, &cube_view);
  glMultMatrixf (m);

  /* skew the cube */
  glRotatef (cube_orientation[1], 1, 0, 0);	/* horizontally */
  glRotatef (cube_orientation[0], 0, 1, 0);	/* vertically */

  /*
   * DM 3-Jan-2004
   *
   * Add a couple of 90 degree turns to get the top and right faces in their
   * logical positions when the program starts up.
   */
  glRotatef (90.0, 1, 0, 0);
  glRotatef (-90.0, 0, 0, 1);

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
accPerspective (GLdouble fovy, GLdouble aspect,
		GLdouble zNear, GLdouble zFar, GLdouble pixdx, GLdouble pixdy,
		GLdouble eyedx, GLdouble eyedy, GLdouble focus)
{
  GLdouble fov2, left, right, top, bottom;

  fov2 = (fovy * M_PI / 180.0) / 2.0;
  top = zNear / (cos (fov2) / sin (fov2));
  bottom = -top;
  right = top * aspect;
  left = -right;
  accFrustum (left, right, bottom, top, zNear, zFar, pixdx, pixdy, eyedx,
	      eyedy, focus);

}


void
projection_init (int jitter)
{
  bounding_sphere_radius = 2 * cube_dimension;

  fovy = 33.0;
  cp_near = (GLdouble) bounding_sphere_radius / (tan (fovy * M_PI / 360.0));
  cp_far = cp_near + 2 * bounding_sphere_radius;

  glEnable (GL_DEPTH_TEST);
  glClearColor (0, 0, 0, 0);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  accPerspective (fovy, 1, cp_near, cp_far,
		  j8[jitter].x, j8[jitter].y, 0.0, 0.0, 1.0);

}


void
lighting_init (void)
{
  GLfloat light_ambient[] = { 0.6, 0.6, 0.6, 1.0 };
  GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
  GLfloat mat_specular[] = { 0.2, 0.2, 0.2, 1.0 };
  GLfloat mat_shininess = 1.0;
  GLfloat light_position[] = { -1.0, 0.5, 0.0, 0.0 };

  glShadeModel (GL_SMOOTH);

  glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

  glMaterialfv (GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv (GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialf (GL_FRONT, GL_SHININESS, mat_shininess);

  glLightfv (GL_LIGHT0, GL_POSITION, light_position);
  glLightfv (GL_LIGHT1, GL_AMBIENT, light_ambient);


  glDisable (GL_LIGHTING);

  glEnable (GL_COLOR_MATERIAL);

  glEnable (GL_LIGHT0);
  glEnable (GL_LIGHT1);
}
