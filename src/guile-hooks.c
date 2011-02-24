/*
    Copyright (C) 2004, 2011  Dale Mellor

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

#include "guile-hooks.h"

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>

#include "cube.h"
#include "menus.h"

#include <libguile.h>


/* When a script runs,  the first cube movement that is requested by the script
   must flush the move queue after the place of current insertion; further move
   requests must be appended to the end of the move queue. This variable is used
   to track the first request. */

static int moved;



/* This function is called from the menu when the user makes a selection. The
   data is a string which was registered with the menu system and gives the name
   of a scheme procedure to execute. */

static void
run_scheme (GtkAction *act, SCM exp)
{
  moved = 0;

  scm_eval (exp, scm_interaction_environment());

  //  request_play ();
}

/* The menu manager */
static GtkUIManager *uim;

static SCM
gnubik_create_menu (SCM name, SCM loc)
{
  char *ml = scm_to_locale_string (name);
  char *loc_str = NULL;

  GtkActionGroup *ag = gtk_action_group_new (ml);

  GtkActionEntry gae;

  if (SCM_UNBNDP (loc))
    loc_str = g_strdup ("/ui/MainMenu/scripts-menu");
  else
    loc_str = scm_to_locale_string (loc);

  gae.name = ml;
  gae.stock_id = NULL;
  gae.label = ml;
  gae.accelerator = NULL;
  gae.tooltip = NULL;
  gae.callback = NULL;

  gtk_action_group_add_actions (ag, &gae, 1, NULL);

  gtk_ui_manager_insert_action_group (uim, ag, 0);

  gtk_ui_manager_add_ui (uim,
			 gtk_ui_manager_new_merge_id (uim),
			 loc_str, ml, ml, GTK_UI_MANAGER_MENU, TRUE);

  char *menuloc = g_strdup_printf ("%s/%s", loc_str, ml);

  SCM sml = scm_from_locale_string (menuloc);

  free (ml);
  free (menuloc);
  free (loc_str);

  return sml;
}

/*
  Function callable from scheme (as gnubik-register-script) which allows a
  script to specify a menu entry and the name of a procedure to call when that
  menu entry is selected. Note that /Script-fu/ is always appended,  so all
  scripts are forced under the Script-fu main menu item. 
*/
static SCM
gnubik_register_script (SCM menu_location, SCM callback, SCM loc)
{
  char *ml = scm_to_locale_string (menu_location);

  char *loc_str = scm_to_locale_string (loc);

  GtkActionGroup *ag = gtk_action_group_new (ml);

  GtkActionEntry gae;
  gae.name = ml;
  gae.stock_id = NULL;
  gae.label = ml;
  gae.accelerator = NULL;
  gae.tooltip = NULL;
  gae.callback = G_CALLBACK (run_scheme);

  scm_permanent_object (callback);

  gtk_action_group_add_actions (ag, &gae, 1, callback);

  gtk_ui_manager_insert_action_group (uim, ag, 0);

  gtk_ui_manager_add_ui (uim, gtk_ui_manager_new_merge_id (uim),
			 loc_str, ml, ml, GTK_UI_MANAGER_MENUITEM, TRUE);

  free (loc_str);
  free (ml);

  return SCM_UNSPECIFIED;
}




extern GbkGame *the_game;

/* Function callable from scheme as gnubik-cube-state which returns a structure
   reflecting the current state of the cube. */

static SCM
gnubik_cube_state ()
{
  return make_scm_cube (the_game->cube);
}




/* The first time a script makes a move on the cube,  the move_queue must be
   truncated to the current place,  and the place is marked so that the viewer
   can rewind the effects of the script. This script performs the necessary
   preparations. */

static void
start_moves_if_first ()
{
  if (!moved)
    {
      moved = 1;
      //request_truncate_move_queue ();
      //      request_mark_move_queue ();
    }
}




/* Function which,  when called from scheme as gnubik-rotate-animated,  causes one
   side of the cube to rotate on-screen. */

static SCM
gnubik_rotate_animated (SCM list)
{
  start_moves_if_first ();

  for (; !SCM_NULLP (list); list = SCM_CDR (list))
    {
      struct move_data *move = move_create (scm_to_int (SCM_CADAR (list)),
					    scm_to_int (SCM_CAAR (list)),
					    scm_to_int (SCM_CADDAR (list)));

      gbk_cube_rotate_slice (the_game->cube, move);
      move_unref (move);
    }

  return SCM_UNSPECIFIED;
}



/* Function allowing a script to apply all its moves in one go to the cube,
   without creating animations on the display. */

static SCM
gnubik_run_moves ()
{
  //request_fast_forward ();

  return SCM_UNSPECIFIED;
}




/* Function to allow a guile script to display a message to the user. */

static SCM
gnubik_error_dialog (SCM message)
{
  char *msg = scm_to_locale_string (message);
  error_dialog (the_game->toplevel, msg);
  free (msg);

  return SCM_UNSPECIFIED;
}




/* Function to scan the named directory for all files with a .scm extension,  and
   execute the contents of each file. */
static void
read_script_directory (const char *dir_name)
{
  static char buffer[1024];

  DIR *directory = opendir (dir_name);

  if (directory)
    {
      struct dirent *entry;

      for (entry = readdir (directory); entry; entry = readdir (directory))

	if (strcmp (".scm", entry->d_name + strlen (entry->d_name) - 4) == 0)
	  {
	    snprintf (buffer, 1024, "%s/%s", dir_name, entry->d_name);

	    scm_primitive_load (scm_makfrom0str (buffer));
	  }
    }

  closedir (directory);
}




/* This function initializes the scheme world for us,  and once the scripts have
   all been run,  it returns the requested menu structure to the caller. Before
   running the scripts,  however,  it first makes sure all the pertinent C
   functions are registered in the guile world. */

void
startup_guile_scripts (GtkUIManager * ui_manager)
{
  /* Register C functions that the scheme world can access. */

  scm_c_define_gsubr ("gnubik-create-menu", 1, 1, 0, gnubik_create_menu);

  scm_c_define_gsubr ("gnubik-register-script",
		      3, 0, 0, gnubik_register_script);

  scm_c_define_gsubr ("gnubik-cube-state", 0, 0, 0, gnubik_cube_state);

  scm_c_define_gsubr ("gnubik-rotate-animated",
		      1, 0, 0, gnubik_rotate_animated);

  scm_c_define_gsubr ("gnubik-run-moves", 0, 0, 0, gnubik_run_moves);

  scm_c_define_gsubr ("gnubik-error-dialog", 1, 0, 0, gnubik_error_dialog);

  uim = ui_manager;

  /* Run all the initialization files in .../share/gnubik/guile, and the
     system scripts in .../share/gnubik/scripts. */

  read_script_directory (GUILEDIR);
  read_script_directory (SCRIPTDIR);

    {
      gchar *cfd = g_strdup_printf ("%s/gnubik",g_get_user_config_dir ());

      read_script_directory (cfd);

      g_free (cfd);
    }
}
