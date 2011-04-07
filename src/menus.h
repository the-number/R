/*
    GNUbik -- A 3 dimensional magic cube game.
    Copyright (C) 2003, 2011  John Darrington

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
#ifndef WIDGET_SET_H
#define WIDGET_SET_H


#include <gtk/gtk.h>
#include "cube.h"
#include "dialogs.h"

GtkWidget *create_menubar (GbkGame *);

GtkWidget *create_play_toolbar (GbkGame *);


enum
{
  PLAY_TOOLBAR_BACK = 0x0b << 0,
  PLAY_TOOLBAR_STOP = 0x01 << 2,
  PLAY_TOOLBAR_PLAY = 0x03 << 4
};
void set_toolbar_state (unsigned state);

GtkWidget *create_statusbar (GbkGame *);

/* Popup an error dialog box */
void error_dialog (GtkWindow * parent, const char *format, ...);

void start_new_game (GbkGame *, int size0, int size1, int size2,
		     gboolean scramble);

#endif
