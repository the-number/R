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
#include "colour-dialog.h"
#include "drwBlock.h"
#include "textures.h"
#include "widget-set.h"
#include "select.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

#define BOX_PADDING 5

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)


#define SWATCH_WIDTH  64
#define SWATCH_HEIGHT 64


static GdkPixbuf *
create_pixbuf_from_file (const gchar *filename, GError **gerr)
{
  GdkPixbuf *pixbuf;

  GdkPixbuf *unscaled_pixbuf = gdk_pixbuf_new_from_file (filename, gerr);

  if (*gerr)
    return 0;

  /* We must scale the image,  because Mesa/OpenGL insists on it being of size
     2^n ( where n is integer ) */

  pixbuf = gdk_pixbuf_scale_simple (unscaled_pixbuf,
				    SWATCH_WIDTH, SWATCH_HEIGHT,
				    GDK_INTERP_NEAREST);

  g_object_unref (unscaled_pixbuf);
  unscaled_pixbuf = 0;

  return pixbuf;
}


struct swatch 
{
  int id;
  GtkWidget *da;
  GtkWidget *button;
  GdkGC *gc;

  struct cube_rendering rendering;
};


/* This function sets the GC foreground colour from the
   Mesa/OpenGL colour values provided */
static void
set_gc_foreground (struct swatch *sw)
{
  GdkColor fg;

  g_assert (sw->gc);

  fg.red = sw->rendering.red * 65535.0;
  fg.green = sw->rendering.green * 65535.0;
  fg.blue = sw->rendering.blue * 65535.0;

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

  sw->rendering.red = colour.red / 65535.0;
  sw->rendering.green = colour.green / 65535.0;
  sw->rendering.blue = colour.blue / 65535.0;
}

struct colour_dialog_state
{
  int selected_swatch;
  struct swatch *swatches[6];
  GtkWidget *colour_selector;

  GSList *button_group;
  GtkWidget *button_plain;
  GtkWidget *button_tile;
  GtkWidget *button_mosaic;
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
select_swatch (GtkToggleButton *w, gpointer data)
{
  GdkGCValues gcv;
  GdkColor colour;

  gboolean active = gtk_toggle_button_get_active (w);
  if ( ! active )
    return;

  int i;
  struct colour_dialog_state *cds = data;
  for ( i = 0; i < 6 ; ++i)
    {
      GtkToggleButton *b = GTK_TOGGLE_BUTTON (cds->swatches[i]->button);
      if ( w == b)
	{
	  cds->selected_swatch = i;
	}
      else
	{
	  gtk_toggle_button_set_active (b, FALSE);
	}
    }

  GdkColormap *cmap = gtk_widget_get_colormap (cds->swatches[cds->selected_swatch]->da);

  gdk_gc_get_values (cds->swatches[cds->selected_swatch]->gc, &gcv);

  gdk_colormap_query_color (cmap, gcv.foreground.pixel, &colour);
  
  gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (cds->colour_selector), 
			       &colour);

  /* Set the sensitivity of the radio buttons */

  GSList *l = cds->button_group;
  while (l)
    {
      gtk_widget_set_sensitive (l->data,
				(cds->swatches[cds->selected_swatch]->rendering.surface != SURFACE_COLOURED));
      l = l->next;
    }

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cds->button_plain), 
				(cds->swatches[cds->selected_swatch]->rendering.surface == SURFACE_COLOURED));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cds->button_tile), 
				(cds->swatches[cds->selected_swatch]->rendering.surface == SURFACE_TILED));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cds->button_mosaic), 
				(cds->swatches[cds->selected_swatch]->rendering.surface == SURFACE_MOSAIC));
  
}


static void 
on_swatch_realise (GtkWidget *w, gpointer data)
{
  struct swatch *sw = data;

  GdkWindow *pw = gtk_widget_get_parent_window (sw->da);
  sw->gc = gdk_gc_new (pw);

  getColour (sw->id, &sw->rendering);
  if ( sw->rendering.pixbuf)
    g_object_ref (sw->rendering.pixbuf);

  set_gc_foreground (sw);

  gtk_widget_set_size_request (sw->da, SWATCH_WIDTH, SWATCH_HEIGHT);
}

