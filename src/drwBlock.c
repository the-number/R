/*
  The routines which actually draw the blocks of the cube.
  Copyright (C) 1998, 2003, 2011 John Darrington

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

/*
  NB: glLoadName is a Mesa/OpenGL command,  which loads a `name' so
  that the selection mechanism can identify  an object .
*/

#include <GL/gl.h>
#include "drwBlock.h"
#include <stdio.h>
#include "cube.h"
#include "textures.h"
#include "colour-dialog.h"

/* We use a little bit of glut in debug mode */
#if DEBUG && HAVE_GL_GLUT_H
#include <GL/glut.h>

void
renderString (const char *string)
{
  int i = 0;
  for (i = 0; i < strlen (string); ++i)
    {
      glutStrokeCharacter (GLUT_STROKE_MONO_ROMAN, string[i]);
    }
}

#endif


typedef enum
{
  COL_BLACK,
  COL_WHITE
} colour_type;

static const GLfloat colors[][3] = {
  {0.0, 0.0, 0.0},		/*Black */
  {1.0, 1.0, 1.0}		/*White */
};



static void draw_face (GbkCubeview *, GLint face, int block_id,
		       GLboolean draw_names);

/* this macro produces +1 if i is even. -1 if i is odd */
/* We use it to transform the faces of the block from zero,  to the
   appropriate place */
#define SHIFT(i) ((i%2) * 2 -1)

/*  Render the block pointed to by BLOCK_ID.
    If ANCILLIARY is true, render the ancialliary components also.
 */
static void
draw_block (GbkCubeview * cv, int block_id, GLboolean ancilliary)
{
  int i;

  /* Load the name of this block */
  glLoadName (block_id);

  /* Start a new name level ( for the face of the block) */
  glPushName (-1);

  /* Rasterise only the exterior faces,  to speed things up */
  glEnable (GL_CULL_FACE);

  for (i = 0; i < 6; i++)
    {
      int mask;
      glPushMatrix ();
      switch (i)
	{
	case 1:
	case 0:
	  glTranslated (0, 0, SHIFT (i));
	  break;
	case 2:
	case 3:
	  glTranslated (0, SHIFT (i), 0);
	  glRotatef (-90, 1, 0, 0);
	  break;
	case 4:
	case 5:
	  glTranslated (SHIFT (i), 0, 0);
	  glRotatef (90, 0, 1, 0);
	  break;
	}

      /* make sure all  the sides are faced with their visible
         surface pointing to the outside!!  */
      if (!(i % 2))
	glRotatef (180, 1, 0, 0);

      /* draw the face,  iff it is visible */
      mask = 0x01 << i;
      if (gbk_cube_get_visible_faces (cv->cube, block_id) & mask)
	{
	  glLoadName (i);
	  draw_face (cv, i, block_id, ancilliary);
	}

      glPopMatrix ();
    }
  glPopName ();

}				/* end block */


/* render FACE of BLOCK_ID.
   If DRAW_NAMES is true, the render the ancillary polygons used for selection.
 */
