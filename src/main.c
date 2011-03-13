/*
    GNUbik -- A 3 dimensional magic cube game.
    Copyright (C) 1998, 2003, 2004, 2011  John Darrington

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
#include <stdbool.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <libintl.h>
#include <getopt.h>

#include "menus.h"
#include "cubeview.h"
#include "glarea.h"
#include "version.h"
#include "dialogs.h"

#include "game.h"

static const char help_string[];



struct application_options
{
  bool solved;
  int initial_cube_size[3];
  int frameQty;
};

static void parse_app_opts (int *argc, char **argv,
			    struct application_options *opts);


GbkGame *the_game;

static void
c_main (void *closure, int argc, char *argv[])
{
  GbkCube *cube = NULL;

  GtkWidget *form;
  GtkWidget *hbox;
  GtkWidget *window;
  GtkWidget *menubar;
  GtkWidget *playbar;
  GtkWidget *cubeview;
  GtkWidget *statusbar;

  struct application_options opts = { false, {3, 3, 3}, 2 };

  /* Internationalisation stuff */
  bindtextdomain (PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (PACKAGE, "UTF-8");
  textdomain (PACKAGE);

  gtk_init (&argc, &argv);
  gtk_gl_init (&argc, &argv);

#if DEBUG && HAVE_GL_GLUT_H
  glutInit ();
#endif


  /* Create the top level widget --- that is,  the main window which everything
     goes in */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (window, "delete-event", G_CALLBACK (gtk_main_quit), NULL);

  gtk_window_set_icon_name (GTK_WINDOW (window), "gnubik");

  /* process arguments specific to this program */
  parse_app_opts (&argc, argv, &opts);


  form = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), form);

  /* create the cube */
  cube = GBK_CUBE (gbk_cube_new (opts.initial_cube_size[0],
				 opts.initial_cube_size[1],
				 opts.initial_cube_size[2]));

  /* If a solved cube has not been requested,  then do some random
     moves on it */
  if (!opts.solved)
    gbk_cube_scramble (cube);


  the_game = GBK_GAME (gbk_game_new (cube));
  the_game->toplevel = GTK_WINDOW (window);

  menubar = create_menubar (the_game);
  gtk_box_pack_start (GTK_BOX (form), menubar, FALSE, TRUE, 0);

  playbar = create_play_toolbar (the_game);
  gtk_box_pack_start (GTK_BOX (form), playbar, FALSE, TRUE, 0);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (form), hbox, TRUE, TRUE, 0);

  cubeview = gbk_cubeview_new (cube);
  gbk_cubeview_set_frame_qty (GBK_CUBEVIEW (cubeview), opts.frameQty);
  gtk_box_pack_start (GTK_BOX (hbox), cubeview, TRUE, TRUE, 0);

  gbk_game_add_view (the_game, GBK_CUBEVIEW (cubeview), TRUE);

  statusbar = create_statusbar (the_game);
  gtk_box_pack_start (GTK_BOX (form), statusbar, FALSE, FALSE, 0);

  gtk_widget_show_all (window);

  gtk_main ();
}



#include <libguile.h>

int
main (int argc, char **argv)
{
  scm_boot_guile (argc, argv, c_main, 0);
  return 0;
}

/* process the options we're interested in.  X resource overrides should
have already been extracted */
static void
parse_app_opts (int *argc, char **argv, struct application_options *opts)
{
#ifdef HAVE_GETOPT_LONG
#define GETOPT(A,  B,  C,  D,  E) getopt_long (A,  B,  C,  D,  E)
#else
#define GETOPT(A,  B,  C,  D,  E) getopt (A,  B,  C)
#endif

  int c;
  const char shortopts[] = "vhsz:a:";

#ifdef HAVE_GETOPT_LONG
  int longind;
  const struct option longopts[] = {
    {"animation", 1, 0, 'a'},
    {"help", 0, 0, 'h'},
    {"version", 0, 0, 'v'},
    {"solved", 0, 0, 's'},
    {"size", 1, 0, 'z'},
    {0, 0, 0, 0}
  };
#endif

  while ((c = GETOPT (*argc, argv, shortopts, longopts, &longind)) != -1)
    {
      int i;
      switch (c)
	{
	case 'a':
	  {
	    sscanf (optarg, "%d", &opts->frameQty);
	  }
	  break;
	case 's':
	  opts->solved = 1;
	  break;
	case 'z':
	  opts->initial_cube_size[0] = atoi (strtok (optarg, ","));
	  if (opts->initial_cube_size[0] <= 0)
	    opts->initial_cube_size[0] = 3;

	  for (i = 1; i < 3; ++i)
	    {
	      char *x = strtok (NULL, ",");
	      opts->initial_cube_size[i] = x ? atoi (x) : -1;

	      if (opts->initial_cube_size[i] <= 0)
		opts->initial_cube_size[i] = opts->initial_cube_size[i - 1];
	    }
	  break;
	case 'h':
	  printf ("%s", help_string);
	  exit (0);
	  break;
	case 'v':
	  printf ("%s\n", PACKAGE_VERSION);
	  printf ("%s", copyleft_notice);
	  exit (0);
	  break;
	default:
	  break;
	}
    }
}

static const char help_string[] =
  "-h\n--help\tShow this help message\n\n"
  "-v\n--version\tShow version number\n\n"
  "-s\n--solved\tStart with the cube already solved\n\n"
  "-z n,m,p\n--size=n\tShow a   n x m x p   sized cube \n\n"
  "-a n\n--animation=n\tNumber of intermediate positions to be shown in animations\n\n"
  "\n\nBug reports to " PACKAGE_BUGREPORT "\n";
