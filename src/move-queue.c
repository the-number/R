/*
  Copyright (c) 2004 Dale Mellor
  Copyright (c) 2011 John Darrington

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


#include "move-queue.h"

#include <stdlib.h>
#include <string.h>



/* Create and return a pointer to a new object,  which represents a queue holding
   no data. */

static void
_initialize_new_object (Move_Queue * ret)
{
  ret->head = ret->tail = ret->current = NULL;
  ret->queue_length = 0;
  ret->current_place = 0;
  ret->mark_next_push = 0;
}

Move_Queue *
new_move_queue (void)
{
  Move_Queue *ret;

  if ((ret = g_malloc (sizeof (*ret))) == NULL)
    return NULL;

  _initialize_new_object (ret);

  return ret;
}




/* Release all the memory associated with the given queue object. */

static void
_release_items (struct _Move_Queue_Item *next_item)
{
  struct _Move_Queue_Item *next;

  while (next_item != NULL)
    {
      next = next_item->next;
      g_free (next_item);
      next_item = next;
    }
}

void
free_move_queue (Move_Queue * move_queue)
{
  _release_items (move_queue->head);
  g_free (move_queue);
}




/* Routine to push a new Move_Data object onto the _back_ of the given queue. A
   new,  locally-managed copy is made of the incumbent datum. Return 1 (one) if
   the operation is successful; 0 (zero) indicates that there was insufficient
   memory to grow the queue. */
int
move_queue_push (Move_Queue * move_queue, const struct move_data *move_data)
{
  struct _Move_Queue_Item *new_element;

  if ((new_element = g_malloc (sizeof (*new_element))) == NULL)
    return 0;

  memcpy (&new_element->data, move_data, sizeof (*move_data));
  new_element->mark = move_queue->mark_next_push;
  new_element->next = NULL;

  if (move_queue->tail != NULL)
    move_queue->tail->next = new_element;

  move_queue->tail = new_element;

  ++move_queue->queue_length;

  if (move_queue->head == NULL)
    move_queue->head = new_element;

  if (move_queue->current == NULL)
    move_queue->current = new_element;

  move_queue->mark_next_push = 0;

  return 1;
}




/* Procedure to copy the incumbent datum to the current place in the queue,  and
   drop all subsequent queue entries (so that the current element becomes the
   tail). If current points past the end of the tail (including the case when
   there are no data in the queue),  then a regular push operation is performed
   at the tail,  and in this case zero may be returned if there is insufficient
   memory to grow the queue. In all other cases 1 (one) will be returned. */
int
move_queue_push_current (Move_Queue * move_queue,
			 const struct move_data *move_data)
{
  /* If there are no data in the queue,  then perform a standard push
     operation. Also do this if we are at the tail. */
  if (move_queue->current == NULL)
    return move_queue_push (move_queue, move_data);

  _release_items (move_queue->current->next);

  memcpy (&move_queue->current->data, move_data, sizeof (*move_data));
  move_queue->current->next = NULL;
  move_queue->tail = move_queue->current;
  move_queue->queue_length = move_queue->current_place + 1;
  move_queue->mark_next_push = 0;

  return 1;
}




/* Routine to get the data at the 'current' position in the queue. If there are
   no data in the queue,  NULL will be returned. */
const struct move_data *
move_queue_current (const Move_Queue * move_queue)
{
  return move_queue->current;
}




/* Routine to retard the current pointer (move it towards the _head_ of the
   queue). If there are no data on the queue,  or the head already is the current
   pointer,  then zero is returned to indicate that there are no more data to
   consider. If a marked item is hit,  -1 is returned.

   Note that,  as this is implemented as a singly-linked list,  this is the
   expensive operation,  as we have to trace the list from the head to the
   current datum,  in order to locate the datum above the current which will
   become the new current. The alternatives are either to use a doubly-linked
   list which will make the logic more complicated and use a (tiny) amount of
   extra memory,  or to use the list in the opposite direction but then pushing,
   poping and advancing all become expensive operations. */
int
move_queue_retard (Move_Queue * move_queue)
{
  if (move_queue->head == move_queue->current)	/* We are trying to move past
						   the head. */
    return 0;

  else if (move_queue->current == NULL)	/* We are past the tail. */
    move_queue->current = move_queue->tail;

  else				/* We must be somewhere in the middle. */
    {
      struct _Move_Queue_Item *cursor;

      for (cursor = move_queue->head;
	   cursor->next != move_queue->current; cursor = cursor->next);

      move_queue->current = cursor;
    }

  --move_queue->current_place;

  return move_queue->current->mark == 1 ? -1 : 1;
}




/* Remove the current object and all those that come afterwards. */
void
move_queue_truncate (Move_Queue * move_queue)
{
  if (move_queue->current == move_queue->head)
    {
      _release_items (move_queue->head);
      _initialize_new_object (move_queue);
    }

  else
    {
      move_queue->mark_next_push
	= move_queue->current == NULL ? 0 : move_queue->current->mark;

      move_queue_retard (move_queue);

      if (move_queue->current != NULL)
	_release_items (move_queue->current->next);

      move_queue->current->next = NULL;

      move_queue->tail = move_queue->current;

      move_queue_advance (move_queue);

      move_queue->queue_length = move_queue->current_place;
    }
}




/* Routine to advance the current position (move it towards the _tail_). If the
   current position is already at the tail (which will also be the case if the
   queue is empty) then no action takes place and 0 (zero) is returned to
   indicate that no more data are available for consideration. */
int
move_queue_advance (Move_Queue * move_queue)
{
  if (move_queue->current == NULL)
    return 0;

  move_queue->current = move_queue->current->next;
  ++move_queue->current_place;

  return (move_queue->current != NULL);
}




/* A simple compound accessor. */
Move_Queue_Progress
move_queue_progress (const Move_Queue * move_queue)
{
  Move_Queue_Progress p;

  p.current = move_queue->current_place;
  p.total = move_queue->queue_length;

  return p;
}




/* And a mutator. */
void
move_queue_mark_current (Move_Queue * move_queue)
{
  if (move_queue->current)
    move_queue->current->mark = 1;

  else
    move_queue->mark_next_push = 1;
}
