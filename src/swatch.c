/*
  Copyright (c) 2011        John Darrington

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

#include "swatch.h"
#include <gtk/gtk.h>

#include "colour-dialog.h"

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)


static GtkWidgetClass *parent_class = NULL;


enum
{
  PROP_0 = 0,
  PROP_COLOR,
  PROP_TEXTURE,
  PROP_SURFACE
};



static void
swatch_set_property (GObject * object,
		     guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GbkSwatch *sw = GBK_SWATCH (object);

  switch (prop_id)
    {
    case PROP_COLOR:
      {
	GdkColor *new_color = g_value_get_boxed (value);

	if (new_color)
	  {
	    if (sw->color)
	      gdk_color_free (sw->color);

	    sw->color = gdk_color_copy (new_color);

	    if (gtk_widget_get_realized (GTK_WIDGET (sw)))
	      {
		gdk_gc_set_rgb_fg_color (sw->gc, sw->color);
		gtk_widget_queue_draw (GTK_WIDGET (sw));
	      }
	  }
	else
	  g_warning ("Invalid color property for swatch");
      }
      break;
    case PROP_TEXTURE:
      sw->pixbuf = g_value_get_object (value);

      if (sw->pixbuf)
	{
	  if (sw->stype == SURFACE_COLOURED)
	    sw->stype = SURFACE_TILED;
	}
      else
	sw->stype = SURFACE_COLOURED;

      gtk_widget_queue_draw (GTK_WIDGET (sw));
      break;
    case PROP_SURFACE:
      sw->stype = g_value_get_enum (value);
      if (sw->stype == SURFACE_COLOURED)
	sw->pixbuf = NULL;

      gtk_widget_queue_draw (GTK_WIDGET (sw));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    };
}


static void
swatch_get_property (GObject * object,
		     guint prop_id, GValue * value, GParamSpec * pspec)
{
  GbkSwatch *sw = GBK_SWATCH (object);

  switch (prop_id)
    {
    case PROP_COLOR:
      g_value_set_boxed (value, sw->color);
      break;
    case PROP_TEXTURE:
      g_value_set_object (value, sw->pixbuf);
      break;
    case PROP_SURFACE:
      g_value_set_enum (value, sw->stype);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    };
}


static void
gbk_swatch_init (GbkSwatch * sw)
{
  sw->da = gtk_drawing_area_new ();

  gtk_container_add (GTK_CONTAINER (sw), sw->da);

  gtk_widget_set_tooltip_text (GTK_WIDGET (sw),
			       _
			       ("A sample of the colour. You can click and select a new colour, or drag one to this space."));

  sw->color = NULL;
  sw->pixbuf = NULL;
  sw->stype = SURFACE_COLOURED;
}



static gboolean
on_da_expose (GtkWidget * w, GdkEventExpose * event, gpointer data)
{
  GbkSwatch *sw = GBK_SWATCH (data);

  if (sw->pixbuf == NULL)
    {
      gdk_draw_rectangle (w->window,
			  sw->gc,
			  TRUE,
			  0, 0, w->allocation.width, w->allocation.height);
    }
  else
    {
      GdkPixbuf *scaled_pixbuf = 0;
      gint width, height;

      gdk_drawable_get_size (w->window, &width, &height);

      scaled_pixbuf = gdk_pixbuf_scale_simple (sw->pixbuf,
					       width, height,
					       GDK_INTERP_NEAREST);

      g_assert (scaled_pixbuf);

      gdk_draw_pixbuf (w->window,
		       sw->gc,
		       scaled_pixbuf,
		       0, 0, 0, 0, width, height, GDK_RGB_DITHER_NONE, 0, 0);

      g_object_unref (scaled_pixbuf);
    }

  return FALSE;
}


#define SWATCH_WIDTH 64
#define SWATCH_HEIGHT 64

enum
{
  GBK_DRAG_FILELIST,
  GBK_DRAG_COLOUR
};

static const GtkTargetEntry targets[2] = {
  {"text/uri-list", 0, GBK_DRAG_FILELIST},
  {"application/x-color", 0, GBK_DRAG_COLOUR},
};

static void
on_drag_data_rx (GtkWidget * widget,
		 GdkDragContext * drag_context,
		 gint x,
		 gint y,
		 GtkSelectionData * selection_data,
		 guint info, guint time, gpointer user_data)
{
  gboolean success = TRUE;

  if (selection_data->length < 0)
    {
      g_warning ("Empty drag data");
      success = FALSE;
      goto end;
    }

  switch (info)
    {
    case GBK_DRAG_COLOUR:
      {
	guint16 *vals;
	GdkColor colour;

	if ((selection_data->format != 16) || (selection_data->length != 8))
	  {
	    success = FALSE;
	    g_warning ("Received invalid color data");
	    goto end;
	  }

	vals = (guint16 *) selection_data->data;

	colour.red = vals[0];
	colour.green = vals[1];
	colour.blue = vals[2];

	g_object_set (widget, "color", &colour, NULL);
	break;
      }
    case GBK_DRAG_FILELIST:
      {
	gchar **s = 0;
	gchar **start = 0;

	start = s =
	  g_strsplit ((const gchar *) selection_data->data, "\r\n", 0);

	while (*s)
	  {
	    GdkPixbuf *pixbuf = NULL;
	    gchar *utf8;
	    gchar *filename;

	    GError *gerr = 0;

	    if (strcmp (*s, "") == 0)
	      {
		s++;
		continue;
	      }

	    /* Convert to utf8.  Is this necessary ?? */
	    utf8 = g_locale_to_utf8 (*s, -1, 0, 0, &gerr);
	    if (gerr)
	      {
		g_warning (gerr->message);
		g_clear_error (&gerr);
		gerr = 0;
		continue;
	      }

	    /* Extract the filename from the uri */
	    filename = g_filename_from_uri (utf8, 0, &gerr);
	    if (gerr)
	      {
		g_warning (gerr->message);
		g_clear_error (&gerr);
		continue;
	      }
	    g_free (utf8);

	    pixbuf = create_pixbuf_from_file (filename, &gerr);

	    g_free (filename);

	    g_object_set (widget, "texture", pixbuf, NULL);

	    /* For now,  just use the first one.
	       Later,  we'll add some method for disambiguating multiple files
	     */
	    break;
	    s++;
	  }
      }
      break;
    default:
      g_warning ("Unsupported drag data type");
      break;
    }

