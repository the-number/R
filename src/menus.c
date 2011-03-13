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
#include <string.h>

#include "cube.h"
#include "menus.h"
#include "dialogs.h"
#include "guile-hooks.h"
#include "colour-dialog.h"
#include "drwBlock.h"
#include "glarea.h"

#include "game.h"

#include <gtk/gtk.h>

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)


static void
new_view (GbkGame * game, const gchar * description, const gfloat * aspect)
{
  gchar *title;
  GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  GtkWidget *view = gbk_cubeview_new (game->cube);

  /* Copy all the properties from the master view to the new one */
  {
    guint n, i;
    GParamSpec **specs =
      g_object_class_list_properties (G_OBJECT_GET_CLASS (view), &n);
    for (i = 0; i < n; ++i)
      {
	GValue value = { 0 };

	if (!(specs[i]->flags & G_PARAM_WRITABLE))
	  continue;

	if (!(specs[i]->flags & G_PARAM_READABLE))
	  continue;

	if (specs[i]->owner_type != G_OBJECT_TYPE (view))
	  continue;

	g_value_init (&value, specs[i]->value_type);
	g_object_get_property (G_OBJECT (game->masterview), specs[i]->name,
			       &value);
	g_object_set_property (G_OBJECT (view), specs[i]->name, &value);
	g_value_unset (&value);
      }
    g_free (specs);
  }

  gtk_window_set_icon_name (GTK_WINDOW (window), "gnubik");

  title = g_strdup_printf ("%s %s", PACKAGE, description);

  gtk_window_set_title (GTK_WINDOW (window), title);

  g_free (title);

  gtk_container_add (GTK_CONTAINER (window), view);

  g_object_set (view, "aspect", aspect, NULL);

  gbk_game_add_view (game, GBK_CUBEVIEW (view), FALSE);

  gtk_widget_show_all (window);
}

static void
view_rear (GtkAction * act, GbkGame * game)
{
  gfloat aspect[4] = { 180, 0, 1, 0 };
  new_view (game, _("Rear View"), aspect);
}


static void
view_bottom (GtkAction * act, GbkGame * game)
{
  gfloat aspect[4] = { 90, 1, 0, 0 };
  new_view (game, _("Bottom View"), aspect);
}

static void
view_top (GtkAction * act, GbkGame * game)
{
  gfloat aspect[4] = { -90, 1, 0, 0 };
  new_view (game, _("Top View"), aspect);
}


static void
view_left (GtkAction * act, GbkGame * game)
{
  gfloat aspect[4] = { -90, 0, 1, 0 };
  new_view (game, _("Left View"), aspect);
}

static void
view_right (GtkAction * act, GbkGame * game)
{
  gfloat aspect[4] = { 90, 0, 1, 0 };
  new_view (game, _("Right View"), aspect);
}



#define MSGLEN 100
static void
update_statusbar_mark (GbkGame * game, gint x, GtkStatusbar * statusbar)
{
  gchar mesg[MSGLEN];

  int context = gtk_statusbar_get_context_id (statusbar, "marks");

  g_snprintf (mesg, MSGLEN,
	      _("A mark is now set at position %d."), game->posn);

  gtk_statusbar_pop (statusbar, context);

  game->mesg_id = gtk_statusbar_push (statusbar, context, mesg);
}

static void
update_statusbar_moves (GbkGame * game, GtkStatusbar * statusbar)
{
  gchar mesg[MSGLEN];

  int context = gtk_statusbar_get_context_id (statusbar, "move-count");

  g_snprintf (mesg, MSGLEN, _("Moves: %d / %d"), game->posn, game->total);

  gtk_statusbar_pop (statusbar, context);

  game->mesg_id = gtk_statusbar_push (statusbar, context, mesg);



  /* Now check if the cube is solved and update statusbar accordingly */

  context = gtk_statusbar_get_context_id (statusbar, "cube-state");

  enum cube_status status = gbk_cube_get_status (game->cube);

  if (NOT_SOLVED == status)
    {
      gtk_statusbar_pop (statusbar, context);
    }
  else if (SOLVED == status)
    {
      g_snprintf (mesg, MSGLEN,
		  ngettext ("Cube solved in %d move",
			    "Cube solved in %d moves", game->posn),
		  game->posn);
      game->mesg_id = gtk_statusbar_push (statusbar, context, mesg);
    }
  else if (HALF_SOLVED == status)
    {
      g_snprintf (mesg,
		  MSGLEN,
		  _
		  ("Cube is NOT solved! The colours are correct,  but have incorrect orientation"));
      game->mesg_id = gtk_statusbar_push (statusbar, context, mesg);
    }
}

