/*
    Menu definition code.
    Copyright (C) 2003  John Darrington

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

#ifndef MENUS_GTK_H
#define MENUS_GTK_H

#include <gtk/gtk.h>

void request_new_game ( GtkWidget *w,    gpointer   data );
			
void preferences ( GtkWidget *w,  gpointer   data );

/* Close  a window without doing anything else */
void close_no_action (GtkWidget *w ,  gpointer data);
			
void about (GtkWidget *w, GtkWindow *);

void  move ( GtkWidget *w,       gpointer   data );


#endif
