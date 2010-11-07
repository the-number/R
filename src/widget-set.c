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
#include "widget-set.h"
#include "menus-gtk.h"
#include "colour-sel.h"
#include "version.h"
#include "guile-hooks.h"
#include "move-queue.h"
#include "ui.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <gdk/gdkx.h>
#include <gtk/gtkgl.h>

#include "drwBlock.h"

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)



void
widget_set_init (int *argc,  char *** argv)
{

  gtk_init (argc,  argv);

  gtk_gl_init (argc,  argv);

}


static GtkWidget *window;

GtkWidget *
create_top_level_widget (void)
{

  GtkWidget *win;

  /* create a new window */
  win = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (win, "delete-event",
		      G_CALLBACK (gtk_main_quit), 0);


  window = win;

  return win;

}

GtkWidget *
create_container_widget (GtkWidget * parent)
{
  GtkWidget *vbox ;

  /* create a vbox to hold the drawing area and the menubar */
  vbox = gtk_vbox_new (FALSE, 0);


  gtk_container_add (GTK_CONTAINER (parent),  vbox);

  gtk_widget_show (vbox);

  return vbox;

}



static GtkWidget *statusbar;
extern Move_Queue *move_queue;
#define MSGLEN 100
void
update_statusbar (void)
{

  static int context=0;

  static guint mesg_id = 0;

  gchar mesg[MSGLEN];

  if ( 0 == context )
    context  = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar),
					     "move-count");

  g_snprintf (mesg,
             MSGLEN,
             _("Moves: %d / %d"),
             move_queue_progress (move_queue).current,
             move_queue_progress (move_queue).total);

  if ( mesg_id != 0 )
    gtk_statusbar_remove (GTK_STATUSBAR (statusbar),  context,  mesg_id);

  mesg_id = gtk_statusbar_push (GTK_STATUSBAR (statusbar),  context,  mesg);

}

void
declare_win (const struct cube *cube)
{
  static int context=0;

  static guint mesg_id = 0;

  gchar mesg[MSGLEN];

  enum Cube_Status status = cube_get_status (cube);

  if ( 0 == context )
    context  = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar),
					     "win");

  switch (status)
    {
    case SOLVED:
      g_snprintf (mesg,  MSGLEN,  ngettext ("Cube solved in %d move",
					    "Cube solved in %d moves",
					    move_queue_progress (move_queue).total),
		  move_queue_progress (move_queue).total);
      break ;
    case HALF_SOLVED:
      g_snprintf (mesg,
		  MSGLEN, _("Cube is NOT solved! The colours are correct,  but have incorrect orientation"));
      break;
    default:
      g_assert_not_reached ();
    }


  mesg_id = gtk_statusbar_push (GTK_STATUSBAR (statusbar),  context,  mesg);
}

GtkWidget *
create_statusbar (GtkWidget * container)
{


  statusbar = gtk_statusbar_new ();

  gtk_box_pack_start (GTK_BOX (container),  statusbar,  FALSE,  TRUE,  0);

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
        gtk_widget_set_sensitive (GTK_WIDGET (list->data),
                                  flags & 1);
        flags >>= 1;
    }
}


static GtkWidget *play_toolbar;


GtkWidget *
create_play_toolbar (GtkWidget * container,  GtkWidget * toplevel)
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


  play_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (play_toolbar),  GTK_TOOLBAR_BOTH);
  gtk_container_set_border_width (GTK_CONTAINER (play_toolbar),  0);
  gtk_box_pack_start (GTK_BOX (container),  play_toolbar,  FALSE,  TRUE, 0);


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
toggle_visibility (GtkMenuItem *menuitem,
		  gpointer user_data)
{
  GtkWidget **w = (user_data);

  g_assert (*w);

  if ( GTK_WIDGET_VISIBLE (*w))
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
  gtk_menu_shell_append (GTK_MENU_SHELL (menu),  menuitem);
  gtk_widget_show (menuitem);

  g_signal_connect (menuitem,  "activate",
			G_CALLBACK (toggle_visibility),  &play_toolbar);

  menuitem = gtk_menu_item_new_with_mnemonic (_("_Status Bar"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu),  menuitem);
  gtk_widget_show (menuitem);

  g_signal_connect (menuitem,  "activate",
			G_CALLBACK (toggle_visibility),  &statusbar);



  return menu;
}


/* Duplicate the GtkItemFactoryEntries,  calling gettext on the path */
static GtkItemFactoryEntry *
dup_items (const GtkItemFactoryEntry *src, gint n)
{
  gint i;
  GtkItemFactoryEntry *dest;

  dest = g_malloc ( n * sizeof (GtkItemFactoryEntry) );

  for (i = 0 ; i < n; ++i )
    {
      dest[i] = src[i];
      dest[i].path = g_strdup (gettext (src[i].path));
    }

  return dest;
}