static void
update_statusbar_animation (GbkCubeview * view, GParamSpec * sp,
			    GtkStatusbar * statusbar)
{
  int context;

  gchar mesg[MSGLEN];
  int n;

  context = gtk_statusbar_get_context_id (statusbar, "animation");

  g_object_get (view, "animation-frames", &n, NULL);
  g_snprintf (mesg, MSGLEN,
	      ngettext ("Animation rate set to %d frame per turn.",
			"Animation rate set to %d frames per turn.", n), n);

  gtk_statusbar_pop (statusbar, context);
  gtk_statusbar_push (statusbar, context, mesg);
}


GtkWidget *
create_statusbar (GbkGame * game)
{
  GtkWidget *statusbar = gtk_statusbar_new ();

  g_signal_connect (game, "queue-changed",
		    G_CALLBACK (update_statusbar_moves), statusbar);

  g_signal_connect (game, "mark-set",
		    G_CALLBACK (update_statusbar_mark), statusbar);

  g_signal_connect (game->masterview, "notify::animation-frames",
		    G_CALLBACK (update_statusbar_animation), statusbar);

  return statusbar;
}


static const GtkActionEntry action_entries[] = {
  {"game-menu-action", NULL, N_("_Game")},
  {"view-menu-action", NULL, N_("_View")},
  {"add-view-menu-action", NULL, N_("Add _View"),
   NULL, N_("Add an auxiliary view of the cube")},


  {"help-menu-action", NULL, N_("_Help")},
  {"show-hide-menu-action", NULL, N_("Sho_w/Hide")},
  {"scripts-menu-action", NULL, N_("_Scripts")},

  {
   "about-action", GTK_STOCK_ABOUT, NULL,
   NULL, "about", G_CALLBACK (about_dialog)},

  {
   "quit-action", GTK_STOCK_QUIT, NULL,
   "<control>Q", "quit", G_CALLBACK (gtk_main_quit)}
};


static void
restart_game (GtkWidget * w, GbkGame * game)
{
  gbk_cube_scramble (game->cube);
  gbk_game_reset (game);
}

void
start_new_game (GbkGame * game, int size0, int size1, int size2,
		gboolean scramble)
{
  gbk_cube_set_size (game->cube, size0, size1, size2);

  if (scramble)
    gbk_cube_scramble (game->cube);

  gbk_game_reset (game);
}


static void
animate_faster (GtkWidget * w, GbkGame * game)
{
  GSList *v;
  int frames;

  g_object_get (game->masterview, "animation-frames", &frames, NULL);
  frames *= 2 / 3.0;

  /* Iterate over all the views and set the new properties */
  for (v = game->views; v != NULL; v = g_slist_next (v))
    {
      g_object_set (v->data, "animation-frames", frames, NULL);
    }
}

static void
animate_slower (GtkWidget * w, GbkGame * game)
{
  int frames;
  GSList *v;
  g_object_get (game->masterview, "animation-frames", &frames, NULL);
  frames /= 2 / 3.0;
  if (frames == 0)
    frames = 1;

  if (frames == 1)
    frames = 2;

  /* Iterate over all the views and set the new properties */
  for (v = game->views; v != NULL; v = g_slist_next (v))
    {
      g_object_set (v->data, "animation-frames", frames, NULL);
    }
}

