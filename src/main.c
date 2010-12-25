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
#include <stdbool.h>

static const char help_string[];

struct cube *the_cube = NULL;

static void app_opts (int *argc, char **argv);

struct application_options
{
  bool solved ;
  int initial_cube_size[3] ;
};

static struct application_options opts = { false, {3,3,3}};

extern struct animation animation;


struct cublet_selection *the_cublet_selection;

static gboolean
on_crossing  (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
  struct cublet_selection *cs = data;

  if (event->type == GDK_ENTER_NOTIFY)
    select_enable (cs);

  if (event->type == GDK_LEAVE_NOTIFY)
    select_disable (cs);

  return TRUE;
}

static gboolean
enable_selection  (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  int x, y;
  gtk_widget_get_pointer (widget, &x, &y);

  struct cublet_selection *cs = data;
  GdkRectangle area = event->area;


  if ( x < area.x || y < area.y || x >= area.x + area.width || y >= area.y + area.height )
    select_disable (cs);
  else
    select_enable (cs);

  return TRUE;
}


static void
c_main (void *closure, int argc, char *argv[])
{
  GtkWidget *form;
  GtkWidget *glxarea;
  GtkWidget *window;
  GtkWidget *menubar;
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
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (window, "delete-event", G_CALLBACK (gtk_main_quit), 0);

  gtk_window_set_icon_name (GTK_WINDOW(window), "gnubik");


  /* process arguments specific to this program */
  app_opts (&argc, argv);

  /* create a vbox to hold the drawing area and the menubar */
  form = gtk_vbox_new (FALSE, 0);

  gtk_container_add (GTK_CONTAINER (window), form);

  menubar = create_menubar (window);

  gtk_box_pack_start (GTK_BOX (form), menubar, FALSE, TRUE, 0);

  glxarea = create_gl_area ();

  gtk_box_pack_start (GTK_BOX (form), glxarea, TRUE, TRUE, 0);


  /* create the cube */
  the_cube = new_cube (opts.initial_cube_size[0],
		   opts.initial_cube_size[1],
		   opts.initial_cube_size[2]);
  
  /* If a solved cube has not been requested,  then do some random
     moves on it */
  if (!opts.solved)
    cube_scramble (the_cube);

  scene_init ();

  /* initialise the selection mechanism */
  the_cublet_selection = select_create (glxarea, 50, 1, selection_func);

  gtk_widget_add_events (GTK_WIDGET (glxarea),
			 GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK
			 | GDK_BUTTON_RELEASE_MASK
			 /* | GDK_BUTTON_MOTION_MASK */
			 | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
			 | GDK_VISIBILITY_NOTIFY_MASK
			 | GDK_POINTER_MOTION_MASK);

  GTK_WIDGET_SET_FLAGS (glxarea, GTK_CAN_FOCUS);

  g_signal_connect (glxarea, "key-press-event",
		    G_CALLBACK (cube_orientate_keys), 0);

  g_signal_connect (glxarea, "motion-notify-event",
		    G_CALLBACK (cube_orientate_mouse), 0);

  g_signal_connect (glxarea, "scroll-event",
		    G_CALLBACK (z_rotate), 0);

  g_signal_connect (glxarea, "expose-event",
		    G_CALLBACK (enable_selection), the_cublet_selection);

  g_signal_connect (glxarea, "leave-notify-event",
		    G_CALLBACK (on_crossing), the_cublet_selection);

  g_signal_connect (glxarea, "enter-notify-event",
		    G_CALLBACK (on_crossing), the_cublet_selection);

  g_signal_connect (glxarea, "button-press-event",
		    G_CALLBACK (on_button_press_release), the_cublet_selection);

  g_signal_connect (glxarea, "button-release-event",
		    G_CALLBACK (on_button_press_release), the_cublet_selection);

  g_signal_connect (glxarea, "button-press-event",
		    G_CALLBACK (on_mouse_button), 0);
  
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
	  sscanf (optarg, "%d", &animation.frameQty);
	  break;
	case 's':
	  opts.solved = 1;
	  break;
	case 'z':
	    opts.initial_cube_size[0] = atoi (strtok (optarg, ","));
	    if ( opts.initial_cube_size[0] <= 0)
	      opts.initial_cube_size[0] = 3;
	    
	    for (i = 1; i < 3; ++i)
	      {
		char *x = strtok (NULL, ",");
		opts.initial_cube_size[i] = x ? atoi (x) : -1;

		if ( opts.initial_cube_size[i] <= 0)
		  opts.initial_cube_size[i] = opts.initial_cube_size[i-1];
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


#if !X_DISPLAY_MISSING
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#endif

void
set_the_colours (GtkWidget *w, const char *progname)
{
#if !X_DISPLAY_MISSING
  int i;

  Display *dpy = GDK_WINDOW_XDISPLAY (gtk_widget_get_parent_window (w));

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