static struct swatch *
swatch_create (int i)
{
  struct swatch *sw = g_malloc (sizeof (*sw));

  sw->button = gtk_toggle_button_new ();

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

static void
update_preview_cb (GtkFileChooser *fc, gpointer data)
{
  char *filename;
  GdkPixbuf *pixbuf;

  GtkWidget *image_preview = GTK_WIDGET (data);
  filename = gtk_file_chooser_get_preview_filename (fc);
  if (filename == NULL)
    return;

  pixbuf = gdk_pixbuf_new_from_file_at_size (filename, 128, 128, NULL);
  g_free (filename);

  gtk_image_set_from_pixbuf (GTK_IMAGE (image_preview), pixbuf);

  gtk_file_chooser_set_preview_widget_active (fc, pixbuf != NULL);

  if (pixbuf)
    g_object_unref (pixbuf);
}


/*
 Set the swatch image from a file.  Returns TRUE if successfull
*/
static gboolean
set_swatch_image (const gchar *filename, struct colour_dialog_state *cds)
{
  GtkWindow *window = NULL;

  GError *gerr = 0;

  struct swatch *sw = cds->swatches[cds->selected_swatch];

  sw->rendering.pixbuf = create_pixbuf_from_file (filename, &gerr);
  if (gerr)
    {
      error_dialog (window, gerr->message);
      g_clear_error (&gerr);
      return FALSE;
    }

  sw->rendering.texName = create_pattern_from_pixbuf (sw->rendering.pixbuf, &gerr);

  if (gerr)
    {
      error_dialog (window, _("Cannot create image from file %s: %s"), filename,
		    gerr->message);
      g_clear_error (&gerr);
      return FALSE;
    }
  sw->rendering.surface = SURFACE_TILED;

  gtk_widget_queue_draw (sw->da);

  return TRUE;
}


/* Display a dialog box to set the image */
static void
choose_image (GtkWidget *w, gpointer data)
{
  GtkWidget *fc;
  GtkWidget *image_preview;
  GtkFileFilter *all_files_filter;
  GtkFileFilter *all_images_filter;
  gint response;
  struct colour_dialog_state *cds = data;
  GtkWindow *parent = NULL;

  fc = gtk_file_chooser_dialog_new (_("Image Selector"),
				    parent,
				    GTK_FILE_CHOOSER_ACTION_OPEN,
				    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				    GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);

  gtk_window_set_icon_name (GTK_WINDOW (fc), "gnubik");


  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fc),
				       g_get_home_dir ());
  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (fc), FALSE);
  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (fc), TRUE);
  gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (fc), FALSE);

  /* Thumbnail preview of the image during selection */
  image_preview = gtk_image_new ();
  gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (fc), image_preview);

  g_signal_connect (GTK_FILE_CHOOSER (fc), "update-preview",
		    G_CALLBACK (update_preview_cb), image_preview);

  /* File format filters. Probably just All Images is enough */
  all_images_filter = gtk_file_filter_new ();
  gtk_file_filter_add_pixbuf_formats (all_images_filter);
  gtk_file_filter_set_name (all_images_filter, _("All Images"));
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fc), all_images_filter);
  all_files_filter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (all_files_filter, "*");
  gtk_file_filter_set_name (all_files_filter, _("All Files"));
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fc), all_files_filter);

  gtk_window_set_transient_for (GTK_WINDOW (fc), parent);

  gtk_widget_show (fc);

  response = gtk_dialog_run (GTK_DIALOG (fc));
  if (response != GTK_RESPONSE_OK)
    {
      gtk_widget_destroy (fc);
      return;
    }

  gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fc));
  set_swatch_image (filename, cds);
  g_free (filename);

  gtk_widget_destroy (fc);
}

static void
set_mosaic (GtkToggleButton *b, gpointer data)
{
  struct colour_dialog_state *cds = data;
  struct swatch *sw = cds->swatches[cds->selected_swatch];

  if ( !gtk_toggle_button_get_active (b))
    return;

  sw->rendering.surface = SURFACE_MOSAIC;

  gtk_widget_queue_draw (sw->button);
}

static void
set_tiled (GtkToggleButton *b, gpointer data)
{
  struct colour_dialog_state *cds = data;
  struct swatch *sw = cds->swatches[cds->selected_swatch];
  if ( !gtk_toggle_button_get_active (b))
    return;

  sw->rendering.surface = SURFACE_TILED;

  gtk_widget_queue_draw (sw->button);
}

