/*
    Copyright (c) 2004  Dale Mellor

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

#ifndef MOVE_QUEUE_I_H
#define MOVE_QUEUE_I_H

#ifndef MOVE_QUEUE_H
#error This is a private header file and should not be included in application files.
#endif


/*
 * The queue is implemented as a singly-linked list of _Move_Queue_Item
 * objects. For maximum efficiency pushes are made at the back (tail),  pops at
 * the front (head),  and the normal direction of recursion is towards the tail;
 * retarding the cursor is the one (relatively) expensive operation.
 *
 * Note that the cursor,  current,  points to the next move to be made (the true
 * location of the cursor is between this move and the previous move).
 */


struct _Move_Queue_Item
{
  struct move_data data;
  int mark;
  struct _Move_Queue_Item *next;
};

struct _Move_Queue
{
  struct _Move_Queue_Item *head;	/* Pop off this end. */
  struct _Move_Queue_Item *tail;	/* Push on this end. */
  struct _Move_Queue_Item *current;	/* User's 'roaming' cursor. */
  int queue_length;		/* The number of items on the queue. */
  int current_place;		/* Number of steps of current from head. */
  int mark_next_push;		/* Will the next move be a marked point? */
};




#endif /* Undefined MOVE_QUEUE_I_H. */
