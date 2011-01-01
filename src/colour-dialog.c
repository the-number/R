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
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <string.h>
#include "glarea.h"
#include "colour-dialog.h"
#include "drwBlock.h"
#include "textures.h"
#include "widget-set.h"
#include "select.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

#define BOX_PADDING 10

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)


struct swatch 
{
  int id;
  GtkWidget *da;
  GtkWidget *button;
  GdkGC *gc;

  GLfloat red;
  GLfloat green;
  GLfloat blue;
};


/* This function sets the GC foreground colour from the
   Mesa/OpenGL colour values provided */
static void
set_gc_foreground (struct swatch *sw)
{
  GdkColor fg;

  g_assert (sw->gc);

  fg.red = sw->red * 65535.0;
  fg.green = sw->green * 65535.0;
  fg.blue = sw->blue * 65535.0;

  gdk_gc_set_rgb_fg_color (sw->gc, &fg);
}

/* Set the swatch colours from the Graphics Context */
static void
get_gc_foreground (struct swatch *sw)
{
  GdkColor colour;
  g_assert (sw->gc);

  GdkGCValues gcv;
  gdk_gc_get_values (sw->gc, &gcv);

  GdkColormap *cmap = gtk_widget_get_colormap (sw->da);
  
  gdk_colormap_query_color (cmap, gcv.foreground.pixel, &colour);

  sw->red = colour.red / 65535.0;
  sw->green = colour.green / 65535.0;
  sw->blue = colour.blue / 65535.0;
}

struct colour_dialog_state
{
  int selected_swatch;
  struct swatch *swatches[6];
  GtkWidget *colour_selector;
};

static void
set_swatch_colour (GtkColorSelection *cs, gpointer data)
{
  GdkColor c;

  struct colour_dialog_state *cds = data;

  struct swatch *sw = cds->swatches[cds->selected_swatch];

  GdkColormap *cmap = gtk_widget_get_colormap (sw->da);

  gtk_color_selection_get_current_color (cs, &c);

  gdk_rgb_find_color (cmap, &c);

  gdk_gc_set_foreground  (sw->gc, &c);

  get_gc_foreground (sw);

  gtk_widget_queue_draw (sw->da);
}




static gboolean draw_swatch (GtkWidget *widget, GdkEventExpose *event, gpointer data);

/* 
   Called when a swatch button is clicked.
 */
static void
select_swatch (GtkWidget *w, gpointer data)
{
  int i;
  struct colour_dialog_state *csd = data;
  for ( i = 0; i < 6 ; ++i)
    {
      if ( w == csd->swatches[i]->button)
	{
	  csd->selected_swatch = i;
	  break;
	}
    }

  GdkColor colour;

  GdkColormap *cmap = gtk_widget_get_colormap (csd->swatches[i]->da);

  GdkGCValues gcv;
  gdk_gc_get_values (csd->swatches[i]->gc, &gcv);

  gdk_colormap_query_color (cmap, gcv.foreground.pixel, &colour);
  
  gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (csd->colour_selector), 
			       &colour);
}


#define SWATCH_WIDTH  64
#define SWATCH_HEIGHT 64


static void 
on_swatch_realise (GtkWidget *w, gpointer data)
{
  struct swatch *sw = data;

  GdkWindow *pw = gtk_widget_get_parent_window (sw->da);
  sw->gc = gdk_gc_new (pw);

  getColour (sw->id, &sw->red, &sw->green, &sw->blue);
  set_gc_foreground (sw);

  gtk_widget_set_size_request (sw->da, SWATCH_WIDTH, SWATCH_HEIGHT);
}

static struct swatch *
swatch_create (int i)
{
  struct swatch *sw = g_malloc (sizeof (*sw));

  sw->button = gtk_button_new ();

  gtk_widget_set_tooltip_text (sw->button,
				   _("A sample of the colour. You can click and select a new colour, or drag one to this space."));

  sw->da = gtk_drawing_area_new ();
  sw->id = i;
  sw->gc = NULL;

  gtk_container_add (GTK_CONTAINER (sw->button), sw->da);

  g_signal_connect (sw->da, "realize", G_CALLBACK (on_swatch_realise), sw);

  g_signal_connect (sw->da, "expose-event", G_CALLBACK (draw_swatch), sw);

  return sw;
}

