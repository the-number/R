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
#include "widget-set.h"
#include "colour-sel.h"
#include "version.h"
#include "move-queue.h"
#include "ui.h"
#include "dialogs.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <gtk/gtkgl.h>

#include "drwBlock.h"

#include "guile-hooks.h"

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)



void
widget_set_init (int *argc, char ***argv)
{

  gtk_init (argc, argv);

  gtk_gl_init (argc, argv);

}


GtkWidget *
create_top_level_widget (void)
{
  /* create a new window */
  GtkWidget *win = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (win, "delete-event", G_CALLBACK (gtk_main_quit), 0);


  return win;
}

GtkWidget *
create_container_widget (GtkWidget * parent)
{
  /* create a vbox to hold the drawing area and the menubar */
  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);


  gtk_container_add (GTK_CONTAINER (parent), vbox);

  gtk_widget_show (vbox);

  return vbox;
}



static GtkWidget *statusbar;
extern Move_Queue *move_queue;
#define MSGLEN 100
void
update_statusbar (void)
{

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

}

void
declare_win (const struct cube *cube)
{
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
}

GtkWidget *
create_statusbar (GtkWidget * container)
{


  statusbar = gtk_statusbar_new ();

  gtk_box_pack_start (GTK_BOX (container), statusbar, FALSE, TRUE, 0);

  gtk_widget_show (statusbar);

  return statusbar;

}



static GList *play_button_list;