static void
draw_face (GbkCubeview * cv, GLint face, int block_id, GLboolean draw_names)
{
  point p1;
  point p2;
  vector v;

  /* lratio is the proportion of a face's linear dimension,  which is
     coloured. That is,  covered by a sticky label */
  const GLfloat lratio = 0.9;


  /* First Draw the surface of the cube,  that is the plastic material
     he thing is constructed from */
  glColor3fv (colors[COL_BLACK]);

  if (draw_names)
    {
      /* the dead zone is the space on the square,  which pointing to with
         the mouse will not change the selection.  This gives a bit of
         histeresis,  and makes it easier to use. */
      const GLfloat deadZone = 0.02;
      const GLfloat limit1 = (1 - deadZone);
      const GLfloat limit2 = (1 - 2 * deadZone);

      /* This polygon is drawn as four quadrants,  thus:
         _______
         |\     /|
         | \   / |
         |  \ /  |
         |   \   |
         |  / \  |
         | /   \ |
         |/____ \|

         The reason for this is to provide support for an enhanced selection
         mechanism which can detect which edge of the face is being pointed to.
       */

      p1[0] = 0;
      p1[1] = 0;
      p1[2] = 0;
      p1[3] = 1;
      p2[0] = 0;
      p2[1] = 0;
      p2[2] = 1;
      p2[3] = 1;

      vector_from_points (v, p2, p1);

      glPushName (0);

      p1[0] = -deadZone;
      p1[1] = 0;
      p1[2] = 0;
      p1[3] = 1;
      p2[0] = -limit1;
      p2[1] = 0;
      p2[2] = 0;
      p2[3] = 1;

      glBegin (GL_POLYGON);
      glVertex3fv (p1);
      glVertex3d (-limit1, limit2, 0);
      glVertex3d (-limit1, -limit2, 0);
      glEnd ();

      vector_from_points (v, p2, p1);
      gbk_cube_set_quadrant_vector (cv->cube, block_id, face, 0, v);

      glLoadName (1);

      p1[0] = 0;
      p1[1] = -deadZone;
      p1[2] = 0;
      p2[0] = 0;
      p2[1] = limit1;
      p2[2] = 0;

      glBegin (GL_POLYGON);
      glVertex3fv (p1);
      glVertex3d (limit2, limit1, 0);
      glVertex3d (-limit2, limit1, 0);
      glEnd ();

      vector_from_points (v, p2, p1);
      gbk_cube_set_quadrant_vector (cv->cube, block_id, face, 1, v);

      glLoadName (2);

      p1[0] = deadZone;
      p1[1] = 0;
      p1[2] = 0;
      p2[0] = limit1;
      p2[1] = 0;
      p2[2] = 0;

      glBegin (GL_POLYGON);
      glVertex3fv (p1);
      glVertex3d (limit1, -limit2, 0);
      glVertex3d (limit1, limit2, 0);
      glEnd ();

      vector_from_points (v, p2, p1);
      gbk_cube_set_quadrant_vector (cv->cube, block_id, face, 2, v);

      glLoadName (3);


      p1[0] = 0;
      p1[1] = deadZone;
      p1[2] = 0;
      p2[0] = 0;
      p2[1] = -limit1;
      p2[2] = 0;

      glBegin (GL_POLYGON);
      {
	glVertex3d (0, -deadZone, 0);
	glVertex3fv (p1);
	glVertex3d (-limit2, -limit1, 0);
	glVertex3d (limit2, -limit1, 0);
      }
      glEnd ();
      glPopName ();

      vector_from_points (v, p2, p1);
      gbk_cube_set_quadrant_vector (cv->cube, block_id, face, 3, v);
    }

  /* Now do the colours  (ie the little sticky labels) */
  glColor3fv (cv->colour[face]);
  glTranslatef (0, 0, 0.01);


  glScalef (lratio, lratio, lratio);

  if (-1 == cv->texName[face])
    {
      glDisable (GL_TEXTURE_2D);
    }
  else
    {
      glEnable (GL_TEXTURE_2D);
      if (cv->surface[face] != SURFACE_COLOURED)
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
      else
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

      glBindTexture (GL_TEXTURE_2D, cv->texName[face]);
    }

  glBegin (GL_POLYGON);
  {
    GLfloat iss_x = 0;
    GLfloat iss_y = 0;
    GLint xpos = 0;
    GLint ypos = 0;

    if (cv->surface[face] == SURFACE_MOSAIC)
      {
	switch (face)
	  {
	  case 0:
	  case 1:
	    iss_x = 1.0 / gbk_cube_get_size (cv->cube, 0);
	    iss_y = 1.0 / gbk_cube_get_size (cv->cube, 1);
	    xpos = block_id % gbk_cube_get_size (cv->cube, 0);
	    ypos =
	      (block_id % (gbk_cube_get_size (cv->cube, 0)
			   * gbk_cube_get_size (cv->cube, 1)))
	      / gbk_cube_get_size (cv->cube, 0);
	    break;
	  case 2:
	  case 3:
	    iss_x = 1.0 / gbk_cube_get_size (cv->cube, 0);
	    iss_y = 1.0 / gbk_cube_get_size (cv->cube, 2);
	    xpos = block_id % (gbk_cube_get_size (cv->cube, 0)
			       * gbk_cube_get_size (cv->cube, 1))
	      % gbk_cube_get_size (cv->cube, 0);
	    ypos =
	      block_id / (gbk_cube_get_size (cv->cube, 0) *
			  gbk_cube_get_size (cv->cube, 1));
	    break;
	  case 4:
	  case 5:
	    iss_x = 1.0 / gbk_cube_get_size (cv->cube, 2);
	    iss_y = 1.0 / gbk_cube_get_size (cv->cube, 1);
	    xpos =
	      block_id / (gbk_cube_get_size (cv->cube, 0) *
			  gbk_cube_get_size (cv->cube, 1));
	    ypos =
	      block_id % (gbk_cube_get_size (cv->cube, 0) *
			  gbk_cube_get_size (cv->cube,
					     1)) /
	      gbk_cube_get_size (cv->cube, 0);
	    break;
	  }

	/* Invert positions as necessary */
	switch (face)
	  {
	  case 1:
	    ypos = gbk_cube_get_size (cv->cube, 1) - ypos - 1;
	    break;
	  case 5:
	    ypos = gbk_cube_get_size (cv->cube, 1) - ypos - 1;
	    /* fallthrough */
	  case 4:
	    xpos = gbk_cube_get_size (cv->cube, 2) - xpos - 1;
	    break;
	  case 2:
	    ypos = gbk_cube_get_size (cv->cube, 2) - ypos - 1;
	    break;
	  }
      }
    else
      {				/* TILED */
	xpos = ypos = 0;
	iss_x = iss_y = 1.0;
      }

    glTexCoord2f (iss_x * xpos, iss_y * (ypos + 1));
    glVertex3d (-1, -1, 0);

    glTexCoord2f (iss_x * (xpos + 1), iss_y * (ypos + 1));
    glVertex3d (1, -1, 0);

    glTexCoord2f (iss_x * (xpos + 1), iss_y * ypos);
    glVertex3d (1, 1, 0);

    glTexCoord2f (iss_x * xpos, iss_y * ypos);
    glVertex3d (-1, 1, 0);
  }
  glEnd ();

  glDisable (GL_TEXTURE_2D);

#if DEBUG && HAVE_GL_GLUT_H
  {
    char str[4];

    /* render the block number */
    glPushMatrix ();
    glColor3f (0, 0, 0);

    glTranslatef (-1, -0.8, 0.1);
    glScalef (0.01, 0.01, 0.01);

    snprintf (str, 4, "%d", block_id);
    renderString (str);
    glPopMatrix ();

    /* render the face number,  a little bit smaller
       so we can see what's what. */
    glPushMatrix ();
    glTranslatef (+0.5, +0.4, 0.1);
    glScalef (0.005, 0.005, 0.005);
    snprintf (str, 4, "%d", face);
    renderString (str);
    glPopMatrix ();
  }
#endif
}