void
colour_select_menu (GtkWidget *w, GtkWindow *window)
{
  int i;

  GtkWidget *table;

  struct colour_dialog_state *cds = g_malloc (sizeof *cds);

  cds->colour_selector = gtk_color_selection_new ();

  GtkWidget *dialog = gtk_dialog_new_with_buttons (_("Colour selector"),
						   window,
						   GTK_DIALOG_MODAL
						   | GTK_DIALOG_DESTROY_WITH_PARENT,
						   GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
						   GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
						   NULL);

  GtkWidget *hbox_swatch = gtk_hbox_new (FALSE, BOX_PADDING);
  GtkWidget *hbox_cs = gtk_hbox_new (FALSE, BOX_PADDING);

  GtkWidget *image_select_vbox = gtk_vbox_new (FALSE, BOX_PADDING);
  GtkWidget *image_select_hbox = gtk_hbox_new (FALSE, BOX_PADDING);

  GtkWidget *frame = gtk_frame_new (_("Image"));
  GtkWidget *button_use_image = gtk_button_new_with_mnemonic (_("Se_lect"));

  
  cds->selected_swatch = 0;

  gtk_color_selection_set_has_opacity_control (GTK_COLOR_SELECTION (cds->colour_selector), FALSE);

  gtk_window_set_icon_name (GTK_WINDOW(dialog), "gnubik");

  gtk_widget_set_tooltip_text (button_use_image, _("Select an image file"));

  GtkWidget *button_tile = gtk_radio_button_new_with_mnemonic (0, _("_Tiled"));
  gtk_widget_set_tooltip_text (button_tile,
			       _("Place a copy of the image on each block"));

  GtkWidget *button_mosaic =
    gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON
						    (button_tile),
						    _("_Mosaic"));

  gtk_widget_set_tooltip_text (button_mosaic,
			       _("Place a copy of the image across the entire face of the cube"));


  GSList *button_group =
    gtk_radio_button_get_group (GTK_RADIO_BUTTON (button_mosaic));

  gtk_widget_set_sensitive (button_tile, FALSE);
  gtk_widget_set_sensitive (button_mosaic, FALSE);

  gtk_box_pack_start (GTK_BOX (image_select_vbox), button_use_image,
		      TRUE, TRUE, BOX_PADDING);
  gtk_box_pack_start (GTK_BOX (image_select_vbox), button_tile,
		      TRUE, TRUE, BOX_PADDING);
  gtk_box_pack_start (GTK_BOX (image_select_vbox), button_mosaic,
		      TRUE, TRUE, BOX_PADDING);

  gtk_box_pack_start (GTK_BOX (image_select_hbox), image_select_vbox,
		      TRUE, TRUE, BOX_PADDING);

  gtk_container_add (GTK_CONTAINER (frame), image_select_hbox);

  table = gtk_table_new (2, 3, TRUE);

  for (i = 0; i < 6; ++i)
    {
      cds->swatches[i] = swatch_create (i);

#if 0
      const GtkTargetEntry target[2] = {
	{"text/uri-list", 0, RDRAG_FILELIST},
	{"application/x-color", 0, RDRAG_COLOUR},
      };
#endif

      g_signal_connect (cds->swatches[i]->button, "clicked",
			G_CALLBACK (select_swatch), cds);

      gtk_table_attach (GTK_TABLE (table), cds->swatches[i]->button,
			i / 2, i / 2 + 1,
			i % 2, i % 2 + 1,
			GTK_EXPAND, GTK_EXPAND, 0, 0);
    }

  g_signal_connect (cds->colour_selector, "color-changed",
		    G_CALLBACK (set_swatch_colour), cds);

  gtk_container_add (GTK_CONTAINER (hbox_swatch), table);

  gtk_box_pack_start (GTK_BOX (hbox_swatch), frame, TRUE, TRUE, BOX_PADDING);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), hbox_swatch);

  gtk_box_pack_start (GTK_BOX (hbox_cs), cds->colour_selector,
		      TRUE, TRUE, BOX_PADDING);
  
  gtk_widget_show_all (hbox_cs);
  gtk_widget_show_all (hbox_swatch);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), hbox_cs);

  gint resp = gtk_dialog_run (GTK_DIALOG (dialog));

  if ( resp == GTK_RESPONSE_ACCEPT)
    {
      for (i = 0; i < 6 ; ++i)
	{
	  struct swatch *s = cds->swatches[i];
	  setColour (i, s->red, s->green, s->blue);
	}
    }


  

  gtk_widget_destroy (dialog);

  for (i = 0; i < 6; ++i)
    g_free (cds->swatches[i]);

  g_free (cds);
}
/* end of colour_select_menu (..... */


static gboolean
draw_swatch (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  struct swatch *sw = data;

  gdk_draw_rectangle (widget->window,
			  sw->gc,
			  TRUE, 0, 0,
			  widget->allocation.width,
			  widget->allocation.height);

  return FALSE;
}
