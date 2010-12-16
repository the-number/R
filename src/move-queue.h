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

#ifndef MOVE_QUEUE_H
#define MOVE_QUEUE_H





/*
 *  OBJECT Move_Queue
 *
 *  DESCRIPTION
 *
 *    This object is designed to hold runs of moves which are applied to a
 *    cube. New moves are pushed onto the back of the queue,  and old ones may be
 *    popped of the front. The queue is indexed by a moving cursor (known
 *    conventionally as the 'current' place) which can be moved backwards and
 *    forwards. In this way,  the GNUbik application can maintain a running list
 *    of moves being applied (either by the user or from script-fu),  and can
 *    allow the user to move backwards and forwards through the moves giving a
 *    rewind/replay facility.
 *
 *    The queue object also holds some (redundant) state information,  which is
 *    not currently used (or even accessible),  but can be used to monitor the
 *    state of the queue (length,  percentage run,  etc).
 */



/* A structure containing information about a movement taking place. */
typedef struct _Move_Data
{
  int slice;
  int dir;			/* 0 or 1. */
  int axis;
} Move_Data;


#include "move-queue_i.h"


typedef struct _Move_Queue Move_Queue;




/* Construct a new queue object. */
Move_Queue *new_move_queue (void);


/* Destroy an object created with the routine above. */
void free_move_queue (Move_Queue * move_queue);


/* Add a new Move_Data item onto the tail end of the queue. 1 (one) is returned
   unless there is insufficient memory to grow the queue. A local copy of the
   move_data object is made,  so the incumbent object remains in the ownership of
   the calling application. */
int move_queue_push (Move_Queue * move_queue, const Move_Data * move_data);


/* Add a new Move_Data item at the current place in the queue,  dropping all
   subsequent moves. */
int move_queue_push_current (Move_Queue * move_queue,
			     const Move_Data * move_data);


/* Remove the current item and all those that come afterwards. */
void move_queue_truncate (Move_Queue * move_queue);


/* Return the data regarded as the current item on the queue. */
const Move_Data *move_queue_current (const Move_Queue * move_queue);


/* Move the current item upwards (towards the first,  or oldest,  item pushed onto
   the queue). */
int move_queue_advance (Move_Queue * move_queue);


/* Move the current item downwards (towards the last,  or newest,  item added to
   the queue). */
int move_queue_retard (Move_Queue * move_queue);


/* A simple compound accessor. */

typedef struct _Move_Queue_Progress
{
  int current, total;
}
Move_Queue_Progress;

Move_Queue_Progress move_queue_progress (const Move_Queue * move_queue);


/* Mark the current item as a rewind stop point. */
void move_queue_mark_current (Move_Queue * move_queue);


#endif /* Undefined MOVE_QUEUE_H. */
