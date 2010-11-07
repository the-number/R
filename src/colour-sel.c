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
static const char RCSID[]="$Id: colour-sel.c,v 1.6 2009/01/19 06:55:46 jmd Exp $";


#include <config.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <string.h>
#include "glarea.h"
#include "colour-sel.h"
#include "drwBlock.h"
#include "textures.h"
#include "widget-set.h"
#include "select.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "menus-gtk.h"

#define BOX_PADDING 10

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)



extern GtkWidget *main_application_window;
void redraw_swatches (void) ;

static gboolean preset=FALSE;

/* Set a swatch to a particular colour */
void set_swatch_colour (gint swatch,  GdkColor *color) ;
void set_swatch_pattern (gint swatch,  gint pattern);




void initialise_rendering (gint i);


gboolean
draw_swatch (GtkWidget *widget,  GdkEventExpose *event,  gpointer data);

gboolean
draw_texture_sample (GtkWidget *widget,  GdkEventExpose *event,  gpointer data);


gboolean select_side (GtkToggleButton *togglebutton,  gpointer user_data);

gboolean update_swatch_colours (GtkWidget *w,  gpointer user_data);

static gboolean
colour_select_response (GtkWidget *w,  GtkResponseType resp,  gpointer user_data);


void set_cube_rendering (void);

static void select_texture (GtkButton *button,  gpointer user_data) ;


gboolean preset_cs (GtkButton *button,  gpointer user_data) ;


/* An array of widgets showing the color/texture of the faces of the cube */
static GtkWidget *swatch[6];

/* The swatch which we're currently modifying*/
static int selected_swatch=0;


#define SWATCH_WIDTH  64
#define SWATCH_HEIGHT 64



static GdkGC *the_gcs[6]={0, 0, 0, 0, 0, 0};

struct swatch_rendering {
  GdkPixbuf *pixbuf;
  GLfloat color[4];
  gint stock_pattern;
};


struct face_rendering {
  struct cube_rendering rendering;
  struct swatch_rendering sr;
};



static struct face_rendering current_face_rendering[6];
static struct face_rendering proposed_face_rendering[6];

struct  cube_rendering *rendering[6]=
  {
    (struct cube_rendering *) &current_face_rendering[0],
    (struct cube_rendering *) &current_face_rendering[1],
    (struct cube_rendering *) &current_face_rendering[2],
    (struct cube_rendering *) &current_face_rendering[3],
    (struct cube_rendering *) &current_face_rendering[4],
    (struct cube_rendering *) &current_face_rendering[5],
  };



/* Set the sensitivity of the TILE/MOSAIC button for the given swatch */
gboolean set_distr_control_sensitivity (gint swatch);


static GdkPixbuf *
create_pixbuf_from_file (const gchar *filename,  GError **gerr)
{
  GdkPixbuf *unscaled_pixbuf;
  GdkPixbuf *pixbuf;

  unscaled_pixbuf =  gdk_pixbuf_new_from_file (filename,  gerr);

  if ( *gerr )
    return 0;

  /* We must scale the image,  because Mesa/OpenGL insists on it being of size
     2^n ( where n is integer ) */

  pixbuf = gdk_pixbuf_scale_simple (unscaled_pixbuf,
				    SWATCH_WIDTH,  SWATCH_HEIGHT,
				    GDK_INTERP_NEAREST);

  g_object_unref (unscaled_pixbuf);
  unscaled_pixbuf=0;


  return pixbuf;

}



/* Set the swatch image from a file.
   Returns TRUE if successfull
*/
static gboolean
set_swatch_image (gint theSwatch,  const gchar *filename,  GtkWidget *fs )
{
  GLuint texName;

  GdkPixbuf *pixbuf ;
  GError *gerr = 0;

  pixbuf = create_pixbuf_from_file (filename,  &gerr);
  if ( gerr ) {
    error_dialog (fs,  gerr->message);
    g_clear_error (&gerr);
    return FALSE ;
  }


  proposed_face_rendering[theSwatch].sr.pixbuf = pixbuf;

  texName = create_pattern_from_pixbuf (pixbuf, &gerr) ;

  if ( gerr ) {
    error_dialog (fs, _("Cannot create image from file %s: %s"),  filename,
		  gerr->message);
    g_clear_error (&gerr);
    return FALSE ;
  }

  proposed_face_rendering[theSwatch].rendering.type=IMAGED;

  proposed_face_rendering[theSwatch].rendering.texName = texName;

  draw_swatch (swatch[theSwatch], 0, (gpointer)theSwatch);

  set_distr_control_sensitivity (theSwatch);

  return TRUE;
}



