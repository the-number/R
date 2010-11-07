/*
    The routines which actually draw the blocks of the cube.
    Copyright (C) 1998,  2003 John Darrington

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
	glLoadName is a Mesa/OpenGL command,  which loads a `name' so that the
	selection mechanism can identify  an object .
*/


#include <GL/gl.h>
#include "drwBlock.h"
#include <stdio.h>
#include "cube.h"
#include "textures.h"
#include "colour-sel.h"

#include <math.h>
#include "gnubik.h"


/* We use a little bit of glut in debug mode */
#if DEBUG && HAVE_GL_GLUT_H
#include <GL/glut.h>

void
renderString (const char *string)
{
  int i=0;
  for (i = 0 ; i < strlen (string) ; ++i ) {
        glutStrokeCharacter (GLUT_STROKE_MONO_ROMAN,  string[i]);
  }

}

#endif



typedef enum {
	COL_RED=0,  COL_GREEN,  COL_BLUE,
	COL_CYAN,  COL_MAGENTA,  COL_YELLOW,
	COL_BLACK,  COL_WHITE
} colour_type;

static GLfloat colors[][3] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},  {0.0, 0.0, 1.0},
	{0.0, 1.0, 1.0},  {1.0, 0.0, 1.0},  {1.0, 1.0, 0.0},
	{0.0, 0.0, 0.0},  /*Black */
	{1.0, 1.0, 1.0}  /*White */
};


void
draw_face (GLint face,  GLint col_outline ,  int block_id);

/* this macro produces +1 if i is even. -1 if i is odd */
/* We use it to transform the faces of the block from zero,  to the
appropriate place */
#define SHIFT(i) ((i%2) * 2 -1)

