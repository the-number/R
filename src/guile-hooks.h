/*
    Copyright (C) 2004  Dale Mellor

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

#ifndef GUILE_HOOKS_H
#define GUILE_HOOKS_H


#include <libguile.h>
#include <gtk/gtk.h>

/* Data type to implement a singly-linked list of GtkItemFactoryEntry
   objects. */

typedef struct _Menu_Item_List
{
    GtkItemFactoryEntry entry;
    char *callback_data;
    struct _Menu_Item_List *next;
} Menu_Item_List;



/* The method which seeks out all scripts and executes them. The scripts can
   callback to the C world to register themselves. On completion,  the function
   returns a list of registered items. The function must be called exactly
   once. */

Menu_Item_List *startup_guile_scripts (void);



#endif /* defined GUILE_HOOKS_H. */