end:
  gtk_drag_finish (drag_context, success, FALSE, time);
}

static void
realize (GtkWidget * w)
{
  GbkSwatch *sw = GBK_SWATCH (w);

  if (GTK_WIDGET_CLASS (parent_class)->realize)
    GTK_WIDGET_CLASS (parent_class)->realize (w);

  gtk_widget_set_size_request (GTK_WIDGET (sw), SWATCH_WIDTH, SWATCH_HEIGHT);

  GdkWindow *pw = gtk_widget_get_parent_window (sw->da);
  sw->gc = gdk_gc_new (pw);

  if (sw->color)
    gdk_gc_set_rgb_fg_color (sw->gc, sw->color);

  gtk_drag_dest_set (w, GTK_DEST_DEFAULT_ALL, targets, 2, GDK_ACTION_COPY);

  g_signal_connect (sw->da, "expose-event", G_CALLBACK (on_da_expose), sw);
  g_signal_connect (sw, "drag-data-received", G_CALLBACK (on_drag_data_rx),
		    0);
}


static void
gbk_swatch_class_init (GbkSwatchClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GParamSpec *color_param_spec;
  GParamSpec *texture_param_spec;
  GParamSpec *surface_param_spec;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->set_property = swatch_set_property;
  gobject_class->get_property = swatch_get_property;

  widget_class->realize = realize;

  color_param_spec = g_param_spec_boxed ("color",
					 "Colour",
					 "The colour of the swatch",
					 GDK_TYPE_COLOR, G_PARAM_READWRITE);

  texture_param_spec = g_param_spec_object ("texture",
					    "Texture",
					    "A pixbuf representing the texture of the swatch",
					    GDK_TYPE_PIXBUF,
					    G_PARAM_READWRITE);

  surface_param_spec = g_param_spec_enum ("surface",
					  "Surface",
					  "Tiled or Mosaic",
					  GBK_TYPE_SURFACE,
					  SURFACE_COLOURED,
					  G_PARAM_READWRITE);


  g_object_class_install_property (gobject_class,
				   PROP_COLOR, color_param_spec);

  g_object_class_install_property (gobject_class,
				   PROP_TEXTURE, texture_param_spec);

  g_object_class_install_property (gobject_class,
				   PROP_SURFACE, surface_param_spec);
}

G_DEFINE_TYPE (GbkSwatch, gbk_swatch, GTK_TYPE_TOGGLE_BUTTON);

GtkWidget *
gbk_swatch_new (void)
{
  return g_object_new (gbk_swatch_get_type (), NULL);
}