static void
set_plain (GtkToggleButton *b, gpointer data)
{
  struct colour_dialog_state *cds = data;
  struct swatch *sw = cds->swatches[cds->selected_swatch];

  if ( !gtk_toggle_button_get_active (b))
    return;

  sw->rendering.surface = SURFACE_COLOURED;

  if (sw->rendering.pixbuf)
    g_object_unref (sw->rendering.pixbuf);
  sw->rendering.pixbuf = NULL;

  sw->rendering.texName = 0;

  gtk_widget_queue_draw (sw->button);
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

  cds->button_tile = gtk_radio_button_new_with_mnemonic (0, _("_Tiled"));
  gtk_widget_set_tooltip_text (cds->button_tile,
			       _("Place a copy of the image on each block"));

  cds->button_mosaic =
    gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON
						    (cds->button_tile),
						    _("_Mosaic"));

  gtk_widget_set_tooltip_text (cds->button_mosaic,
			       _("Place a copy of the image across the entire face of the cube"));


  cds->button_plain =
    gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON
						    (cds->button_tile),
						    _("_Plain"));

  gtk_widget_set_tooltip_text (cds->button_plain,
			       _("Remove image from the cube face"));


  g_signal_connect (cds->button_mosaic, "toggled", G_CALLBACK (set_mosaic), cds);
  g_signal_connect (cds->button_tile, "toggled", G_CALLBACK (set_tiled), cds);
  g_signal_connect (cds->button_plain, "toggled", G_CALLBACK (set_plain), cds);

  cds->button_group =
    gtk_radio_button_get_group (GTK_RADIO_BUTTON (cds->button_mosaic));


  gtk_widget_set_sensitive (cds->button_tile, FALSE);
  gtk_widget_set_sensitive (cds->button_mosaic, FALSE);
  gtk_widget_set_sensitive (cds->button_plain, FALSE);

  gtk_box_pack_start (GTK_BOX (image_select_vbox), button_use_image,
		      FALSE, TRUE, BOX_PADDING);
  gtk_box_pack_start (GTK_BOX (image_select_vbox), cds->button_tile,
		      FALSE, TRUE, BOX_PADDING);
  gtk_box_pack_start (GTK_BOX (image_select_vbox), cds->button_mosaic,
		      FALSE, TRUE, BOX_PADDING);
  gtk_box_pack_start (GTK_BOX (image_select_vbox), cds->button_plain,
		      FALSE, TRUE, BOX_PADDING);

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

      g_signal_connect (cds->swatches[i]->button, "toggled",
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

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox_swatch,
			       TRUE, TRUE, BOX_PADDING);

  gtk_box_pack_start (GTK_BOX (hbox_cs), cds->colour_selector,
		      TRUE, TRUE, BOX_PADDING);
  
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox_cs,
			       TRUE, TRUE, BOX_PADDING);

  g_signal_connect (button_use_image, "clicked", 
		    G_CALLBACK (choose_image), cds);

  gtk_widget_show_all (hbox_cs);
  gtk_widget_show_all (hbox_swatch);

  gint resp = gtk_dialog_run (GTK_DIALOG (dialog));

  if ( resp == GTK_RESPONSE_ACCEPT)
    {
      for (i = 0; i < 6 ; ++i)
	{
	  struct swatch *s = cds->swatches[i];
	  setColour (i, &s->rendering);
	}
      gtk_widget_queue_draw (GTK_WIDGET (window));
    }

  gtk_widget_destroy (dialog);

  for (i = 0; i < 6; ++i)
    {
      g_free (cds->swatches[i]);
    }

  g_free (cds);
}
/* end of colour_select_menu (..... */


static gboolean
draw_swatch (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  struct swatch *sw = data;

  if ( sw->rendering.pixbuf == NULL)
    {
      gdk_draw_rectangle (widget->window,
			  sw->gc,
			  TRUE, 0, 0,
			  widget->allocation.width,
			  widget->allocation.height);
    }
  else
    {
      GdkPixbuf *scaled_pixbuf = 0;
      gint width, height;

      gdk_drawable_get_size (widget->window, &width, &height);

      scaled_pixbuf =
	gdk_pixbuf_scale_simple (sw->rendering.pixbuf,
				 width, height, GDK_INTERP_NEAREST);

      g_assert (scaled_pixbuf);

      gdk_draw_pixbuf (widget->window,
		       sw->gc,
		       scaled_pixbuf,
		       0, 0,
		       0, 0, width, height, GDK_RGB_DITHER_NONE, 0, 0);

      g_object_unref (scaled_pixbuf);
    }

  return FALSE;
}
