/*
    GNUbik -- A 3 dimensional magic cube game.
    Copyright (C) 2003, 2004  John Darrington

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
#include "menus-gtk.h"
#include <stdlib.h>

#include "cube.h"
#include "glarea.h"
#include "version.h"
#include "ui.h"
#include "drwBlock.h"
#include "widget-set.h"
#include "gnubik.h"

#include <memory.h>
#include <glib.h>
#include <assert.h>

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)

extern int frameQty;

static int new_frameQty;
static int new_dim ;

static void confirm_preferences (void);

static void
set_lighting (GtkToggleButton *b,  gpointer user_data);


static void
value_changed (GtkAdjustment *adj,  gpointer user_data);


#if DEBUG
static void manual_move (GtkWidget *w ,  Move_Data * data);

static void
fill_value (GtkAdjustment *adj,  int *p)
{
  *p = (int) adj->value;
}

#endif


static void
set_lighting (GtkToggleButton *b ,  gpointer user_data)
{

  if (   gtk_toggle_button_get_active (b) )
    glEnable (GL_LIGHTING);
  else
    glDisable (GL_LIGHTING);

  postRedisplay ();
}



extern Move_Queue *move_queue;

gboolean new_game (gpointer p);

gboolean
new_game (gpointer p)
{
  int i;

  int reallocate = GPOINTER_TO_INT (p);

  if ( is_animating () )
    return TRUE;

  if ( reallocate )
    {

      re_initialize_glarea ();


      destroy_the_cube ();
      /* create the cube */
      number_of_blocks = create_the_cube (cube_dimension);
      if ( 0 > number_of_blocks ) {
	print_cube_error (the_cube,  "Error creating cube");
	exit (-1);
      }

    }

  for ( i = 0; i < 8*cube_dimension; i++ )
  {
      Slice_Blocks *slice =
          identify_blocks (the_cube,  rand () % number_of_blocks,  rand () % 3);

      rotate_slice (the_cube,  rand ()%2 +1,  slice);

      free_slice_blocks (slice);
  }

  postRedisplay ();

  if (move_queue != NULL) free_move_queue (move_queue);
  move_queue = new_move_queue ();

  set_toolbar_state (0);
  update_statusbar ();


  return FALSE;
}



/* Request that the game be restarted
   If data is non zero,  then all it's data will be reallocated
*/
void
request_new_game ( GtkWidget *w,  gpointer   reallocate )
{

  request_stop ();
  if (is_animating () )
    abort_animation ();
  g_idle_add (new_game,  reallocate);

}


static void  size_changed  (GtkEditable *editable,  gpointer data);

#define BOX_PADDING 10

static GtkWidget *
create_dimension_widget (void)
{
  GtkWidget *label;


  GtkWidget *hbox = gtk_hbox_new (FALSE,  BOX_PADDING);

  GtkObject *adj = gtk_adjustment_new (cube_dimension, 1,  G_MAXFLOAT, 1, 1, 10);

  GtkWidget *entry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 0, 0);

  gtk_widget_set_tooltip_text (entry, _("Sets the number of blocks in each side"));

  label  = gtk_label_new (_("Size of cube:"));

  gtk_box_pack_start (GTK_BOX (hbox),  label,  TRUE,  FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox),  entry,  FALSE,  FALSE, 0);


  new_dim = cube_dimension;

  g_signal_connect (entry, "changed",
		     G_CALLBACK (size_changed), 0);


  gtk_widget_show_all (hbox);

  return hbox;
}