/* Open an image file and set it to be the rendering for the currently
   selected swatch */
static void
set_swatch_from_fs (GtkFileChooser *fc)
{

  gchar *filename = gtk_file_chooser_get_filename (fc);

  /* The fc widget is destroyed in the caller function -
      popupImageSelector ().  If the error_dialog () in
     set_swatch_image () is called and error dialog showed ontop of
     the fc widget, after closing the error dialog the fc widget stops
     responding. So the fc widget is destoryed no matter the result
     from set_swatch_image (). So when there is an error with the
     selected file the user should click the image selection button
     again to select a new image. This could also be removed and 
     the code moved to popupImageSelector ().
  */    
  set_swatch_image (selected_swatch,  filename,  GTK_WIDGET (fc));

  g_free (filename);

}


/* Callback to update the image preview in the image selection dialog */
static void
update_preview_cb (GtkFileChooser *fc, gpointer data)
{
  GtkWidget *image_preview;
  char *filename;
  GdkPixbuf *pixbuf;
  gboolean have_preview;

  image_preview = GTK_WIDGET (data);
  filename = gtk_file_chooser_get_preview_filename (fc);
  if ( filename == NULL )
    return ;

  pixbuf = gdk_pixbuf_new_from_file_at_size (filename, 128, 128, NULL);
  have_preview = (pixbuf != NULL);
  g_free (filename);

  gtk_image_set_from_pixbuf (GTK_IMAGE (image_preview), pixbuf);
  if (pixbuf)
    g_object_unref (pixbuf);

  gtk_file_chooser_set_preview_widget_active (fc, have_preview);
}


/* Display a dialog box to set the image */
static void
popupImageSelector (GtkWidget *w,  GtkWidget *parent)
{
  GtkWidget *fc;
  GtkWidget *image_preview;
  GtkFileFilter *all_files_filter;
  GtkFileFilter *all_images_filter;
  gint response ;

  fc = gtk_file_chooser_dialog_new (_("Image Selector"),
				    GTK_WINDOW (parent),
				    GTK_FILE_CHOOSER_ACTION_OPEN,
				    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				    GTK_STOCK_OPEN, GTK_RESPONSE_OK,
				    NULL);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fc),
				       g_get_home_dir ());
  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (fc),
					FALSE);
  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (fc),
				   TRUE);
  gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (fc),
				  FALSE);

  /* Thumbnail preview of the image during selection */
  image_preview = gtk_image_new ();
  gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (fc),
				       image_preview);
  g_signal_connect (GTK_FILE_CHOOSER (fc), "update-preview",
		    G_CALLBACK (update_preview_cb), image_preview);
  

  /* File format filters. Probably just All Images is enough */
  all_images_filter = gtk_file_filter_new ();
  gtk_file_filter_add_pixbuf_formats (all_images_filter);
  gtk_file_filter_set_name (all_images_filter,
			    _("All Images"));
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fc),
			       all_images_filter);
  all_files_filter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (all_files_filter,
			       "*");
  gtk_file_filter_set_name (all_files_filter,
			    _("All Files"));
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fc),
			       all_files_filter);

  g_assert (GTK_IS_WINDOW (parent));
  gtk_window_set_transient_for (GTK_WINDOW (fc),  GTK_WINDOW (parent));

  gtk_widget_show (fc);

  response = gtk_dialog_run (GTK_DIALOG (fc));
  if ( response != GTK_RESPONSE_OK) {
    gtk_widget_destroy (fc);
    return;
  }

  set_swatch_from_fs (GTK_FILE_CHOOSER (fc));
  set_distr_control_sensitivity (-1);

  gtk_widget_destroy (fc);

}


