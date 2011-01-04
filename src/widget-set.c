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

#include "widget-set.h"
#include "ui.h"
#include "dialogs.h"
#include "guile-hooks.h"
#include "colour-dialog.h"
#include "glarea.h"
#include "drwBlock.h"

#include <gtk/gtk.h>

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)



#define MSGLEN 100
void
update_statusbar (void)
{
#if WIDGETS_NOT_DISABLED

  static int context = 0;

  static guint mesg_id = 0;

  gchar mesg[MSGLEN];

  if (0 == context)
    context = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar),
					    "move-count");

  g_snprintf (mesg,
	      MSGLEN,
	      _("Moves: %d / %d"),
	      move_queue_progress (move_queue).current,
	      move_queue_progress (move_queue).total);

  if (mesg_id != 0)
    gtk_statusbar_remove (GTK_STATUSBAR (statusbar), context, mesg_id);

  mesg_id = gtk_statusbar_push (GTK_STATUSBAR (statusbar), context, mesg);
#endif
}

void
declare_win (const struct cube *cube)
{
#if WIDGETS_NOT_DISABLED
  static int context = 0;

  static guint mesg_id = 0;

  gchar mesg[MSGLEN];

  enum Cube_Status status = cube_get_status (cube);

  if (0 == context)
    context = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "win");

  switch (status)
    {
    case SOLVED:
      g_snprintf (mesg, MSGLEN, ngettext ("Cube solved in %d move",
					  "Cube solved in %d moves",
					  move_queue_progress (move_queue).
					  total),
		  move_queue_progress (move_queue).total);
      break;
    case HALF_SOLVED:
      g_snprintf (mesg,
		  MSGLEN,
		  _
		  ("Cube is NOT solved! The colours are correct,  but have incorrect orientation"));
      break;
    default:
      g_assert_not_reached ();
    }

  mesg_id = gtk_statusbar_push (GTK_STATUSBAR (statusbar), context, mesg);
#endif
}

#if WIDGETS_NOT_DISABLED
GtkWidget *
create_statusbar (GtkWidget * container)
{
  statusbar = gtk_statusbar_new ();

  gtk_box_pack_start (GTK_BOX (container), statusbar, FALSE, TRUE, 0);

  gtk_widget_show (statusbar);

  return statusbar;
}


static GList *play_button_list;

#endif

void
set_toolbar_state (unsigned flags)
{
#if WIDGETS_NOT_DISABLED
  GList *list;

  for (list = play_button_list; list != NULL; list = list->next)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (list->data), flags & 1);
      flags >>= 1;
    }
#endif
}

#if WIDGETS_NOT_DISABLED

static GtkWidget *play_toolbar;