static GtkWidget *
create_animation_widget (void)
{
  GtkWidget *vboxOuter;
  GtkWidget *label1;
  GtkWidget *label2;
  GtkWidget *vbox;
  GtkObject *adjustment;
  GtkWidget *scrollbar;
  GtkWidget *hbox;



  vboxOuter = gtk_vbox_new (TRUE,  BOX_PADDING);
  vbox = gtk_vbox_new (FALSE,  BOX_PADDING);

  adjustment = gtk_adjustment_new (frameQty, 0, 99, 1, 1, 10);


  hbox = gtk_hbox_new (FALSE, 0);
  label1  = gtk_label_new (_("Faster"));
  gtk_label_set_justify (GTK_LABEL (label1),  GTK_JUSTIFY_LEFT);

  label2  = gtk_label_new (_("Slower"));
  gtk_label_set_justify (GTK_LABEL (label2),  GTK_JUSTIFY_RIGHT);

  scrollbar = gtk_hscrollbar_new (GTK_ADJUSTMENT (adjustment));
  gtk_widget_set_tooltip_text (scrollbar,
		       _("Controls the speed with which slices rotate"));


  gtk_box_pack_start (GTK_BOX (hbox),  label1,  TRUE,  FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox),  label2,  TRUE,  FALSE, 0);


  gtk_box_pack_start (GTK_BOX (vbox),  hbox,  TRUE,  FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox),  scrollbar,  TRUE,  FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vboxOuter),  vbox,  TRUE,  TRUE, 0);


  /* Add any necessary callbacks */

  g_signal_connect (adjustment, "value-changed",
		     G_CALLBACK (value_changed), 0);

  /* Show the widgets */

  gtk_widget_show_all (vbox);



  return vboxOuter;
}



void
preferences (GtkWidget *w, gpointer data)
{
  gint response;

  GtkWidget *frame_dimensions;
  GtkWidget *frame_animation;
  GtkWidget *dimensions;
  GtkWidget *animations;

  GtkWidget *frame_lighting;
  GtkWidget *button_lighting;

  GtkWindow *toplevel  = GTK_WINDOW (data);


  GtkWidget *dialog = gtk_dialog_new_with_buttons (_("Preferences"),
						   toplevel,
						   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						   GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
						   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						   NULL);

  GtkWidget *vbox = GTK_DIALOG (dialog)->vbox;

  gtk_window_set_transient_for (GTK_WINDOW (dialog),  toplevel);


  frame_dimensions = gtk_frame_new (_("Dimensions"));
  frame_animation = gtk_frame_new (_("Animation"));
  frame_lighting = gtk_frame_new (_("Lighting"));


  dimensions = create_dimension_widget ();
  gtk_container_add (GTK_CONTAINER (frame_dimensions),  dimensions);

  animations = create_animation_widget ();
  gtk_container_add (GTK_CONTAINER (frame_animation),  animations);

  button_lighting = gtk_check_button_new_with_label (_("Enable lighting"));
  gtk_widget_set_tooltip_text (button_lighting,
		       _("Makes the cube appear illuminated"));

  gtk_container_add (GTK_CONTAINER (frame_lighting),  button_lighting);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_lighting),
			       glIsEnabled (GL_LIGHTING));


  gtk_box_pack_start (GTK_BOX (vbox),  frame_dimensions,  FALSE, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox),  frame_animation,  FALSE, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox),  frame_lighting,  FALSE, 0, 0);



  g_signal_connect (button_lighting, "toggled",
		     G_CALLBACK (set_lighting),
		     0);

  gtk_widget_show_all (vbox);

  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if ( response == GTK_RESPONSE_ACCEPT )
	confirm_preferences ();

  gtk_widget_destroy (dialog);
}

extern GtkWidget *main_application_window;

static gboolean new_values=FALSE;

static void
yesnoresp (GtkWidget *w,  gint resp,  gpointer data)
{
  switch ( resp ) {
  case GTK_RESPONSE_YES:

    cube_dimension = new_dim ;

    request_new_game (w,  GINT_TO_POINTER (1));

    break ;

  case GTK_RESPONSE_NO:
    break ;

  default:
    g_assert_not_reached ();
  }

  gtk_widget_destroy (w);

}

/* Close the preferences window,  and update all the necessary values */
static void
confirm_preferences (void)
{

  if ( new_values) 
    frameQty = new_frameQty;

  if ( new_dim != cube_dimension) 
  {
    /* Popup dialog asking whether to restart the cube */
    GtkWidget *dialog = 
	gtk_message_dialog_new ( GTK_WINDOW (main_application_window),
			     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			     GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
			     _("Start cube with new settings?"));

    g_signal_connect_swapped (dialog,  "response",
                           G_CALLBACK (yesnoresp),
                           dialog);

    gtk_widget_show_all (dialog);
  }
}