void
initialise_rendering (gint i)
{


  /* If this is happening for the very first time */
  if ( current_face_rendering[i].rendering.type == UNDEFINED ) {

    GLfloat red,  green,  blue;

    current_face_rendering[i].rendering.type=COLORED;
    current_face_rendering[i].rendering.texName =-1;

    getColour (i, &red, &green, &blue);


    current_face_rendering[i].sr.color[0] = red   ;
    current_face_rendering[i].sr.color[1] = green ;
    current_face_rendering[i].sr.color[2] = blue  ;

    current_face_rendering[i].sr.stock_pattern=-1;

  }

}




/* Set the proposed renderings to the current  ones.
   If the current renderings are uninitialised then do so first */
static void
reset_swatch (void)
{
  int i;



  for ( i = 0 ; i < 6 ; ++i ) {
    int j;

    initialise_rendering (i);

    proposed_face_rendering[i]=current_face_rendering[i];

    for (j=0 ; j < 4 ; j++) {
      proposed_face_rendering[i].sr.color[j]=
	current_face_rendering[i].sr.color[j];
    }

    if ( the_gcs[i] ) {
      g_object_unref (the_gcs[i]);
      the_gcs[i]=0;
    }

    set_swatch_pattern (i,  current_face_rendering[i].sr.stock_pattern);

  }


}



/* Set the proposed rendering to be the current rendering */
static void
set_swatch (void)
{

  int i;

  /* Set the proposed gc's to be the same as the current ones */
  for ( i = 0 ; i < 6 ; ++i ) {
    int j;

    current_face_rendering[i]=proposed_face_rendering[i];

    for (j=0 ; j < 4 ; j++) {
      current_face_rendering[i].sr.color[j]=
	proposed_face_rendering[i].sr.color[j];
    }


  }


}



static  GtkWidget *colourSelectorWidget;

static GtkWidget *
createColourSelector (void)
{
  int i;

  gchar *texture_tip=0;



  GtkWidget *button_box;
  GtkWidget *button_no_texture;

  GtkWidget *frame_texture;
  GtkWidget *frame_colour;

  GtkWidget *vbox;



  vbox=gtk_vbox_new (FALSE,  BOX_PADDING);

  colourSelectorWidget = gtk_color_selection_new ();

  gtk_color_selection_set_has_opacity_control (
					       GTK_COLOR_SELECTION (colourSelectorWidget),
					       FALSE);



  frame_texture = gtk_frame_new (_("Pattern"));
  frame_colour  = gtk_frame_new (_("Colour"));



  button_box = gtk_hbutton_box_new ();



  gtk_box_set_spacing (GTK_BOX (button_box), 0);

  gtk_container_set_border_width (GTK_CONTAINER (button_box),  BOX_PADDING);


  gtk_button_box_set_child_size (GTK_BUTTON_BOX (button_box), 25, 25);


  button_no_texture = gtk_button_new ();

  gtk_container_add (GTK_CONTAINER (button_box),  button_no_texture);
  g_signal_connect (button_no_texture, "clicked",
		    G_CALLBACK (select_texture), (gpointer)-1);

  g_signal_connect_swapped (button_no_texture, "clicked",
			    G_CALLBACK (set_distr_control_sensitivity),
			    (gpointer)-1);


  texture_tip=g_strdup (_("Click here to use a pattern on the cube surface"));
  gtk_widget_set_tooltip_text (button_no_texture,  texture_tip);

  for ( i = 0 ; i < 6 ; ++i ) {

    GtkWidget *button;
    GtkWidget *drawingArea;

    button = gtk_button_new ();

    gtk_widget_set_tooltip_text (button,  texture_tip);

    drawingArea = gtk_drawing_area_new ();
    gtk_widget_set_size_request (drawingArea, 45, 45);

    g_signal_connect (drawingArea, "expose-event",
		      G_CALLBACK (draw_texture_sample), (gpointer)i);

    g_signal_connect (button, "clicked",
		      G_CALLBACK (select_texture), (gpointer)i);

    g_signal_connect_swapped (button, "clicked",
			      G_CALLBACK (set_distr_control_sensitivity),
			      (gpointer)-1);


    gtk_container_add (GTK_CONTAINER (button),  drawingArea);


    gtk_container_add (GTK_CONTAINER (button_box),  button);

  }

  g_free (texture_tip);

  g_signal_connect (colourSelectorWidget,  "color-changed",
		    G_CALLBACK (update_swatch_colours),
		    colourSelectorWidget);

  gtk_container_add (GTK_CONTAINER (frame_colour),  colourSelectorWidget);

  gtk_container_add (GTK_CONTAINER (vbox),  frame_colour);



  gtk_container_add (GTK_CONTAINER (frame_texture),  button_box);

  gtk_container_add (GTK_CONTAINER (vbox),
		     frame_texture);


  return vbox;

}