static const GtkActionEntry game_action_entries[] = {
  {
   "restart-game-action", NULL, N_("_Restart Game"),
   NULL, "restart-game", G_CALLBACK (restart_game)},

  {
   "new-game-action", GTK_STOCK_NEW, N_("_New Game"),
   "<control>N", "new-game", G_CALLBACK (new_game_dialog)},


  {"add-view-rear-action", NULL, N_("_Rear"), NULL, NULL,
   G_CALLBACK (view_rear)},
  {"add-view-left-action", NULL, N_("_Left"), NULL, NULL,
   G_CALLBACK (view_left)},
  {"add-view-right-action", NULL, N_("Ri_ght"), NULL, NULL,
   G_CALLBACK (view_right)},
  {"add-view-top-action", NULL, N_("_Top"), NULL, NULL,
   G_CALLBACK (view_top)},
  {"add-view-bottom-action", NULL, N_("_Bottom"), NULL, NULL,
   G_CALLBACK (view_bottom)},


  {
   "colours-action", GTK_STOCK_SELECT_COLOR, N_("_Colours"),
   NULL, "colours", G_CALLBACK (colour_select_menu)},

  {
   "animation-action", NULL, N_("_Animation"),
   NULL, "animation", NULL},

  {
   "animate-faster-action", NULL, N_("_Faster"),
   "<control>plus", "animate-faster", G_CALLBACK (animate_faster)},

  {
   "animate-slower-action", NULL, N_("_Slower"),
   "<control>minus", "animate-slower", G_CALLBACK (animate_slower)},

};


static const char menu_tree[] = "<ui>\
  <menubar name=\"MainMenu\">\
    <menu name=\"game-menu\" action=\"game-menu-action\">\
     <menuitem name=\"restart-game\" action=\"restart-game-action\"/> \
     <menuitem name=\"new-game\" action=\"new-game-action\"/> \
     <menuitem name=\"quit\" action=\"quit-action\"/>\
    </menu>\
    <menu name=\"view-menu\" action=\"view-menu-action\"> \
     <menuitem name=\"colours\" action=\"colours-action\"/> \
     <menu name=\"add view\"   action=\"add-view-menu-action\"> \
      <menuitem name=\"rear\"  action=\"add-view-rear-action\"/> \
      <menuitem name=\"left\"  action=\"add-view-left-action\"/> \
      <menuitem name=\"right\" action=\"add-view-right-action\"/> \
      <menuitem name=\"top\"   action=\"add-view-top-action\"/> \
      <menuitem name=\"bottom\" action=\"add-view-bottom-action\"/> \
     </menu> \
     <menu name=\"animation-menu\" action=\"animation-action\"> \
       <menuitem name=\"faster\"   action=\"animate-faster-action\"/>	\
       <menuitem name=\"slower\"   action=\"animate-slower-action\"/>	\
     </menu> \
    </menu> \
    <menu name=\"scripts-menu\" action=\"scripts-menu-action\">\
    </menu> \
    <menu name=\"help-menu\" action=\"help-menu-action\">\
     <menuitem name=\"about\" action=\"about-action\"/>\
    </menu>\
  </menubar>\
</ui>";


GtkWidget *
create_menubar (GbkGame * game)
{
  GtkWidget *menubar;
  GtkUIManager *menu_manager = gtk_ui_manager_new ();
  GtkActionGroup *action_group = gtk_action_group_new ("dialog-actions");
  GtkActionGroup *game_action_group = gtk_action_group_new ("game-actions");

  gtk_action_group_set_translation_domain (action_group, PACKAGE);
  gtk_action_group_set_translation_domain (game_action_group, PACKAGE);

  gtk_action_group_add_actions (action_group, action_entries,
				sizeof (action_entries) /
				sizeof (action_entries[0]), game->toplevel);


  gtk_action_group_add_actions (game_action_group, game_action_entries,
				sizeof (game_action_entries) /
				sizeof (game_action_entries[0]), game);


  gtk_ui_manager_insert_action_group (menu_manager, action_group, 0);
  gtk_ui_manager_insert_action_group (menu_manager, game_action_group, 0);

  if (0 ==
      gtk_ui_manager_add_ui_from_string (menu_manager, menu_tree,
					 strlen (menu_tree), NULL))
    g_return_val_if_reached (NULL);

  startup_guile_scripts (menu_manager);

  menubar = gtk_ui_manager_get_widget (menu_manager, "/ui/MainMenu");

  gtk_window_add_accel_group (GTK_WINDOW (game->toplevel),
			      gtk_ui_manager_get_accel_group (menu_manager));

  gtk_widget_show (menubar);

  return menubar;
}

enum
{
  ACT_REWIND = 0,
  ACT_PREV,
  ACT_STOP,
  ACT_MARK,
  ACT_NEXT,
  ACT_PLAY,
  n_ACTS
};


