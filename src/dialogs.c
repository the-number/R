/*
    GNUbik -- A 3 dimensional magic cube game.
    Copyright (C) 2003, 2004, 2011  John Darrington

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
#include "version.h"
#include "drwBlock.h"
#include "menus.h"

#include "game.h"

#include <glib.h>
#include <assert.h>

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)

#define BOX_PADDING 10

struct preferences_state
{
  GtkObject *adj[3];
  GtkWidget *entry[3];
};

static struct preferences_state *
pref_state_create (GtkBox * parent, const GbkGame * game)
{
  gint i;
  struct preferences_state *ps = g_malloc (sizeof (*ps));

  for (i = 0; i < 3; ++i)
    {
      ps->adj[i] =
	gtk_adjustment_new (gbk_cube_get_size (game->cube, 0), 1, G_MAXFLOAT,
			    1, 1, 0);
      g_object_ref (ps->adj[i]);
      ps->entry[i] = gtk_spin_button_new (GTK_ADJUSTMENT (ps->adj[i]), 0, 0);
      g_object_ref (ps->entry[i]);
      gtk_box_pack_start (parent, ps->entry[i], FALSE, FALSE, 0);
    }

  return ps;
}

static void
pref_state_destroy (struct preferences_state *ps)
{
  int i;
  for (i = 0; i < 3; ++i)
    {
      g_object_unref (ps->adj[i]);
      g_object_unref (ps->entry[i]);
    }

  g_free (ps);
}


/* Allows only cubic cubes if the togglebutton is active */
static void
toggle_regular (GtkToggleButton * button, gpointer data)
{
  gint i;
  struct preferences_state *ps = data;

  if (gtk_toggle_button_get_active (button))
    {
      for (i = 0; i < 3; ++i)
	gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON (ps->entry[i]),
					GTK_ADJUSTMENT (ps->adj[0]));
      gtk_adjustment_value_changed (GTK_ADJUSTMENT (ps->adj[0]));
    }
  else
    {
      for (i = 0; i < 3; ++i)
	gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON (ps->entry[i]),
					GTK_ADJUSTMENT (ps->adj[i]));
    }
}

static struct preferences_state *
create_dimension_widget (GtkContainer * parent, const GbkGame * game)
{
  gint i;
  GtkWidget *label = gtk_label_new (_("Size of cube:"));

  GtkWidget *hbox = gtk_hbox_new (FALSE, BOX_PADDING);
  GtkWidget *vbox = gtk_vbox_new (TRUE, BOX_PADDING);
  GtkWidget *checkbox =
    gtk_check_button_new_with_mnemonic (_("Re_gular cube"));
  GtkWidget *vbox2 = gtk_vbox_new (TRUE, BOX_PADDING);

  struct preferences_state *ps = pref_state_create (GTK_BOX (vbox), game);

  gtk_widget_set_tooltip_text (vbox,
			       _("Sets the number of blocks in each side"));

  gtk_widget_set_tooltip_text (checkbox,
			       _
			       ("Allow only cubes with all sides equal size"));

  g_signal_connect (checkbox, "toggled", G_CALLBACK (toggle_regular), ps);

  gtk_box_pack_start (GTK_BOX (vbox2), label, TRUE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox2), checkbox, TRUE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, FALSE, 0);

  for (i = 0; i < 3; ++i)
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (ps->entry[i]),
			       gbk_cube_get_size (game->cube, i)),
      gtk_widget_show_all (hbox);

  gtk_container_add (parent, hbox);

  return ps;
}


void
about_dialog (GtkWidget * w, GtkWindow * toplevel)
{
  /* Create the widgets */
  GtkWidget *dialog = gtk_about_dialog_new ();

  gtk_window_set_icon_name (GTK_WINDOW (dialog), "gnubik");

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
				  "Copyright © 1998, 2003 John Darrington;\n 2004 John Darrington, Dale Mellor;\n2011 John Darrington");

  gtk_dialog_run (GTK_DIALOG (dialog));

  gtk_widget_destroy (GTK_WIDGET (dialog));
}






void
new_game_dialog (GtkWidget * w, GbkGame * game)
{
  gint response;

  GtkWidget *dialog = gtk_dialog_new_with_buttons (_("New Game"),
						   game->toplevel,
						   GTK_DIALOG_MODAL |
						   GTK_DIALOG_DESTROY_WITH_PARENT,
						   GTK_STOCK_OK,
						   GTK_RESPONSE_ACCEPT,
						   GTK_STOCK_CANCEL,
						   GTK_RESPONSE_CANCEL,
						   NULL);

  GtkWidget *vbox = gtk_vbox_new (FALSE, 5);

  GtkWidget *frame_dimensions = gtk_frame_new (_("Dimensions"));

  g_object_set (frame_dimensions, "shadow-type", GTK_SHADOW_NONE, NULL);

  gtk_window_set_icon_name (GTK_WINDOW (dialog), "gnubik");

  gtk_window_set_transient_for (GTK_WINDOW (dialog), game->toplevel);

  struct preferences_state *ps =
    create_dimension_widget (GTK_CONTAINER (frame_dimensions), game);

  GtkWidget *frame_pos = gtk_frame_new (_("Initial position"));
  GtkWidget *bb = gtk_vbutton_box_new ();

  GtkWidget *random_state =
    gtk_radio_button_new_with_mnemonic_from_widget (NULL, _("_Random"));
  GtkWidget *solved_state =
    gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON
						    (random_state),
						    _("_Solved"));

  g_object_set (frame_pos, "shadow-type", GTK_SHADOW_NONE, NULL);

  gtk_container_add (GTK_CONTAINER (bb), random_state);
  gtk_container_add (GTK_CONTAINER (bb), solved_state);

  gtk_container_add (GTK_CONTAINER (frame_pos), bb);

  gtk_box_pack_start (GTK_BOX (vbox), frame_pos, FALSE, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame_dimensions, FALSE, 0, 0);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), vbox);

  gtk_widget_show_all (GTK_DIALOG (dialog)->vbox);

  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (response == GTK_RESPONSE_ACCEPT)
    {
      start_new_game
	(game,
	 gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->entry[0])),
	 gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->entry[1])),
	 gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->entry[2])),
	 gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (random_state)));
    }

  pref_state_destroy (ps);

  gtk_widget_destroy (dialog);
}
