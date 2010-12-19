/*
    GNUbik -- A 3 dimensional magic cube game.
    Copyright (C) 2003, 2004, 2010  John Darrington

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
#include "dialogs.h"
#include <stdlib.h>

#include "cube.h"
#include "glarea.h"
#include "version.h"
#include "ui.h"
#include "drwBlock.h"
#include "widget-set.h"

#include <glib.h>
#include <assert.h>

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)

extern int frameQty;

static int new_frameQty;
static int new_dim;

static void set_lighting (GtkToggleButton * b, gpointer user_data);


static void value_changed (GtkAdjustment * adj, gpointer user_data);


static void
set_lighting (GtkToggleButton * b, gpointer user_data)
{

  if (gtk_toggle_button_get_active (b))
    glEnable (GL_LIGHTING);
  else
    glDisable (GL_LIGHTING);

  postRedisplay ();
}



extern Move_Queue *move_queue;


static gboolean
new_game (gpointer p)
{
  int i;
  int *new_dimension = p;

  if (is_animating ())
    return TRUE;

  re_initialize_glarea ();

  destroy_the_cube ();
  /* create the cube */
  create_the_cube (*new_dimension);

  for (i = 0; i < 8 * cube_get_dimension (the_cube); i++)
    {
      Slice_Blocks *slice =
	identify_blocks (the_cube, rand () % cube_get_number_of_blocks (the_cube), rand () % 3);

      rotate_slice (the_cube, rand () % 2 + 1, slice);

      free_slice_blocks (slice);
    }

  postRedisplay ();

  if (move_queue != NULL)
    free_move_queue (move_queue);
  move_queue = new_move_queue ();

  set_toolbar_state (0);
  update_statusbar ();


  return FALSE;
}



/* Request that the game be restarted
   If data is non zero,  then all it's data will be reallocated
*/
static int new_dim;
void
request_new_game (GtkAction *act, int *dim)
{
  new_dim = *dim;
  request_stop ();
  if (is_animating ())
    abort_animation ();
  g_idle_add (new_game, dim);
}


static void size_changed (GtkEditable * editable, gpointer data);

#define BOX_PADDING 10

static GtkWidget *
create_dimension_widget (void)
{
  GtkWidget *label;


  GtkWidget *hbox = gtk_hbox_new (FALSE, BOX_PADDING);

  GtkObject *adj =
    gtk_adjustment_new (cube_get_dimension (the_cube), 1, G_MAXFLOAT, 1, 1, 0);

  GtkWidget *entry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 0, 0);

  gtk_widget_set_tooltip_text (entry,
			       _("Sets the number of blocks in each side"));

  label = gtk_label_new (_("Size of cube:"));

  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);


  new_dim = cube_get_dimension (the_cube);

  g_signal_connect (entry, "changed", G_CALLBACK (size_changed), 0);


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



  vboxOuter = gtk_vbox_new (TRUE, BOX_PADDING);
  vbox = gtk_vbox_new (FALSE, BOX_PADDING);

  adjustment = gtk_adjustment_new (frameQty, 0, 99, 1, 1, 0);


  hbox = gtk_hbox_new (FALSE, 0);
  label1 = gtk_label_new (_("Faster"));
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);

  label2 = gtk_label_new (_("Slower"));
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_RIGHT);

  scrollbar = gtk_hscrollbar_new (GTK_ADJUSTMENT (adjustment));
  gtk_widget_set_tooltip_text (scrollbar,
			       _
			       ("Controls the speed with which slices rotate"));


  gtk_box_pack_start (GTK_BOX (hbox), label1, TRUE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), label2, TRUE, FALSE, 0);


  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), scrollbar, TRUE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vboxOuter), vbox, TRUE, TRUE, 0);


  /* Add any necessary callbacks */

  g_signal_connect (adjustment, "value-changed",
		    G_CALLBACK (value_changed), 0);

  /* Show the widgets */

  gtk_widget_show_all (vbox);



  return vboxOuter;
}

static gboolean new_values = FALSE;