static void
set_playbar_sensitivities (GbkGame * g, GtkAction ** acts)
{
  gboolean play_state = (g->mode == MODE_PLAY);

  gtk_action_set_sensitive (acts[ACT_REWIND], !play_state
			    && !gbk_game_at_start (g));
  gtk_action_set_sensitive (acts[ACT_PREV], !play_state
			    && !gbk_game_at_start (g));

  gtk_action_set_sensitive (acts[ACT_PLAY], !play_state
			    && !gbk_game_at_end (g));
  gtk_action_set_sensitive (acts[ACT_NEXT], !play_state
			    && !gbk_game_at_end (g));

  gtk_action_set_sensitive (acts[ACT_STOP], play_state);
}

GtkWidget *
create_play_toolbar (GbkGame * game)
{
  int i;
  static GtkAction *acts[n_ACTS];

  acts[ACT_REWIND] =
    gtk_action_new ("rewind",
		    _("Rewind"),
		    _
		    ("Go to the previous mark (or the beginning) of the sequence of moves"),
		    GTK_STOCK_MEDIA_REWIND);


  acts[ACT_PREV] =
    gtk_action_new ("previous",
		    _("Back"),
		    _("Make one step backwards"), GTK_STOCK_MEDIA_PREVIOUS);


  acts[ACT_STOP] =
    gtk_action_new ("stop",
		    _("Stop"),
		    _("Stop running the sequence of moves"),
		    GTK_STOCK_MEDIA_STOP);


  acts[ACT_MARK] =
    gtk_action_new ("mark",
		    _("Mark"),
		    _("Mark the current place in the sequence of moves"),
		    GTK_STOCK_MEDIA_STOP);


  acts[ACT_NEXT] =
    gtk_action_new ("next",
		    _("Forward"),
		    _("Make one step forwards"), GTK_STOCK_MEDIA_NEXT);


  acts[ACT_PLAY] =
    gtk_action_new ("forward",
		    _("Play"),
		    _("Run forward through the sequence of moves"),
		    GTK_STOCK_MEDIA_PLAY);


  GtkWidget *play_toolbar = gtk_toolbar_new ();

  set_playbar_sensitivities (game, acts);

  g_signal_connect (game, "queue-changed",
		    G_CALLBACK (set_playbar_sensitivities), acts);

  gtk_toolbar_set_style (GTK_TOOLBAR (play_toolbar), GTK_TOOLBAR_BOTH);


  for (i = 0; i < n_ACTS; ++i)
    {
      gtk_toolbar_insert (GTK_TOOLBAR (play_toolbar),
			  GTK_TOOL_ITEM (gtk_action_create_tool_item
					 (acts[i])), -1);
    }

  g_signal_connect_swapped (acts[ACT_REWIND], "activate",
			    G_CALLBACK (gbk_game_rewind), game);

  g_signal_connect_swapped (acts[ACT_STOP], "activate",
			    G_CALLBACK (gbk_game_stop_replay), game);

  g_signal_connect_swapped (acts[ACT_NEXT], "activate",
			    G_CALLBACK (gbk_game_next_move), game);

  g_signal_connect_swapped (acts[ACT_PREV], "activate",
			    G_CALLBACK (gbk_game_prev_move), game);

  g_signal_connect_swapped (acts[ACT_MARK], "activate",
			    G_CALLBACK (gbk_game_set_mark), game);


  g_signal_connect_swapped (acts[ACT_PLAY], "activate",
			    G_CALLBACK (gbk_game_replay), game);

  gtk_widget_show_all (play_toolbar);

  return play_toolbar;
}


/* Popup an error dialog box */
void
error_dialog (GtkWindow * parent, const gchar * format, ...)
{
  va_list ap;
  GtkWidget *dialog;
  gchar *message;

  va_start (ap, format);

  message = g_strdup_vprintf (format, ap);

  va_end (ap);

  dialog = gtk_message_dialog_new (parent,
				   GTK_DIALOG_MODAL,
				   GTK_MESSAGE_ERROR,
				   GTK_BUTTONS_CLOSE, message);
  g_free (message);

  gtk_window_set_title (GTK_WINDOW (dialog), _("Gnubik error"));

  gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);

  gtk_dialog_run (GTK_DIALOG (dialog));

  gtk_widget_destroy (dialog);
}