static GSList *button_group;
static GtkWidget *button_tile;
static GtkWidget *button_mosaic;

/* This callback sets the distribution mode of the image */
static void
set_image_distr (GtkToggleButton *b,  enum distrib_type distr)
{
  if (  ! gtk_toggle_button_get_active (b) )
    return ;

  proposed_face_rendering[selected_swatch].rendering.distr=distr;
}

/* Set the state of the toggle buttons depending on the current value
   of the image distribution mode */
static void
get_image_distr (void)
{
  enum distrib_type distr;

  distr=proposed_face_rendering[selected_swatch].rendering.distr;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_tile),  distr==TILED);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_mosaic),  distr==MOSAIC);

}


void
drag_data_received (GtkWidget *widget,
		    GdkDragContext *dc,
		    gint x,  gint y,
		    GtkSelectionData *selection_data,
		    guint info,
		    guint t,
		    gpointer user_data)
{

  gboolean immediate_mode = FALSE;

  /* The special value -1 means that this callback must
     use pickPolygons to find out which face is to be updated
  */
  gint swatch = (gint) user_data;

  if ( swatch == -1) {
    struct facet_selection *fs;

    fs = pickPolygons (x,  y);

    if ( ! fs )  /* If nothing was actually pointed to,  then exit */
      return ;

    swatch = fs->face;

    immediate_mode = TRUE;

  }

  initialise_rendering (swatch);

  switch (info) {
  case RDRAG_COLOUR:
    {
      guint16 *vals;
      GdkColor colour;

      if (selection_data->length < 0)
	return;

      if ((selection_data->format != 16) ||
	  (selection_data->length != 8)) {

	g_warning ("Received invalid color data");

	return;
      }

      vals = (guint16 *)selection_data->data;

      colour.red = vals[0] ;
      colour.green = vals[1];
      colour.blue = vals[2] ;


      if ( immediate_mode ) {
	setColour (swatch,
		   vals[0] / 65535.0 ,
		   vals[1] / 65535.0 ,
		   vals[2] / 65535.0 );
		
	current_face_rendering[swatch].sr.color[0]=vals[0] / 65535.0 ;
	current_face_rendering[swatch].sr.color[1]=vals[1] / 65535.0 ;
	current_face_rendering[swatch].sr.color[2]=vals[2] / 65535.0 ;

	/* FIXME: Must set the graphics context too */


	current_face_rendering[swatch].rendering.type = COLORED;
	current_face_rendering[swatch].rendering.texName = -1;
	current_face_rendering[swatch].rendering.distr = TILED;

      }
      else {
	set_swatch_colour (swatch, &colour);
      }

    }
    break ;
  case RDRAG_FILELIST:
    {
      gchar **s=0;
      gchar **start=0;

      start = s = g_strsplit ((const gchar *) selection_data->data, "\r\n", 0);

      while (*s)
	{
	  gchar *utf8;
	  gchar *filename;

	  GError *gerr=0;

	  if ( strcmp (*s, "") == 0 ) {
	    s++;
	    continue ;
	  }

	  /* Convert to utf8.  Is this necessary ?? */

	  utf8 =  g_locale_to_utf8 (*s, -1, 0, 0, &gerr);
	  if ( gerr ) {
	    g_warning (gerr->message);
	    g_clear_error (&gerr);
	    gerr=0;
	    continue;
	  }

	  /* Extract the filename from the uri */
	  filename = g_filename_from_uri (utf8, 0, &gerr);
	  if ( gerr ) {
	    g_warning (gerr->message);
	    g_clear_error (&gerr);
	    continue;
	  }



	  if ( immediate_mode ) {
	    GLint texName ;

	    GdkPixbuf *pixbuf = 0 ;
	    GError *gerr = 0;

	
	    pixbuf = create_pixbuf_from_file (filename,  &gerr);

	    if ( gerr ) {
	      g_warning (gerr->message);
	      g_clear_error (&gerr);
	      return;
	    }
	
	    texName = create_pattern_from_pixbuf (pixbuf, &gerr);

	    if ( gerr ) {
	      g_warning (gerr->message);
	      g_clear_error (&gerr);
	      return ;
	    }


	    current_face_rendering[swatch].rendering.type = IMAGED;
	    current_face_rendering[swatch].rendering.texName = texName;
	    current_face_rendering[swatch].rendering.distr = TILED;


	    current_face_rendering[swatch].sr.pixbuf = pixbuf;

	  }
	  else {
	    set_swatch_image (swatch,  filename,  main_application_window);
	  }




	  /* For now,  just use the first one.
	     Later,  we'll add some method for disambiguating multiple files
	  */
	  break ;

	  s++;
	}
      g_strfreev (start);

      postRedisplay ();
    }
    break;
    /* Ignore all others */
  }

}