void
set_toolbar_state (unsigned flags)
{
  GList *list;

  for (list = play_button_list; list != NULL; list = list->next)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (list->data), flags & 1);
      flags >>= 1;
    }
}


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
				    _
				    ("Mark the current place in the sequence of moves"),
				    GTK_STOCK_MEDIA_STOP);


  GtkAction *next = gtk_action_new ("next",
				    _("Forward"),
				    _("Make one step forwards"),
				    GTK_STOCK_MEDIA_NEXT);


  GtkAction *play = gtk_action_new ("forward",
				    _("Play"),
				    _
				    ("Run forward through the sequence of moves"),
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
toggle_visibility (GtkMenuItem * menuitem, gpointer user_data)
{
  GtkWidget **w = (user_data);

  g_assert (*w);

  if (GTK_WIDGET_VISIBLE (*w))
    gtk_widget_hide (*w);
  else
    gtk_widget_show (*w);
}

static GtkWidget *
create_show_hide_menu (void)
{
  GtkWidget *menu;
  GtkWidget *menuitem;

  menu = gtk_menu_new ();

  menuitem = gtk_menu_item_new_with_mnemonic (_("_Play Toolbar"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
  gtk_widget_show (menuitem);

  g_signal_connect (menuitem, "activate",
		    G_CALLBACK (toggle_visibility), &play_toolbar);

  menuitem = gtk_menu_item_new_with_mnemonic (_("_Status Bar"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
  gtk_widget_show (menuitem);

  g_signal_connect (menuitem, "activate",
		    G_CALLBACK (toggle_visibility), &statusbar);



  return menu;
}


static const GtkActionEntry action_entries[] = {
  {"game-menu-action", NULL, N_("_Game")},
  {"settings-menu-action", NULL, N_("_Settings")},
  {"help-menu-action", NULL, N_("_Help")},
  {"scripts-menu-action", NULL, N_("_Scripts")},


  {
   "preferences-action", GTK_STOCK_PREFERENCES, NULL,
   NULL, "preferences", G_CALLBACK (preferences_dialog)},

  {
   "colours-action", GTK_STOCK_SELECT_COLOR, N_("_Colours"),
   NULL, "colours", G_CALLBACK (colour_select_menu)},

  {
   "show-hide-action", NULL, N_("Sho_w/Hide"),
   NULL, "show-hide", G_CALLBACK (create_show_hide_menu)},

  {
   "about-action", GTK_STOCK_ABOUT, NULL,
   NULL, "about", G_CALLBACK (about_dialog)},

  {
   "quit-action", GTK_STOCK_QUIT, NULL,
   "<control>Q", "quit", G_CALLBACK (gtk_main_quit)},

  {
   "new-game-action", NULL, N_("_New Game"),
   "<control>N", "new-game", G_CALLBACK (request_new_game)}

};


static const char menu_tree[] = "<ui>\
  <menubar name=\"MainMenu\">\
    <menu name=\"game-menu\" action=\"game-menu-action\">\
     <menuitem name=\"new-game\" action=\"new-game-action\"/>\
     <menuitem name=\"quit\" action=\"quit-action\"/>\
    </menu>\
    <menu name=\"settings-menu\" action=\"settings-menu-action\">\
     <menuitem name=\"preferences\" action=\"preferences-action\"/>\
     <menuitem name=\"colours\" action=\"colours-action\"/>\
     <menuitem name=\"show-hide\" action=\"show-hide-action\"/>\
    </menu>\
    <menu name=\"scripts-menu\" action=\"scripts-menu-action\">\
    </menu>\
    <menu name=\"help-menu\" action=\"help-menu-action\">\
     <menuitem name=\"about\" action=\"about-action\"/>\
    </menu>\
  </menubar>\
</ui>";

GtkWidget *
create_menubar (GtkWidget * container, GtkWidget * toplevel)
{
  GtkWidget *menubar;
  GtkUIManager *menu_manager = gtk_ui_manager_new ();
  GtkActionGroup *action_group = gtk_action_group_new ("menu-actions");

  gtk_action_group_set_translation_domain (action_group, PACKAGE);

  gtk_action_group_add_actions (action_group, action_entries,
				sizeof (action_entries) /
				sizeof (action_entries[0]), toplevel);

  gtk_ui_manager_insert_action_group (menu_manager, action_group, 0);

  if (0 ==
      gtk_ui_manager_add_ui_from_string (menu_manager, menu_tree,
					 strlen (menu_tree), NULL))
    g_return_val_if_reached (NULL);

  startup_guile_scripts (menu_manager);

  menubar = gtk_ui_manager_get_widget (menu_manager, "/ui/MainMenu");

  gtk_window_add_accel_group (GTK_WINDOW (toplevel), gtk_ui_manager_get_accel_group (menu_manager));

  gtk_box_pack_start (GTK_BOX (container), menubar, FALSE, TRUE, 0);

  gtk_widget_show (menubar);

  return menubar;
}

/* Popup an error dialog box */
void
error_dialog (GtkWidget * parent, const gchar * format, ...)
{
  va_list ap;
  GtkWidget *dialog;
  gchar *message;

  va_start (ap, format);

  message = g_strdup_vprintf (format, ap);

  va_end (ap);

  dialog = gtk_message_dialog_new (GTK_WINDOW (parent),
				   GTK_DIALOG_MODAL,
				   GTK_MESSAGE_ERROR,
				   GTK_BUTTONS_CLOSE, message);
  g_free (message);

  gtk_window_set_title (GTK_WINDOW (dialog), _("Gnubik error"));

  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent));

  gtk_dialog_run (GTK_DIALOG (dialog));

  gtk_widget_destroy (dialog);
}



#if !X_DISPLAY_MISSING
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#endif

void
set_the_colours (GtkWidget * w, const char *progname)
{
#if !X_DISPLAY_MISSING
  int i;

  Display *dpy;

  dpy = GDK_WINDOW_XDISPLAY (gtk_widget_get_parent_window (w));


  for (i = 0; i < 6; ++i)
    {
      char *colour = 0;
      char resname[20];
      GdkColor xcolour;
      g_snprintf (resname, 20, "color%d", i);
      colour = XGetDefault (dpy, progname, resname);

      if (!colour)
	continue;

      if (!gdk_color_parse (colour, &xcolour))
	{
	  g_warning ("colour %s not in database\n", colour);
	}
      else
	{
	  /* convert colours to GLfloat values,  and set them */
	  const unsigned short full = ~0;

	  GLfloat red = (GLfloat) xcolour.red / full;
	  GLfloat green = (GLfloat) xcolour.green / full;
	  GLfloat blue = (GLfloat) xcolour.blue / full;

	  setColour (i, red, green, blue);
	}
    }
#endif
}