GtkWidget *
create_play_toolbar (GtkWidget * container, GtkWidget * toplevel)
{
  GtkAction *rewind = gtk_action_new ("rewind",
				      _("Rewind"),
				      _
				      ("Go to the previous mark (or the beginning) of the sequence of moves"),
				      GTK_STOCK_MEDIA_REWIND);

  GtkAction *previous = gtk_action_new ("previous",
					_("Back"),
					_("Make one step backwards"),
					GTK_STOCK_MEDIA_PREVIOUS);


  GtkAction *stop = gtk_action_new ("stop",
				    _("Stop"),
				    _("Stop running the sequence of moves"),
				    GTK_STOCK_MEDIA_STOP);


  GtkAction *mark = gtk_action_new ("mark",
				    _("Mark"),
				    _("Mark the current place in the sequence of moves"),
				    GTK_STOCK_MEDIA_STOP);


  GtkAction *next = gtk_action_new ("next",
				    _("Forward"),
				    _("Make one step forwards"),
				    GTK_STOCK_MEDIA_NEXT);


  GtkAction *play = gtk_action_new ("forward",
				    _("Play"),
				    _("Run forward through the sequence of moves"),
				    GTK_STOCK_MEDIA_PLAY);


  play_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (play_toolbar), GTK_TOOLBAR_BOTH);
  gtk_container_set_border_width (GTK_CONTAINER (play_toolbar), 0);
  gtk_box_pack_start (GTK_BOX (container), play_toolbar, FALSE, TRUE, 0);


  gtk_toolbar_insert (GTK_TOOLBAR (play_toolbar),
		      GTK_TOOL_ITEM (gtk_action_create_tool_item (rewind)),
		      -1);

  gtk_toolbar_insert (GTK_TOOLBAR (play_toolbar),
		      GTK_TOOL_ITEM (gtk_action_create_tool_item (previous)),
		      -1);

  gtk_toolbar_insert (GTK_TOOLBAR (play_toolbar),
		      GTK_TOOL_ITEM (gtk_action_create_tool_item (stop)), -1);

  gtk_toolbar_insert (GTK_TOOLBAR (play_toolbar),
		      GTK_TOOL_ITEM (gtk_action_create_tool_item (mark)), -1);

  gtk_toolbar_insert (GTK_TOOLBAR (play_toolbar),
		      GTK_TOOL_ITEM (gtk_action_create_tool_item (next)), -1);

  gtk_toolbar_insert (GTK_TOOLBAR (play_toolbar),
		      GTK_TOOL_ITEM (gtk_action_create_tool_item (play)), -1);

  g_signal_connect_swapped (rewind, "activate",
			    G_CALLBACK (request_queue_rewind), container);

  g_signal_connect_swapped (previous, "activate",
			    G_CALLBACK (request_back), container);

  g_signal_connect_swapped (stop, "activate",
			    G_CALLBACK (request_stop), container);

  g_signal_connect_swapped (mark, "activate",
			    G_CALLBACK (request_mark_move_queue), container);

  g_signal_connect_swapped (next, "activate",
			    G_CALLBACK (request_forward), container);

  g_signal_connect_swapped (play, "activate",
			    G_CALLBACK (request_play), container);

  gtk_widget_show_all (play_toolbar);

  play_button_list
    = gtk_container_get_children (GTK_CONTAINER (play_toolbar));

  set_toolbar_state (0);

  return play_toolbar;
}


/* Toggle the visibility of a widget */
static void
toggle_visibility (GtkToggleAction *ta, gpointer user_data)
{
  GObject **w = user_data;

  g_object_set (*w, 
		"visible", gtk_toggle_action_get_active (ta),
		NULL);
}
#endif



static const GtkActionEntry action_entries[] =
{
  {"game-menu-action", NULL, N_("_Game")},
  {"settings-menu-action", NULL, N_("_Settings")},
  {"help-menu-action", NULL, N_("_Help")},
  {"show-hide-menu-action", NULL, N_("Sho_w/Hide")},
  {"scripts-menu-action", NULL, N_("_Scripts")},

#if 0
  {
   "preferences-action", GTK_STOCK_PREFERENCES, NULL,
   NULL, "preferences", G_CALLBACK (preferences_dialog)
  },
#endif

  {
   "colours-action", GTK_STOCK_SELECT_COLOR, N_("_Colours"),
   NULL, "colours", G_CALLBACK (colour_select_menu)
  },

  {
   "about-action", GTK_STOCK_ABOUT, NULL,
   NULL, "about", G_CALLBACK (about_dialog)
  },

  {
   "quit-action", GTK_STOCK_QUIT, NULL,
   "<control>Q", "quit", G_CALLBACK (gtk_main_quit)
  }
};


static void
restart_game ()
{
  int size[3];
  size[0] = cube_get_size(the_cube, 0);
  size[1] = cube_get_size(the_cube, 1);
  size[2] = cube_get_size(the_cube, 2);

  free_cube (the_cube);
  the_cube = new_cube (size[0], size[1], size[2]);
  cube_scramble (the_cube);
  scene_init ();
}

void
start_new_game (int size0, int size1, int size2)
{
  free_cube (the_cube);

  the_cube = new_cube (size0, size1, size2);

  cube_scramble (the_cube);
  scene_init ();
}