static void
value_changed (GtkAdjustment *adj, gpointer user_data)
{
  new_values = TRUE;
  new_frameQty = adj->value;
}




static void
size_changed (GtkEditable *editable,  gpointer data)
{
  gchar *str;

  str = gtk_editable_get_chars (GTK_EDITABLE (editable), 0, -1);

  new_dim = g_strtod (str, 0);

  g_free (str);
}


#if DEBUG
void
move (GtkWidget *w, gpointer   data )
{
  GtkWidget *dialog;

  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *hbox4;
  GtkWidget *button_turn;
  GtkWidget *button_close;
  GtkObject *adj1,    *adj2,      *adj4;
  GtkWidget *label1,  *label2,    *label4;
  GtkWidget *sb1,     *sb2,       *sb4;

  static Move_Data md;

  dialog = gtk_dialog_new ();


  button_turn = gtk_button_new_with_label ("Turn");
  button_close = gtk_button_new_with_label ("Close");


  hbox1 = gtk_hbox_new (TRUE, 0);
  adj1 = gtk_adjustment_new (0, 0,  number_of_blocks-1, 1, 1, 10);

  g_signal_connect (adj1, "value-changed",  G_CALLBACK (fill_value), &md.slice);

  sb1 = gtk_spin_button_new (GTK_ADJUSTMENT (adj1), 0, 0);
  label1  = gtk_label_new ("Block to move:");


  hbox2 = gtk_hbox_new (TRUE, 0);
  adj2 = gtk_adjustment_new (0, 0, 2, 1, 1, 10);
  g_signal_connect (adj2, "value-changed",  G_CALLBACK (fill_value), &md.axis);
  sb2 = gtk_spin_button_new (GTK_ADJUSTMENT (adj2), 0, 0);
  label2  = gtk_label_new ("Axis:");

  hbox4 = gtk_hbox_new (TRUE, 0);
  adj4 = gtk_adjustment_new (0, 0, 1, 1, 1, 1);
  g_signal_connect (adj4, "value-changed",  G_CALLBACK (fill_value), &md.dir);
  sb4 = gtk_spin_button_new (GTK_ADJUSTMENT (adj4), 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (sb4),  TRUE);
  label4  = gtk_label_new ("Direction:");



  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
                      button_turn);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
                      button_close);


  gtk_box_pack_start (GTK_BOX (hbox1),  label1,  TRUE,  TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox1),  sb1,  TRUE,  TRUE, 0);

  gtk_box_pack_start (GTK_BOX (hbox2),  label2,  TRUE,  TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2),  sb2,  TRUE,  TRUE, 0);


  gtk_box_pack_start (GTK_BOX (hbox4),  label4,  TRUE,  TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox4),  sb4,  TRUE,  TRUE, 0);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox),
                      hbox1);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox),
                      hbox2);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox),
                      hbox4);

  /* Ensure that the dialog box is destroyed when the user responds. */


  g_signal_connect (button_turn,
		     "clicked",
		     G_CALLBACK (manual_move), &md);

  g_signal_connect_swapped (button_close,
		     "clicked",
		     G_CALLBACK (gtk_widget_destroy), (gpointer) dialog);



  gtk_widget_show_all (dialog);
}






/* Manually make a move on the cube */
static void
manual_move (GtkWidget *w,  Move_Data * data)
{
  /* Just loose the first argument and call the next function */

  request_rotation (data);

  postRedisplay ();
}


#endif

void
about (GtkWidget *w, gpointer data)
{
  GtkWindow *toplevel = GTK_WINDOW (data);

  /* Create the widgets */
  GtkWidget *dialog = gtk_about_dialog_new ();

  gtk_window_set_modal (GTK_WINDOW (dialog),  TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (dialog),  toplevel);

  gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (dialog), copyleft_notice);

  gtk_dialog_run (GTK_DIALOG (dialog));
  
  gtk_widget_destroy (GTK_WIDGET (dialog));
}
