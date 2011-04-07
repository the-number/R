/*
    Texture mapping functions for the Cube
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
#include <GL/gl.h>

#include <stdlib.h>

#include <assert.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "textures.h"

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)

/* Create a texture from a gdk_pixbuf.
   Returns NULL if it cannot be created.
*/
GLuint
create_pattern_from_pixbuf (const GdkPixbuf * pixbuf, GError ** gerr)
{
  GLuint texName;

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glGenTextures (1, &texName);

  glBindTexture (GL_TEXTURE_2D, texName);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  {
    guchar *pixels;
    GLenum format;

    GQuark domain = g_quark_from_string ("gnubik-texture");

    int width = gdk_pixbuf_get_width (pixbuf);
    int height = gdk_pixbuf_get_height (pixbuf);
    GdkColorspace colourSpace = gdk_pixbuf_get_colorspace (pixbuf);

    int channels = gdk_pixbuf_get_n_channels (pixbuf);
    gboolean has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);

    /* This seems to cover all the cases that gdk_pixbuf
       supports at the moment */
    switch (colourSpace)
      {
      case GDK_COLORSPACE_RGB:
	if (channels == 4 && has_alpha)
	  {
	    format = GL_RGBA;
	  }
	else if (channels == 3 && !has_alpha)
	  {
	    format = GL_RGB;
	  }
	else
	  {
	    if (gerr)
	      {
		*gerr = g_error_new (domain, 0,
				     _
				     ("Pixbuf has wrong number of channels"));
	      }
	    return 0;
	  }
	break;
      default:
	if (gerr)
	  {
	    *gerr = g_error_new (domain, 1,
				 _("Pixbuf has unknown colorspace: %d"),
				 colourSpace);
	  }

	return 0;
      }

    pixels = gdk_pixbuf_get_pixels (pixbuf);

    glTexImage2D (GL_TEXTURE_2D, 0, 3, width, height, 0,
		  format, GL_UNSIGNED_BYTE, pixels);
  }

  return texName;
}