static const GtkActionEntry game_action_entries[] =
  {
    {
   "restart-game-action", NULL, N_("_Restart Game"),
   NULL, "restart-game", G_CALLBACK (restart_game)
    },

    {
   "new-game-action", GTK_STOCK_NEW, N_("_New Game"),
   "<control>N", "new-game", G_CALLBACK (new_game_dialog)
    }
  };

#if 0

static const GtkToggleActionEntry statusbar_action_entries[] =
  {
    {
   "statusbar-action", NULL, N_("_Status Bar"),
   NULL, "show-hide-statusbar", G_CALLBACK (toggle_visibility), TRUE
    },
  };


static const GtkToggleActionEntry toolbar_action_entries[] =
  {
    {
   "toolbar-action", NULL, N_("_Play Toolbar"),
   NULL, "show-hide-toolbar", G_CALLBACK (toggle_visibility), TRUE
    }
  };


#endif

static const char menu_tree[] = "<ui>\
  <menubar name=\"MainMenu\">\
    <menu name=\"game-menu\" action=\"game-menu-action\">\
     <menuitem name=\"restart-game\" action=\"restart-game-action\"/> \
     <menuitem name=\"new-game\" action=\"new-game-action\"/> \
     <menuitem name=\"quit\" action=\"quit-action\"/>\
    </menu>\
    <menu name=\"settings-menu\" action=\"settings-menu-action\">"
  /*
     <menuitem name=\"preferences\" action=\"preferences-action\"/>\
  */
"\
     <menuitem name=\"colours\" action=\"colours-action\"/>\
"\
  /*
     <menu name=\"show-hide-menu\" action=\"show-hide-menu-action\">\
     <menuitem name=\"toggle-toolbar\" action=\"toolbar-action\"/>\
     <menuitem name=\"toggle-statusbar\" action=\"statusbar-action\"/>\
     </menu>\
    </menu>\
    <menu name=\"scripts-menu\" action=\"scripts-menu-action\">\
  */
"\
    </menu>\
    <menu name=\"help-menu\" action=\"help-menu-action\">\
     <menuitem name=\"about\" action=\"about-action\"/>\
    </menu>\
  </menubar>\
</ui>";


GtkWidget *
create_menubar (GtkWidget *toplevel)
{
  GtkWidget *menubar;
  GtkUIManager *menu_manager = gtk_ui_manager_new ();
  GtkActionGroup *action_group = gtk_action_group_new ("dialog-actions");
  GtkActionGroup *game_action_group = gtk_action_group_new ("game-actions");
  GtkActionGroup *toolbar_action_group = gtk_action_group_new ("toolbar-actions");
  GtkActionGroup *statusbar_action_group = gtk_action_group_new ("statusbar-actions");

  gtk_action_group_set_translation_domain (action_group, PACKAGE);
  gtk_action_group_set_translation_domain (game_action_group, PACKAGE);
  gtk_action_group_set_translation_domain (toolbar_action_group, PACKAGE);
  gtk_action_group_set_translation_domain (statusbar_action_group, PACKAGE);

  gtk_action_group_add_actions (action_group, action_entries,
				sizeof (action_entries) /
				sizeof (action_entries[0]), toplevel);


  gtk_action_group_add_actions (game_action_group, game_action_entries,
				sizeof (game_action_entries) /
				sizeof (game_action_entries[0]), toplevel);

#if 0
  gtk_action_group_add_toggle_actions (toolbar_action_group, toolbar_action_entries,
				sizeof (toolbar_action_entries) /
				sizeof (toolbar_action_entries[0]), &play_toolbar);

  gtk_action_group_add_toggle_actions (statusbar_action_group, statusbar_action_entries,
				sizeof (statusbar_action_entries) /
				sizeof (statusbar_action_entries[0]), &statusbar);
#endif

  gtk_ui_manager_insert_action_group (menu_manager, action_group, 0);
  gtk_ui_manager_insert_action_group (menu_manager, game_action_group, 0);

#if 0
  gtk_ui_manager_insert_action_group (menu_manager, toolbar_action_group, 0);
  gtk_ui_manager_insert_action_group (menu_manager, statusbar_action_group, 0);
#endif

  if (0 ==
      gtk_ui_manager_add_ui_from_string (menu_manager, menu_tree,
					 strlen (menu_tree), NULL))
    g_return_val_if_reached (NULL);

#if 0
  startup_guile_scripts (menu_manager);
#endif

  menubar = gtk_ui_manager_get_widget (menu_manager, "/ui/MainMenu");

  gtk_window_add_accel_group (GTK_WINDOW (toplevel), gtk_ui_manager_get_accel_group (menu_manager));

  gtk_widget_show (menubar);

  return menubar;
}