static GdkDrawable **colour_menu_window=0;

void
colour_select_menu ( GtkWidget *w,    gpointer   data )
{
  int i;

  GtkWidget *dialog;
  GtkWidget *hbox_swatch;
  GtkWidget *table;

  GtkWidget *frame;
  GtkWidget *button_use_image;
  GtkWidget *image_select_vbox;
  GtkWidget *image_select_hbox;

  GtkWidget *hbox_cs;
  GtkWidget *colourSelector;



  dialog = gtk_dialog_new_with_buttons (_("Colour selector"),
					GTK_WINDOW (main_application_window),
					GTK_DIALOG_MODAL
					| GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_OK,  GTK_RESPONSE_ACCEPT,
					GTK_STOCK_REFRESH, 0,
					GTK_STOCK_CANCEL,  GTK_RESPONSE_REJECT,
					NULL);

  colour_menu_window = &(dialog->window);



  g_signal_connect (dialog, "show",
		    G_CALLBACK (reset_swatch), 0);


  colourSelector = createColourSelector ();

  hbox_swatch = gtk_hbox_new (FALSE,  BOX_PADDING);
  hbox_cs = gtk_hbox_new (FALSE,  BOX_PADDING);

  image_select_vbox = gtk_vbox_new (FALSE,  BOX_PADDING);
  image_select_hbox = gtk_hbox_new (FALSE,  BOX_PADDING);

  {
    frame = gtk_frame_new (_("Image"));
    button_use_image = gtk_button_new_with_mnemonic (_("Se_lect"));

    gtk_widget_set_tooltip_text (button_use_image,
				 _("Select an image file"));
				 //			  _("Use an image file on the cube face"));

    button_tile = gtk_radio_button_new_with_mnemonic (0, _("_Tiled"));
    gtk_widget_set_tooltip_text (button_tile,
			  _("Place a copy of the image\non each block"));

    button_mosaic = gtk_radio_button_new_with_mnemonic_from_widget (
								    GTK_RADIO_BUTTON (button_tile),
								    _("_Mosaic"));

    gtk_widget_set_tooltip_text (button_mosaic,
			  _("Place a copy of the image across\nthe entire face of the cube"));




    g_signal_connect (button_tile, "toggled",
		      G_CALLBACK (set_image_distr),
		      (gpointer) TILED);

    g_signal_connect (button_mosaic, "toggled",
		      G_CALLBACK (set_image_distr),
		      (gpointer) MOSAIC);

    button_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button_mosaic));

    gtk_widget_set_sensitive (button_tile,  FALSE);
    gtk_widget_set_sensitive (button_mosaic,  FALSE);

    g_assert (GTK_IS_WINDOW (dialog));
    g_signal_connect (button_use_image, "clicked",
		      G_CALLBACK (popupImageSelector),
		      (gpointer) dialog);


    gtk_box_pack_start (GTK_BOX (image_select_vbox),  button_use_image,
			TRUE,  TRUE,  BOX_PADDING);
    gtk_box_pack_start (GTK_BOX (image_select_vbox),  button_tile,
			TRUE,  TRUE,  BOX_PADDING);
    gtk_box_pack_start (GTK_BOX (image_select_vbox),  button_mosaic,
			TRUE,  TRUE,  BOX_PADDING);

    gtk_box_pack_start (GTK_BOX (image_select_hbox),  image_select_vbox,
			TRUE,  TRUE,  BOX_PADDING);

    gtk_container_add (GTK_CONTAINER (frame),  image_select_hbox);
  }



  table = gtk_table_new (2, 3,  TRUE);

  for ( i = 0 ; i < 6 ; ++i ) {

    GtkWidget *button;
    const GtkTargetEntry target[2] ={
      { "text/uri-list",  0,  RDRAG_FILELIST },
      { "application/x-color",  0,  RDRAG_COLOUR },
    };

    button = gtk_button_new ();

    gtk_drag_dest_set (button,  GTK_DEST_DEFAULT_ALL,
		       target, 2,  GDK_ACTION_COPY);


    g_signal_connect (button, "drag_data_received",
		      G_CALLBACK (drag_data_received), (gpointer)i);


    gtk_widget_set_tooltip_text (button,
			  _("A sample of the colour or pattern. You can click and select a new colour or pattern,  or drag one to this space."));

    swatch[i] = gtk_drawing_area_new ();

    gtk_widget_set_size_request (swatch[i],
				 SWATCH_WIDTH,
				 SWATCH_HEIGHT);

    g_signal_connect (swatch[i], "expose-event",
		      G_CALLBACK (draw_swatch), (gpointer)i);


    g_signal_connect (button, "clicked",
		      G_CALLBACK (select_side),
		      (gpointer)i);

    g_signal_connect (button, "clicked",
		      G_CALLBACK (preset_cs),
		      colourSelectorWidget);

    g_signal_connect_swapped (button, "clicked",
			      G_CALLBACK (set_distr_control_sensitivity),
			      (gpointer)i);



    gtk_container_add (GTK_CONTAINER (button),  swatch[i]);
    gtk_table_attach (GTK_TABLE (table),  button,  i%3,  i%3+1,  i/3,  i/3+1,
		      GTK_EXPAND,  GTK_EXPAND, 0, 0
		      );

  }

  g_signal_connect (dialog, "response",
		    G_CALLBACK (colour_select_response), 0);

  gtk_container_add (GTK_CONTAINER (hbox_swatch),
		     table);

  gtk_box_pack_start (GTK_BOX (hbox_swatch),  frame,
		      TRUE,  TRUE,  BOX_PADDING);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox),
		     hbox_swatch);

  gtk_box_pack_start (GTK_BOX (hbox_cs),  colourSelector,
		      TRUE,  TRUE,  BOX_PADDING);


  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox),
		     hbox_cs);



  gtk_widget_show_all (dialog);

} /* end of colour_select_menu (..... */