/* render the cube */
void
drawCube (GbkCube * cube, GLboolean ancilliary, GbkCubeview * cv)
{
  int i;

#if DEBUG
  {
    GLfloat offset = 1.6 * gbk_cube_get_size (cube, 0);

    /* Show the directions of the axes */
    glColor3f (1, 1, 1);

    /* X axis */
    glPushMatrix ();
    glTranslatef (-offset, -offset, 0);
    glBegin (GL_LINES);
    glVertex3f (0, 0, 0);
    glVertex3f (2 * gbk_cube_get_size (cube, 0), 0, 0);
    glEnd ();


    glRasterPos3d (offset * 1.1, 0, 0);
    glutBitmapCharacter (GLUT_BITMAP_9_BY_15, '0');

    glPopMatrix ();

    /* Y axis */
    glPushMatrix ();
    glTranslatef (-offset, -offset, 0);
    glBegin (GL_LINES);
    glVertex3f (0, 0, 0);
    glVertex3f (0, 2 * gbk_cube_get_size (cube, 1), 0);
    glEnd ();

    glRasterPos3d (0.1 * offset, offset, 0);
    glutBitmapCharacter (GLUT_BITMAP_9_BY_15, '1');

    glPopMatrix ();

    /* Z axis */
    glPushMatrix ();
    glTranslatef (-offset, -offset, 0);

    glBegin (GL_LINES);
    glVertex3f (0, 0, 0);
    glVertex3f (0, 0, 2 * gbk_cube_get_size (cube, 2));
    glEnd ();

    glRasterPos3d (0.1 * offset, 0, offset);
    glutBitmapCharacter (GLUT_BITMAP_9_BY_15, '2');

    glPopMatrix ();

  }
#endif

  for (i = 0; i < gbk_cube_get_number_of_blocks (cube); i++)
    {
      int j = 0;
      Slice_Blocks *moving_blocks = NULL;

      if (cv->current_move && cv->current_move->blocks_in_motion)
	moving_blocks = cv->current_move->blocks_in_motion;

      /* Find out if this block is one of those currently being
         turned.  If so,  j will be < turning_block_qty */
      if (moving_blocks)
	for (j = 0; j < moving_blocks->number_blocks; j++)
	  {
	    if (moving_blocks->blocks[j] == i)
	      break;
	  }

      glPushMatrix ();
      if (moving_blocks && j != moving_blocks->number_blocks)
	{
	  /* Blocks which are in motion,  need to be animated.
	     so we rotate them according to however much the
	     animation angle is */
	  GLdouble angle = cv->animation_angle;

	  int unity = 1;
	  if (!move_dir (cv->current_move))
	    unity = -1;

	  switch (move_axis (cv->current_move))
	    {
	    case 0:
	    case 3:
	      glRotatef (angle, unity, 0, 0);
	      break;
	    case 1:
	    case 4:
	      glRotatef (angle, 0, unity, 0);
	      break;
	    case 2:
	    case 5:
	      glRotatef (angle, 0, 0, unity);
	      break;
	    }
	}
      {
	Matrix M;

	/* place the block in its current position and
	   orientation */
	gbk_cube_get_block_transform (cv->cube, i, M);
	glPushMatrix ();
	glMultMatrixf (M);

	/* and draw the block */
	draw_block (cv, i, ancilliary);
	glPopMatrix ();
      }

      glPopMatrix ();
    }
}