GtkWidget *
create_menubar (GtkWidget * container,  GtkWidget * toplevel)
{
  GtkWidget *menubar;

  const GtkItemFactoryEntry menu_items[] =
    {
      { N_("/_Game"),          NULL,          NULL,  0,  "<Branch>" },
      { N_("/_Game/_New Game"),      "<control>N",  request_new_game,  0,  NULL },
      { N_("/_Game/sep"),      NULL,          NULL,  0,  "<Separator>" },
      { N_("/_Game/_Quit"),      "<control>Q",  gtk_main_quit,  0, "<StockItem>",  GTK_STOCK_QUIT },
      { N_("/_Settings"),       NULL,          NULL,  0,  "<Branch>" },
      { N_("/_Settings/_Preferences"),   NULL,  preferences,  toplevel, "<StockItem>",  GTK_STOCK_PREFERENCES },
      { N_("/_Settings/_Colours"),   NULL,  colour_select_menu,  0, "<StockItem>",  GTK_STOCK_SELECT_COLOR },
      { N_("/_Settings/Sho_w\\/Hide"),   NULL,  0,  0,  NULL },
      { ("/Script-_fu"),   NULL,  NULL,  0,  "<Branch>" },
#if DEBUG
      { "/_Debug",         NULL,          NULL,  0,  "<Branch>" },
      { "/_Debug/_Move",      NULL,       move,  0,  NULL },
#endif
      { N_("/_Help"),          NULL,          NULL,  0,  "<Branch>" },
      { N_("/_Help/_About"),    NULL,      about,  0,  NULL },
    };

  GtkItemFactory *item_factory;
  GtkAccelGroup *accel_group;

  GtkItemFactoryEntry *menu_copy;


  const gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);


  menu_copy = dup_items (menu_items,  nmenu_items);

  accel_group = gtk_accel_group_new ();

  /* This function initializes the item factory.
     Param 1: The type of menu - can be GTK_TYPE_MENU_BAR,  GTK_TYPE_MENU,
     or GTK_TYPE_OPTION_MENU.
     Param 2: The path of the menu.
     Param 3: A pointer to a gtk_accel_group.  The item factory sets up
     the accelerator table while generating menus.
  */

  item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR,  "<main>",
                                       accel_group);

  /* This function generates the menu items. Pass the item factory,
     the number of items in the array,  the array itself,  and and
     callback data for the the menu items. */
  gtk_item_factory_create_items (item_factory,  nmenu_items,
				 (GtkItemFactoryEntry*) menu_copy,  NULL);


 {
     /* Get the menu structure asked for by the collection of Guile scripts. */
     Menu_Item_List *list = startup_guile_scripts ();

     /* And use the factory to add the items to the menu. */
     for (; list; list = list->next)
         gtk_item_factory_create_item (item_factory,
                                       &list->entry,
                                       list->callback_data,
                                       1);
 }

 {
   GtkWidget *w;


   w = gtk_item_factory_get_widget (item_factory, _("/Settings/Show\\/Hide"));

   if ( w )
     gtk_menu_item_set_submenu (GTK_MENU_ITEM (w),  create_show_hide_menu ());

 }

   /* Attach the new accelerator group to the window. */
  gtk_window_add_accel_group (GTK_WINDOW (window),  accel_group);


    /* Finally,  return the actual menu bar created by the item factory. */
  menubar = gtk_item_factory_get_widget (item_factory,  "<main>");


  gtk_box_pack_start (GTK_BOX (container),  menubar,  FALSE,  TRUE,  0);

  gtk_widget_show (menubar);

  return menubar;

}

void
cleanup (void)
{
}


void
show_widget (GtkWidget * w)
{
  gtk_widget_show (w);

}


void
start_main_loop (void)
{
  gtk_main ();
}

void set_the_colours (GtkWidget *w,  const char *progname);


/* set the colours of the faces */
void
setCubeColours (char *progname)
{
  /*dummy*/

}


/* Popup an error dialog box */
void
error_dialog (GtkWidget *parent,  const gchar *format, ...)
{
  va_list ap;

  GtkWidget *dialog ;

  gchar *message;


  va_start (ap,  format);

  message = g_strdup_vprintf (format,  ap);

  va_end (ap);

  dialog = gtk_message_dialog_new (GTK_WINDOW (parent),
				   GTK_DIALOG_MODAL,
				   GTK_MESSAGE_ERROR,
				   GTK_BUTTONS_CLOSE,
				   message);

  g_free (message);


  gtk_window_set_transient_for (GTK_WINDOW (dialog),  GTK_WINDOW (parent));

 /* Destroy the dialog when the user responds to it (e.g. clicks a button) */
 g_signal_connect_swapped (dialog,  "response",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog);



 gtk_widget_show_all (dialog);

}



#include <X11/Xlib.h>

void
set_the_colours (GtkWidget *w,  const char *progname)
{

  int i;

  Display *dpy;

  dpy = GDK_WINDOW_XDISPLAY (gtk_widget_get_parent_window (w));


  for (i=0;i<6;++i){
    char *colour=0;
    char resname[20];
    GdkColor xcolour;
    g_snprintf (resname, 20, "color%d",  i);
    colour=XGetDefault (dpy,  progname,  resname);

    if (!colour)
      continue;

    if (!gdk_color_parse (colour, &xcolour)) {
      g_warning ("colour %s not in database\n",  colour);
    }
    else{
      /* convert colours to GLfloat values,  and set them*/
      const unsigned short full = ~0;


      GLfloat red =   (GLfloat) xcolour.red/full ;
      GLfloat green = (GLfloat) xcolour.green/full ;
      GLfloat blue =  (GLfloat) xcolour.blue/full ;

      setColour (i,  red,  green,  blue);
    }
  }
}