gboolean
draw_texture_sample (GtkWidget *widget,  GdkEventExpose *event,  gpointer data)
{
  int i;

  GdkGC *gc ;
  GdkColor black={0, 0, 0, 0};
  GdkColor white={0, ~0, ~0, ~0};
  GdkPixmap *pixmap;

  i = (int) data;

  gc = gdk_gc_new (widget->window);

  gdk_gc_set_rgb_fg_color (gc, &black);
  gdk_gc_set_rgb_bg_color (gc, &white);

  gdk_gc_set_fill (gc,  GDK_OPAQUE_STIPPLED);


  pixmap = gdk_bitmap_create_from_data (0, (gchar *)stock_pattern[i].data,
					checkImageWidth,
					checkImageHeight);

  gdk_gc_set_stipple (gc,  pixmap);


  gdk_draw_rectangle (widget->window,
		      gc,
		      TRUE, 0,  0,
		      widget->allocation.width,
		      widget->allocation.height);

  return TRUE;
}


/* This function sets the GC foreground colour from the
   Mesa/OpenGL colour values provided */
static void
set_gc_foreground (GdkGC *gc,  GLdouble red,  GLdouble green,  GLdouble blue)
{
  GdkColor fg;

  g_assert (gc);

  fg.red     = red   * 65535.0;
  fg.green   = green * 65535.0;
  fg.blue    = blue  * 65535.0;


  gdk_gc_set_rgb_fg_color (gc, &fg);

}




