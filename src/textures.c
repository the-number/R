/*
    Texture mapping functions for the Cube
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
#include <GL/gl.h>

#include <stdlib.h>

#include <assert.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "textures.h"

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)


static GLubyte Image[6][checkImageHeight][checkImageWidth][4];
static unsigned char xbm[6][checkImageHeight*checkImageWidth/8];

struct pattern_parameters stock_pattern[6];

static void
texMakePatterns (void)
{
   int i,  j,  c;
   int k=0;
   int x=0;
   int shift=0;

   /* Stripy pattern */
   for (i = 0; i < checkImageHeight; i++) {
     for (j = 0; j < checkImageWidth; j++) {
       c = ((i&0x04)==0) *255;
       Image[k][i][j][0] = (GLubyte) c;
       Image[k][i][j][1] = (GLubyte) c;
       Image[k][i][j][2] = (GLubyte) c;
       Image[k][i][j][3] = (GLubyte) 255;
       xbm[k][x] |= ( c&0x01 ) << shift;
       shift ++;
       if (shift >= 8 ) {
	 shift=0;
	 x++;
       }
     }
   }


   /* Diagonal Striped */
   k=1;
   shift=0;
   x=0;
   for (i = 0; i < checkImageHeight; i++) {
     for (j = 0; j < checkImageWidth; j++) {
       c = (((i+j)&0x08)==0) *255;
       Image[k][i][j][0] = (GLubyte) c;
       Image[k][i][j][1] = (GLubyte) c;
       Image[k][i][j][2] = (GLubyte) c;
       Image[k][i][j][3] = (GLubyte) 255;
       xbm[k][x] |= ( c&0x01 ) << shift;
       shift ++;
       if (shift >= 8 ) {
	 shift=0;
	 x++;
       }
     }
   }


   /* Checked patterns */
   for ( k = 2 ; k < 6 ; ++k ) {
     unsigned int foo = 0x01 << k ;
     shift=0;
     x = 0;
     for (i = 0; i < checkImageHeight; i++) {
       for (j = 0; j < checkImageWidth; j++) {
	 c = ((((i&foo)==0)^((j&foo)==0)))*255;
	 Image[k][i][j][0] = (GLubyte) c;
	 Image[k][i][j][1] = (GLubyte) c;
	 Image[k][i][j][2] = (GLubyte) c;
	 Image[k][i][j][3] = (GLubyte) 255;
	 xbm[k][x] |= ( c&0x01 ) << shift;
	 shift ++;
	 if (shift >= 8 ) {
	   shift=0;
	   x++;
	 }
       }
     }
   }



}



void
texInit (void)
{
  int i;
  GLuint texName[6];

  texMakePatterns ();
  glPixelStorei (GL_UNPACK_ALIGNMENT,  1);

  glGenTextures (6,  texName);

  glTexEnvi (GL_TEXTURE_ENV,  GL_TEXTURE_ENV_MODE,  GL_MODULATE);
  for ( i = 0 ; i < 6 ; ++i ) {

    stock_pattern[i].texName = texName[i];

    glBindTexture (GL_TEXTURE_2D,  texName[i]);
    glTexParameteri (GL_TEXTURE_2D,  GL_TEXTURE_WRAP_S,  GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D,  GL_TEXTURE_WRAP_T,  GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D,  GL_TEXTURE_MAG_FILTER,
		    GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D,  GL_TEXTURE_MIN_FILTER,
		    GL_NEAREST);

    glTexImage2D (GL_TEXTURE_2D,  0,  GL_RGBA,  checkImageWidth,
		 checkImageHeight,  0,  GL_RGBA,  GL_UNSIGNED_BYTE,
		 Image[i]);

    stock_pattern[i].data = xbm[i];
    stock_pattern[i].texFunc = GL_MODULATE;
  }


}

/* Create a texture from a gdk_pixbuf.
   Returns NULL if it cannot be created.
*/
GLuint
create_pattern_from_pixbuf (const GdkPixbuf *pixbuf ,  GError **gerr)
{

  GLuint texName;

  glPixelStorei (GL_UNPACK_ALIGNMENT,  1);
  glGenTextures (1,  &texName);


  glBindTexture (GL_TEXTURE_2D,  texName);
  glTexParameteri (GL_TEXTURE_2D,  GL_TEXTURE_WRAP_S,  GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D,  GL_TEXTURE_WRAP_T,  GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D,  GL_TEXTURE_MAG_FILTER,
		  GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D,  GL_TEXTURE_MIN_FILTER,
		  GL_NEAREST);

  {
    int width;
    int height;
    guchar * pixels;

    gboolean has_alpha;
    int channels;

    GLenum format;
    GdkColorspace colourSpace ;

    GQuark domain = g_quark_from_string ("rubik_texture");

    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);
    colourSpace =  gdk_pixbuf_get_colorspace (pixbuf);


    channels = gdk_pixbuf_get_n_channels (pixbuf);
    has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);

    /* This seems to cover all the cases that gdk_pixbuf
       supports at the moment */
    switch ( colourSpace ) {
    case GDK_COLORSPACE_RGB:
      if ( channels == 4 && has_alpha ) {
	format = GL_RGBA;
      }
      else if ( channels == 3 && ! has_alpha ) {
	format = GL_RGB;
      }
      else {
	if ( gerr ) {
	  *gerr = g_error_new (domain, 0,
		      _("Pixbuf has wrong number of channels"));
	}
	return 0;
      }
      break ;
    default:
      if ( gerr ) {
	*gerr = g_error_new (domain, 1,
			    _("Pixbuf has unknown colorspace: %d"),
			    colourSpace);
      }

      return 0;
    }

    pixels = gdk_pixbuf_get_pixels (pixbuf);

    glTexImage2D (GL_TEXTURE_2D, 0, 3,  width,  height, 0,
		 format,  GL_UNSIGNED_BYTE,  pixels);
  }

  return texName;

}



