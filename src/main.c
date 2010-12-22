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
#include "cube.h"

#include <gtk/gtkgl.h>
#include <assert.h>

#include <libintl.h>

static const char help_string[];


void app_opts (int *argc, char **argv);

/* this is the number of frames drawn when a slice of the cube rotates
   90 degrees */
extern int frameQty;


static short solved = 0;
static int initial_cube_size[3] = {3, 3, 3};

GtkWidget *main_application_window;

static void
c_main (void *closure, int argc, char *argv[])
{
  GtkWidget *form;
  GtkWidget *menubar;
  GtkWidget *play_toolbar;
  GtkWidget *statusbar;
  GtkWidget *glxarea;

  /* Internationalisation stuff */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  gtk_init (&argc, &argv);
  gtk_gl_init (&argc, &argv);

#if DEBUG && HAVE_GL_GLUT_H
  glutInit ();
#endif

  /* Create the top level widget --- that is,  the main window which everything
     goes in */
  main_application_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (main_application_window, "delete-event", G_CALLBACK (gtk_main_quit), 0);

  gtk_window_set_icon_name (GTK_WINDOW(main_application_window), "gnubik");


  /* process arguments specific to this program */
  app_opts (&argc, argv);

  /* create a vbox to hold the drawing area and the menubar */
  form = gtk_vbox_new (FALSE, 0);

  gtk_container_add (GTK_CONTAINER (main_application_window), form);

  menubar = create_menubar (form, main_application_window);
  play_toolbar = create_play_toolbar (form, main_application_window);

  glxarea = create_gl_area ();

  gtk_box_pack_start (GTK_BOX (form), glxarea, TRUE, TRUE, 0);

  statusbar = create_statusbar (form);

  /* create the cube */
  create_the_cube (initial_cube_size[0],
		   initial_cube_size[1],
		   initial_cube_size[2]);
  
  /* If a solved cube has not been requested,  then do some random
     moves on it */
  if (!solved)
    cube_scramble (the_cube);

  g_signal_connect (glxarea, "realize", G_CALLBACK (graphics_area_init), 0);

  g_signal_connect (glxarea, "expose_event", G_CALLBACK (on_expose), 0);

  g_signal_connect (glxarea, "size-allocate", G_CALLBACK (resize_viewport), 0);


  gtk_widget_add_events (GTK_WIDGET (glxarea),
			 GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK
			 | GDK_BUTTON_RELEASE_MASK
			 /* | GDK_BUTTON_MOTION_MASK */
			 | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
			 | GDK_VISIBILITY_NOTIFY_MASK
			 | GDK_POINTER_MOTION_MASK);


  GTK_WIDGET_SET_FLAGS (glxarea, GTK_CAN_FOCUS);


  g_signal_connect (glxarea, "key_press_event",
		    G_CALLBACK (cube_orientate_keys), 0);

  g_signal_connect (glxarea, "motion_notify_event",
		    G_CALLBACK (cube_orientate_mouse), 0);

  g_signal_connect (glxarea, "scroll_event",
		    G_CALLBACK (z_rotate), 0);

  g_signal_connect (glxarea, "button_press_event",
		    G_CALLBACK (on_button_press_release), 0);

  g_signal_connect (glxarea, "button_release_event",
		    G_CALLBACK (on_button_press_release), 0);

  g_signal_connect (glxarea, "button_press_event",
		    G_CALLBACK (cube_controls), 0);
  
  /* initialise the selection mechanism */
  initSelection (glxarea, 50, 1, selection_func);

  gtk_widget_show_all (main_application_window);

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
      int i;
      switch (c)
	{
	case 'a':
	  sscanf (optarg, "%d", &frameQty);
	  break;
	case 's':
	  solved = 1;
	  break;
	case 'z':
	    initial_cube_size[0] = atoi (strtok (optarg, ","));
	    if ( initial_cube_size[0] <= 0)
	      initial_cube_size[0] = 3;
	    
	    for (i = 1; i < 3; ++i)
	      {
		char *x = strtok (NULL, ",");
		initial_cube_size[i] = x ? atoi (x) : -1;

		if ( initial_cube_size[i] <= 0)
		  initial_cube_size[i] = initial_cube_size[i-1];
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