static GdkGC *
get_gc_for_swatch (gint i,  GtkWidget *w)
{


  if ( ! the_gcs[i] ) {
    GLfloat red;
    GLfloat green;
    GLfloat blue;
    GdkGC *gc=0;

    g_assert (*colour_menu_window);

    gc = gdk_gc_new (*colour_menu_window);


    getColour (i, &red, &green, &blue);
    set_gc_foreground (gc,  red,  green,  blue);

    the_gcs[i]=gc;
  }

  return the_gcs[i];

}


gboolean
draw_swatch (GtkWidget *widget,  GdkEventExpose *event,  gpointer data)
{

  int i;
  GdkGC *gc= 0 ;


  i = (int) data;

  gc = get_gc_for_swatch (i,  widget);


  switch ( proposed_face_rendering[i].rendering.type) {

  case COLORED:
    {


      gdk_draw_rectangle (widget->window,
			  gc,
			  TRUE, 0,  0,
			  widget->allocation.width,
			  widget->allocation.height);
    }
    break ;

  case IMAGED:
    {
      GdkPixbuf *scaled_pixbuf=0;
      gint width,  height;



      gdk_drawable_get_size (widget->window, &width, &height);

      g_assert (proposed_face_rendering[i].sr.pixbuf);

      scaled_pixbuf = gdk_pixbuf_scale_simple (
					       proposed_face_rendering[i].sr.pixbuf,
					       width,  height,
					       GDK_INTERP_NEAREST);


      g_assert (scaled_pixbuf);


      gdk_draw_pixbuf (widget->window,
				     gc,
				     scaled_pixbuf,
				     0, 0,
				     0, 0,
				     width,  height,
				     GDK_RGB_DITHER_NONE,
				     0, 0);
    }
    break ;
  default:
    g_assert_not_reached ();
    break;
  };


  return TRUE;
}



/* Preset the colour selector value to the value on the swatch */
gboolean
preset_cs (GtkButton *button,  gpointer user_data)
{
  GdkColor colour;

  GtkColorSelection *cs = GTK_COLOR_SELECTION (user_data);

  GdkColormap *cmap = gtk_widget_get_colormap (GTK_WIDGET (cs));

  preset = TRUE;

  colour.red = proposed_face_rendering[selected_swatch].sr.color[0] * 65535.0;
  colour.green = proposed_face_rendering[selected_swatch].sr.color[1] *65535.0;
  colour.blue = proposed_face_rendering[selected_swatch].sr.color[2] * 65535.0;

  gdk_rgb_find_color (cmap, &colour);

  gtk_color_selection_set_current_color (cs, &colour);

  return FALSE;
}



/* Select a colour of the cube,  for which a colour needs to be decided
 */
gboolean
select_side (GtkToggleButton *togglebutton,  gpointer user_data)
{

  selected_swatch = (int) user_data;

  get_image_distr ();

  return FALSE;

}


/* Set a swatch to a particular colour */
void
set_swatch_colour (gint swatch,  GdkColor *color)
{

  GdkGC *gc = 0;

  gc = get_gc_for_swatch (swatch, 0);

  /* If the type was imaged,  (we're now changing to COLORED),  set the
     texture to the off value */
  if (   proposed_face_rendering[swatch].rendering.type == IMAGED ) {

    proposed_face_rendering[swatch].rendering.texName = -1;
  }


  proposed_face_rendering[swatch].rendering.type = COLORED ;

  proposed_face_rendering[swatch].sr.color[0]=color->red / 65535.0;
  proposed_face_rendering[swatch].sr.color[1]=color->green / 65535.0;
  proposed_face_rendering[swatch].sr.color[2]=color->blue / 65535.0;

  set_gc_foreground (gc,  color->red / 65535.0,
		     color->green / 65535.0,
		     color->blue / 65535.0);
		
  set_distr_control_sensitivity (swatch);

}