GtkWidget *
create_play_toolbar (GtkWidget *toplevel)
{
  GtkAction *rewind =
    gtk_action_new ("rewind",
		    _("Rewind"),
		    _("Go to the previous mark (or the beginning) of the sequence of moves"),
		    GTK_STOCK_MEDIA_REWIND);

  GtkAction *previous =
    gtk_action_new ("previous",
		    _("Back"),
		    _("Make one step backwards"),
		    GTK_STOCK_MEDIA_PREVIOUS);


  GtkAction *stop =
    gtk_action_new ("stop",
		    _("Stop"),
		    _("Stop running the sequence of moves"),
		    GTK_STOCK_MEDIA_STOP);


  GtkAction *mark =
    gtk_action_new ("mark",
		    _("Mark"),
		    _("Mark the current place in the sequence of moves"),
		    GTK_STOCK_MEDIA_STOP);


  GtkAction *next =
    gtk_action_new ("next",
		    _("Forward"),
		    _("Make one step forwards"),
		    GTK_STOCK_MEDIA_NEXT);


  GtkAction *play =
    gtk_action_new ("forward",
		    _("Play"),
		    _("Run forward through the sequence of moves"),
		    GTK_STOCK_MEDIA_PLAY);


  GtkWidget *play_toolbar = gtk_toolbar_new ();

  gtk_toolbar_set_style (GTK_TOOLBAR (play_toolbar),  GTK_TOOLBAR_BOTH);


  gtk_toolbar_insert (GTK_TOOLBAR (play_toolbar),
		      GTK_TOOL_ITEM (gtk_action_create_tool_item (rewind)),
		      -1);

  gtk_toolbar_insert (GTK_TOOLBAR (play_toolbar),
		      GTK_TOOL_ITEM (gtk_action_create_tool_item (previous)),
		      -1);

  gtk_toolbar_insert (GTK_TOOLBAR (play_toolbar),
		      GTK_TOOL_ITEM (gtk_action_create_tool_item (stop)),
		      -1);

  gtk_toolbar_insert (GTK_TOOLBAR (play_toolbar),
		      GTK_TOOL_ITEM (gtk_action_create_tool_item (mark)),
		      -1);

  gtk_toolbar_insert (GTK_TOOLBAR (play_toolbar),
		      GTK_TOOL_ITEM (gtk_action_create_tool_item (next)),
		      -1);

  gtk_toolbar_insert (GTK_TOOLBAR (play_toolbar),
		      GTK_TOOL_ITEM (gtk_action_create_tool_item (play)),
		      -1);
#if 0
  g_signal_connect_swapped (rewind, "activate",
			    G_CALLBACK (request_queue_rewind), container);

  g_signal_connect_swapped (previous, "activate",
			    G_CALLBACK (request_back), container);

  g_signal_connect_swapped (stop, "activate",
			    G_CALLBACK (request_stop), container);

  g_signal_connect_swapped (mark, "activate",
			    G_CALLBACK (request_mark_move_queue), container);

  g_signal_connect_swapped (next, "activate",
			    G_CALLBACK (request_forward), container);

  g_signal_connect_swapped (play, "activate",
			    G_CALLBACK (request_play), container);

  play_button_list
    = gtk_container_get_children (GTK_CONTAINER (play_toolbar));

  set_toolbar_state (0);

#endif

  gtk_widget_show_all (play_toolbar);

  return play_toolbar;
}


/* Popup an error dialog box */
void
error_dialog (GtkWindow *parent, const gchar *format, ...)
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


