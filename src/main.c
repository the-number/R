/*
    GNUbik -- A 3 dimensional magic cube game.
    Copyright (C) 1998, 2003--2004  John Darrington

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

#include <time.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <locale.h>
#include <config.h>
#include "cube.h"
#include "drwBlock.h"
#include "version.h"
#include "ui.h"
#include "glarea.h"
#include "widget-set.h"
#include "select.h"

#include <assert.h>

#include <libintl.h>
#include <ctype.h>

static const char help_string[];


void app_opts (int *argc, char **argv);

/* this is the number of frames drawn when a slice of the cube rotates
   90 degrees */
extern int frameQty;


int cube_dimension = 3;
static short solved = 0;
int number_of_blocks;



static GtkWidget *glxarea;
GtkWidget *main_application_window;

static void
c_main (void *closure, int argc, char *argv[])
{
  GtkWidget *form;
  GtkWidget *menubar;
  GtkWidget *play_toolbar;
  GtkWidget *statusbar;



  /* Internationalisation stuff */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  widget_set_init (&argc, &argv);

#if DEBUG && HAVE_GL_GLUT_H
  glutInit ();
#endif


  /* Create the top level widget --- that is,  the main window which everything
     goes in */
  main_application_window = create_top_level_widget ();


  /* process arguments specific to this program */
  app_opts (&argc, argv);

  /* Create a container widget.   A `thing' which will contain the menubar
     and the gl area */
  form = create_container_widget (main_application_window);


  menubar = create_menubar (form, main_application_window);
  play_toolbar = create_play_toolbar (form, main_application_window);



  glxarea = create_gl_area (form);

  gtk_box_pack_start (GTK_BOX (form), glxarea, TRUE, TRUE, 0);

  gtk_widget_show (glxarea);


  statusbar = create_statusbar (form);


  /* create the cube */
  number_of_blocks = create_the_cube (cube_dimension);
  assert (number_of_blocks > 0);

  /* If a solved cube has not been requested,  then do some random
     moves on it */
  if (!solved)
    {
      int i;
      srand (time (0));
      for (i = 0; i < 8 * cube_dimension; i++)
	{
	  Slice_Blocks *blocks = identify_blocks (the_cube,
						  rand () % number_of_blocks,
						  rand () % 3);

	  rotate_slice (the_cube, rand () % 2 + 1, blocks);

	  free_slice_blocks (blocks);
	}
    }

  /* initialise the selection mechanism */
  initSelection (glxarea, 50, 1, selection_func);


  gtk_widget_show (main_application_window);

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
void
app_opts (int *argc, char **argv)
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
      switch (c)
	{
	case 'a':
	  sscanf (optarg, "%d", &frameQty);
	  break;
	case 's':
	  solved = 1;
	  break;
	case 'z':
	  sscanf (optarg, "%d", &cube_dimension);
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
  "-z n\n--size=n\tShow a   n x n x n   sized cube \n\n"
  "-a n\n--animation=n\tNumber of intermediate positions to be shown in animations\n\n"
  "\n\nBug reports to " PACKAGE_BUGREPORT "\n";