/* This function sets the currently selected swatch to
   the colour on the clr_sel widget */
gboolean
update_swatch_colours (GtkWidget *w,
		       gpointer user_data)
{

  GtkColorSelection *clr_sel = GTK_COLOR_SELECTION (user_data);



  GdkColor color;


  /* If this callback is a result of the colour select widget being
     preset,  then just ignore it */
  if ( preset ) {
    preset = FALSE;
    return FALSE ;
  }


  gtk_color_selection_get_current_color (clr_sel, &color);

  set_swatch_colour (selected_swatch, &color);


  draw_swatch (swatch[selected_swatch], 0, (gpointer)selected_swatch);



  return FALSE;

}


static void
set_face_rendering (gint face)
{
  GLfloat red =0;
  GLfloat blue =0;
  GLfloat green =0;

  red = current_face_rendering[face].sr.color[0];
  green = current_face_rendering[face].sr.color[1];
  blue = current_face_rendering[face].sr.color[2];

  setColour (face,  red,  green,  blue);

}

/* Set the colours of the cube,  from the colour swatch */
void
set_cube_rendering (void)
{

  int i;


  for (i = 0 ; i < 6 ; ++i ) {
    set_face_rendering (i);
  }

  postRedisplay ();

}


void
set_swatch_pattern (gint swatch,  gint pattern)
{

  GdkGC *gc=0;
  GdkPixmap *pixmap;

  gc = get_gc_for_swatch (swatch, 0);

  if ( -1 == pattern ) {

    gdk_gc_set_fill (
		     gc,
		     GDK_SOLID);

  }
  else {
    /* Set the graphics context to that appropriate for the pattern selected */

    gdk_gc_set_function (gc,  GDK_COPY);

    gdk_gc_set_fill (gc,  GDK_OPAQUE_STIPPLED);


    pixmap = gdk_bitmap_create_from_data (0, (gchar *)stock_pattern[pattern].data,
					  checkImageWidth,
					  checkImageHeight);

    gdk_gc_set_stipple (gc,  pixmap);
  }

}

/* Select a texture */
static void
select_texture (GtkButton *button,  gpointer user_data)
{

  int i = (int) user_data;


  proposed_face_rendering[selected_swatch].rendering.type = COLORED;
  proposed_face_rendering[selected_swatch].sr.stock_pattern=i;

  /* -1 is a sentinel.  It means no texture at all */
  if ( -1 == i ) {
    proposed_face_rendering[selected_swatch].rendering.texName=-1;
  }
  else {
    proposed_face_rendering[selected_swatch].rendering.texName=stock_pattern[i].texName;
  }

  set_swatch_pattern (selected_swatch,  i);

  draw_swatch (swatch[selected_swatch], 0, (gpointer)selected_swatch);

}

void
redraw_swatches (void)
{
  int i;

  for (i = 0 ; i < 6 ; ++i ) {
    draw_swatch (swatch[i], 0, (gpointer)i);
  }
}




/* Set the sensitivity of the TILE/MOSAIC buttons */
gboolean
set_distr_control_sensitivity (gint swatch)
{

  if (swatch == -1 )
    swatch = selected_swatch;

  if ( proposed_face_rendering[swatch].rendering.type == IMAGED )
    g_slist_foreach (button_group,
		     (GFunc)gtk_widget_set_sensitive, (gpointer) TRUE);
  else
    g_slist_foreach (button_group,
		     (GFunc)gtk_widget_set_sensitive, (gpointer) FALSE);

  return FALSE;
}



static gboolean
colour_select_response (GtkWidget *w,
			GtkResponseType resp,  gpointer user_data)
{
  switch (resp )
    {
    case GTK_RESPONSE_ACCEPT:
      set_swatch ();
      set_cube_rendering ();
      gtk_widget_destroy (w);
      break;
    case GTK_RESPONSE_REJECT:
      reset_swatch ();
      gtk_widget_destroy (w);
      break;
    case 0:
      reset_swatch ();
      redraw_swatches ();
      break;
    case GTK_RESPONSE_DELETE_EVENT:
      break;
    default:
      g_assert_not_reached ();
    };

  return FALSE;
}