/*  render the block pointed to by attrib.
if highlight is true,  then the outline is white,
otherwise it is black */
void
draw_block (GLboolean highlight,  int block_id)
{

	int i;

	/* Load the name of this block */
	glLoadName (block_id);


	/* Start a new name level ( for the face of the block) */
	glPushName (-1);

	/* Rasterise only the exterior faces,  to speed things up */
	glEnable (GL_CULL_FACE);

	for ( i =0 ; i < 6 ; i++ ) {
		int mask;
		glPushMatrix ();
		switch (i)
		{
		case 1:
		case 0:
			glTranslated (0, 0,  SHIFT (i));
			break;
		case 2:
		case 3:
			glTranslated (0,  SHIFT (i), 0);
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
		if ( !(i % 2) ) {
			glRotatef (180, 1, 0, 0);

		}

		/* draw the face,  iff it is visible */
		mask = 0x01 << i ;
		if ( get_visible_faces (the_cube,  block_id) & mask ) {
			glLoadName (i);
			draw_face (i,
				(highlight?COL_WHITE:COL_BLACK),
				block_id);
			/*
		{
		  point p;
		  get_face_centre (block_id,  i,  p);
		}
			*/


		}

		glPopMatrix ();
	}
	glPopName ();


	/*
	showBlock (block_id);
	*/

} /* end block */



/* render the face,  with a specified fill,  and outline colour */
void
draw_face (GLint face,  GLint col_outline ,  int block_id)
{

  point p1;
  point p2;
  vector v;


  point centre_point;


  /* lratio is the proportion of a face's linear dimension,  which is
     coloured. That is,  covered by a sticky label */
  const GLfloat lratio = 0.9;
	
  /* the dead zone is the space on the square,  which pointing to with
     the mouse will not change the selection.  This gives a bit of
     histeresis,  and makes it easier to use. */
  const GLfloat deadZone = 0.02;
  const GLfloat limit1 = (1-deadZone);
  const GLfloat limit2 = (1-2*deadZone);
	
  centre_point[3]=1.0;





  /* First Draw the surface of the cube,  that is the plastic material
     he thing is constructed from */
  glColor3fv (colors[col_outline]);


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


  p1[0]=0;   p1[1]=0;  p1[2]=0;  p1[3]=1;
  p2[0]=0;   p2[1]=0;  p2[2]=1;  p2[3]=1;

  vector_from_points (p2,  p1,  v);

  set_normal_vector (the_cube,  block_id,  face,  v);

  glPushName (0);

  p1[0]=-deadZone;   p1[1]=0;  p1[2]=0;  p1[3]=1;
  p2[0]=-limit1;   p2[1]=0;  p2[2]=0;  p2[3]=1;


  glBegin (GL_POLYGON);
   glVertex3fv (p1);
   glVertex3d (-limit1,  limit2, 0);
   glVertex3d (-limit1, -limit2, 0);
  glEnd ();



  vector_from_points (p2,  p1,  v);
  set_quadrant_vector (the_cube,  block_id,  face, 0,  v);



  glLoadName (1);

  p1[0]=0; p1[1]=-deadZone;   p1[2]=0;
  p2[0]=0;   p2[1]=limit1;  p2[2]=0;

  glBegin (GL_POLYGON);
   glVertex3fv (p1);
   glVertex3d (limit2,  limit1, 0);
   glVertex3d (-limit2,  limit1, 0);
  glEnd ();

  vector_from_points (p2,  p1,  v);
  set_quadrant_vector (the_cube,  block_id,  face, 1,  v);



  glLoadName (2);

  p1[0]=deadZone;   p1[1]=0;           p1[2]=0;
  p2[0]=limit1;          p2[1]=0;      p2[2]=0;

  glBegin (GL_POLYGON);
   glVertex3fv (p1);
   glVertex3d (limit1, -limit2, 0);
   glVertex3d (limit1,  limit2, 0);
  glEnd ();

  vector_from_points (p2,  p1,  v);
  set_quadrant_vector (the_cube,  block_id,  face, 2,  v);

  glLoadName (3);


  p1[0] = 0 ; p1[1]=deadZone;          p1[2]=0;
  p2[0] = 0;  p2[1]=-limit1;            p2[2]=0;

  glBegin (GL_POLYGON);
  {
   glVertex3d (0, -deadZone, 0);
   glVertex3fv (p1);
   glVertex3d (-limit2, -limit1, 0);
   glVertex3d (limit2, -limit1, 0);
  }
  glEnd ();
  glPopName ();

  vector_from_points (p2,  p1,  v);
  set_quadrant_vector (the_cube,  block_id,  face, 3,  v);


  if ( !(face % 2) ) {
    glRotatef (180, 0, 0, 1);
  }

  /* Now do the colours  (ie the little sticky labels)*/
  glColor3fv (colors[face]);
  glTranslatef (0, 0, 0.01);


  glScalef (lratio,  lratio,  lratio);


  if ( -1 == rendering[face]->texName ) {
    glDisable (GL_TEXTURE_2D);
  }
  else {
    glEnable (GL_TEXTURE_2D);
    if ( rendering[face]->type == IMAGED )
      glTexEnvi (GL_TEXTURE_ENV,  GL_TEXTURE_ENV_MODE,  GL_DECAL);
    else
      glTexEnvi (GL_TEXTURE_ENV,  GL_TEXTURE_ENV_MODE,  GL_MODULATE);

    glBindTexture (GL_TEXTURE_2D,  rendering[face]->texName);
  }



  glBegin (GL_POLYGON);
  {
    GLfloat image_segment_size ;
    GLint xpos = 0;
    GLint ypos = 0;

    if ( rendering[face]->type == IMAGED &&
	 rendering[face]->distr == MOSAIC ) {
      image_segment_size = 1.0 / cube_dimension;
      switch ( face) {
      case 0:
      case 1:
	xpos=block_id % cube_dimension;
	ypos=block_id % (cube_dimension * cube_dimension) / cube_dimension;
	break ;
      case 2:
      case 3:
	xpos=block_id % cube_dimension ;
	ypos=block_id / (cube_dimension * cube_dimension );
	break ;
      case 4:
      case 5:
	xpos=block_id / (cube_dimension * cube_dimension );
	ypos=block_id % ( cube_dimension * cube_dimension ) / cube_dimension ;
	break ;
      }

      /* Invert positions as necessary */
      switch ( face ) {
      case 0:
	xpos = cube_dimension - xpos -1;
	/* fallthrough */
      case 1:
	ypos = cube_dimension - ypos -1;
	break ;
      case 5:
	xpos = cube_dimension - xpos -1;
	/* fallthrough */
      case 4:
	ypos = cube_dimension - ypos -1;
	break ;
      case 2:
	xpos = cube_dimension - xpos -1;
	break;
      }
    }
    else { /* TILED */
      xpos=ypos=0;
      image_segment_size = 1.0;
    }

     glTexCoord2f (image_segment_size*xpos,  image_segment_size*(ypos+1));
     glVertex3d (-1, -1, 0);	
			
     glTexCoord2f (image_segment_size*(xpos+1),  image_segment_size*(ypos+1));
     glVertex3d (1, -1, 0);	

     glTexCoord2f (image_segment_size*(xpos+1),  image_segment_size*(ypos));
     glVertex3d (1, 1, 0);

     glTexCoord2f (image_segment_size*(xpos),  image_segment_size*(ypos));
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

    glTranslatef (-1,  -0.8, 0.1);
    glScalef (0.01, 0.01, 0.01);

    snprintf (str, 4, "%d",  block_id);
    renderString (str);
    glPopMatrix ();

    /* render the face number,  a little bit smaller
       so we can see what's what. */
    glPushMatrix ();
    glTranslatef (+0.5,  +0.4, 0.1);
    glScalef (0.005, 0.005, 0.005);
    snprintf (str, 4, "%d",  face);
    renderString (str);
    glPopMatrix ();
  }
#endif


}

void
setColour (int i,
	  GLfloat red,
	  GLfloat green,
	  GLfloat blue)


{

  colors[i][0]=red;
  colors[i][1]=green;
  colors[i][2]=blue;

}


void
getColour (int i,
	  GLfloat *red,
	  GLfloat *green,
	  GLfloat *blue)


{

  *red =   colors[i][0];
  *green = colors[i][1];
  *blue =  colors[i][2];
}




