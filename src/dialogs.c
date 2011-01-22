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
#include "ui.h"
#include "drwBlock.h"
#include "widget-set.h"

#include <glib.h>
#include <assert.h>

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)

#define BOX_PADDING 10

#if WIDGETS_NOT_DISABLED

extern int frameQty;

static int new_frameQty;

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

#endif

struct preferences_state
{
  GtkObject *adj[3];
  GtkWidget *entry[3];
};

static struct preferences_state *
pref_state_create (GtkBox *parent)
{
  gint i;
  struct preferences_state *ps = g_malloc (sizeof (*ps));

  for (i = 0; i < 3 ; ++i)
    {
      ps->adj[i] = gtk_adjustment_new (gbk_cube_get_size (the_cube, 0), 1, G_MAXFLOAT, 1, 1, 0);
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
  for (i = 0; i < 3 ; ++i)
    {
      g_object_unref (ps->adj[i]);
      g_object_unref (ps->entry[i]);
    }

  g_free (ps);
}


/* Allows only cubic cubes if the togglebutton is active */
static void
toggle_regular (GtkToggleButton *button, gpointer data)
{
  gint i;
  struct preferences_state * ps = data;

  if ( gtk_toggle_button_get_active (button))
    {
      for (i = 0; i < 3 ; ++i)
	gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON (ps->entry[i]),
					GTK_ADJUSTMENT (ps->adj[0]));
      gtk_adjustment_value_changed (GTK_ADJUSTMENT (ps->adj[0]));
    }
  else
    {
      for (i = 0; i < 3 ; ++i)
	gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON (ps->entry[i]),
					GTK_ADJUSTMENT (ps->adj[i]));
    }
}

static struct preferences_state *
create_dimension_widget (GtkContainer *parent)
{
  gint i;
  GtkWidget *label = gtk_label_new (_("Size of cube:"));

  GtkWidget *hbox = gtk_hbox_new (FALSE, BOX_PADDING);
  GtkWidget *vbox = gtk_vbox_new (TRUE, BOX_PADDING);
  GtkWidget *checkbox = gtk_check_button_new_with_label (_("Regular cube"));
  GtkWidget *vbox2 = gtk_vbox_new (TRUE, BOX_PADDING);

  struct preferences_state *ps  = pref_state_create (GTK_BOX (vbox));

  gtk_widget_set_tooltip_text (vbox,
			       _("Sets the number of blocks in each side"));

  gtk_widget_set_tooltip_text (checkbox,
			       _("Allow only cubes with all sides equal size"));

  g_signal_connect (checkbox, "toggled", G_CALLBACK (toggle_regular), ps);

  gtk_box_pack_start (GTK_BOX (vbox2), label, TRUE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox2), checkbox, TRUE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, FALSE, 0);

  for (i = 0; i < 3; ++i)
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (ps->entry[i]),
			       gbk_cube_get_size (the_cube, i)),

  gtk_widget_show_all (hbox);

  gtk_container_add (parent, hbox);

  return ps;
}


#if 0


static GtkWidget *
create_animation_widget (void)
{
  GtkWidget *label2;
  GtkWidget *scrollbar;

  GtkWidget *vboxOuter = gtk_vbox_new (TRUE, BOX_PADDING);
  GtkWidget *vbox = gtk_vbox_new (FALSE, BOX_PADDING);

  GtkObject *adjustment = gtk_adjustment_new (frameQty, 0, 99, 1, 1, 0);

  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
  GtkWidget *label1 = gtk_label_new (_("Faster"));
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);

  label2 = gtk_label_new (_("Slower"));
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_RIGHT);

  scrollbar = gtk_hscrollbar_new (GTK_ADJUSTMENT (adjustment));
  gtk_widget_set_tooltip_text (scrollbar,
			       _("Controls the speed with which slices rotate"));

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
preferences_dialog (GtkWidget *w, GtkWindow *toplevel)
{
  gint response;

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

  struct preferences_state *ps = create_dimension_widget (GTK_CONTAINER (frame_dimensions));

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
      gboolean new_size = FALSE;
      int i;
      if (new_values)
	frameQty = new_frameQty;
      
      for (i = 0; i < 3; ++i)
	{
	  if ( gbk_cube_get_size (the_cube, i) != 
	       gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->entry[i])))
	    {
	      new_size = TRUE;
	      break;
	    }
	}

      if (new_size && confirm_preferences (toplevel))
	{
	  request_new_game (NULL, ps);
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

#endif

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
				  "Copyright © 1998, 2003 John Darrington;\n 2004 John Darrington, Dale Mellor;\n2011 John Darrington");

  gtk_dialog_run (GTK_DIALOG (dialog));

  gtk_widget_destroy (GTK_WIDGET (dialog));
}






void
new_game_dialog (GtkWidget *w, GtkWindow *toplevel)
{
  gint response;

  GtkWidget *dialog = gtk_dialog_new_with_buttons (_("New Game"),
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

  gtk_window_set_icon_name (GTK_WINDOW(dialog), "gnubik");

  gtk_window_set_transient_for (GTK_WINDOW (dialog), toplevel);

  struct preferences_state *ps = create_dimension_widget (GTK_CONTAINER (frame_dimensions));

  gtk_box_pack_start (GTK_BOX (vbox), frame_dimensions, FALSE, 0, 0);

  gtk_widget_show_all (vbox);

  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (response == GTK_RESPONSE_ACCEPT)
    {
      start_new_game 
	(
	 gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->entry[0])),
	 gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->entry[1])),
	 gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->entry[2]))
	);
    }

  pref_state_destroy (ps);
  
  gtk_widget_destroy (dialog);
}