static gboolean
confirm_preferences (GtkWindow *window)
{
  /* Popup dialog asking whether to restart the cube */
  GtkWidget *dialog =
    gtk_message_dialog_new (window,
			    GTK_DIALOG_MODAL |
			    GTK_DIALOG_DESTROY_WITH_PARENT,
			    GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
			    _("Start cube with new settings?"));

  gtk_window_set_icon_name (GTK_WINDOW(dialog), "gnubik");

  gint resp = gtk_dialog_run (GTK_DIALOG (dialog));

  gtk_widget_destroy (dialog);

  return (GTK_RESPONSE_YES == resp);
}

void
preferences_dialog (GtkWidget * w, GtkWindow *toplevel)
{
  gint response;

  GtkWidget *dimensions;
  GtkWidget *animations;

  GtkWidget *button_lighting;

  GtkWidget *dialog = gtk_dialog_new_with_buttons (_("Preferences"),
						   toplevel,
						   GTK_DIALOG_MODAL |
						   GTK_DIALOG_DESTROY_WITH_PARENT,
						   GTK_STOCK_OK,
						   GTK_RESPONSE_ACCEPT,
						   GTK_STOCK_CANCEL,
						   GTK_RESPONSE_CANCEL,
						   NULL);

  GtkWidget *vbox = GTK_DIALOG (dialog)->vbox;


  GtkWidget *frame_dimensions = gtk_frame_new (_("Dimensions"));
  GtkWidget *frame_animation = gtk_frame_new (_("Animation"));
  GtkWidget *frame_lighting = gtk_frame_new (_("Lighting"));

  gtk_window_set_icon_name (GTK_WINDOW(dialog), "gnubik");

  gtk_window_set_transient_for (GTK_WINDOW (dialog), toplevel);

  dimensions = create_dimension_widget ();
  gtk_container_add (GTK_CONTAINER (frame_dimensions), dimensions);

  animations = create_animation_widget ();
  gtk_container_add (GTK_CONTAINER (frame_animation), animations);

  button_lighting = gtk_check_button_new_with_label (_("Enable lighting"));
  gtk_widget_set_tooltip_text (button_lighting,
			       _("Makes the cube appear illuminated"));

  gtk_container_add (GTK_CONTAINER (frame_lighting), button_lighting);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_lighting),
				glIsEnabled (GL_LIGHTING));

  gtk_box_pack_start (GTK_BOX (vbox), frame_dimensions, FALSE, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame_animation, FALSE, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame_lighting, FALSE, 0, 0);

  g_signal_connect (button_lighting, "toggled", G_CALLBACK (set_lighting), 0);

  gtk_widget_show_all (vbox);

  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (response == GTK_RESPONSE_ACCEPT)
    {
      if (new_values)
	frameQty = new_frameQty;

      if (new_dim == cube_get_dimension (the_cube) || confirm_preferences (toplevel))
	{
	  request_new_game (NULL, &new_dim);
	}
    }

  gtk_widget_destroy (dialog);
}




static void
value_changed (GtkAdjustment * adj, gpointer user_data)
{
  new_values = TRUE;
  new_frameQty = adj->value;
}


static void
size_changed (GtkEditable * editable, gpointer data)
{
  gchar *str;

  str = gtk_editable_get_chars (GTK_EDITABLE (editable), 0, -1);

  new_dim = g_strtod (str, 0);

  g_free (str);
}


void
about_dialog (GtkWidget * w, GtkWindow * toplevel)
{
  /* Create the widgets */
  GtkWidget *dialog = gtk_about_dialog_new ();

  gtk_window_set_icon_name (GTK_WINDOW(dialog), "gnubik");

  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (dialog), toplevel);

  gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (dialog), PACKAGE_NAME);

  gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (dialog), PACKAGE_VERSION);

  gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (dialog), PACKAGE_URL);

  gtk_about_dialog_set_license (GTK_ABOUT_DIALOG (dialog), copyleft_notice);

  gtk_about_dialog_set_logo_icon_name (GTK_ABOUT_DIALOG (dialog), "gnubik");

  gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (dialog),
				 _("A 3 dimensional magic cube puzzle"));

  gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG (dialog),
       /* TRANSLATORS: Do not translate this string. Instead, use it to list
	  the people who have helped with translation to your language. */
					   _("translator-credits"));

  gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (dialog),
				  "Copyright © 2011; John Darrington, Dale Mellor");

  gtk_dialog_run (GTK_DIALOG (dialog));

  gtk_widget_destroy (GTK_WIDGET (dialog));
}
