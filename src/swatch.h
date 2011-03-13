/*
    Copyright (c) 2011   John Darrington

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


#ifndef __GBK_SWATCH_H__
#define __GBK_SWATCH_H__


#include <glib-object.h>
#include <gtk/gtktogglebutton.h>

#include "drwBlock.h"

#define GBK_TYPE_SWATCH                  (gbk_swatch_get_type ())
#define GBK_SWATCH(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GBK_TYPE_SWATCH, GbkSwatch))
#define GBK_IS_SWATCH(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GBK_TYPE_SWATCH))
#define GBK_SWATCH_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GBK_TYPE_SWATCH, GbkSwatchClass))
#define GBK_IS_SWATCH_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GBK_TYPE_SWATCH))
#define GBK_SWATCH_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GBK_TYPE_SWATCH, GbkSwatchClass))

typedef struct _GbkSwatch GbkSwatch;
typedef struct _GbkSwatchClass GbkSwatchClass;


struct _GbkSwatch
{
  GtkToggleButton parent_instance;

  GtkWidget *da;

  GdkGC *gc;

  GdkColor *color;
  GdkPixbuf *pixbuf;

  enum surface stype;
};

struct _GbkSwatchClass
{
  GtkToggleButtonClass parent_class;

  /* class members */
};

/* used by GBK_TYPE_SWATCH */
GType gbk_swatch_get_type (void);

/*
 * Method definitions.
 */

GtkWidget *gbk_swatch_new (void);



#endif /* __SWATCH_H__ */
