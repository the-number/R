/*
  Copyright (c) 2011        John Darrington

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

#include "move.h"

#include <glib.h>


#if MD_OPAQUE
/* A structure containing information about a movement taking place. */
struct move_data
{
  int slice;			/* Zero is the centre of the cube */
  short dir;			/* 0 = cw, 1 = ccw */
  short axis;			/* [0,2] */
  short turns;			/* Normally 1 or 2 */

  Slice_Blocks *blocks_in_motion;
};
#endif

void
move_set_turns (struct move_data *m, int turns)
{
  m->turns = turns;
}

struct move_data *
move_create (int slice, short axis, short dir)
{
  struct move_data *m = g_slice_alloc (sizeof (*m));

  m->blocks_in_motion = NULL;

  /* Default to a turn of 90 degrees */
  m->turns = 1;

  m->slice = slice;
  m->dir = dir;
  m->axis = axis;
  m->ref = 1;

  return m;
}

void
move_dump (const struct move_data *m)
{
  g_print ("Slice: %d; Axis: %d; Dir: %d; Turns: %d; bm(%p)\n",
	   m->slice, m->axis, m->dir, m->turns, m->blocks_in_motion);
}

/*
 Release the resources associated with the Slice_Block.
*/
static void
free_slice_blocks (Slice_Blocks * slice)
{
  if (slice && --slice->ref == 0)
    {
      if (slice->blocks)
	g_free (slice->blocks);
      g_free (slice);
    }
}


static void
move_free (struct move_data *m)
{
  free_slice_blocks (m->blocks_in_motion);
  g_slice_free (struct move_data, m);
}

#define NOREFCOUNTING 0

#if NOREFCOUNTING
static struct move_data *
x_move_unref (struct move_data *m)
{
  if (m == NULL)
    return NULL;
  move_free (m);
  return NULL;
}
#else
static struct move_data *
x_move_unref (struct move_data *m)
{
  if (NULL == m)
    return NULL;

  if (0 == --m->ref)
    {
      move_free (m);
      return NULL;
    }

  return m;
}
#endif



void
move_unref (const struct move_data *m_)
{
  struct move_data *m = (struct move_data *) m_;

  x_move_unref (m);
}


#if NOREFCOUNTING
const struct move_data *
move_ref (const struct move_data *m)
{
  return move_copy (m);
}
#else
const struct move_data *
move_ref (const struct move_data *m)
{
  g_assert (m);

  ((struct move_data *) m)->ref++;

  return m;
}
#endif

struct move_data *
move_copy (const struct move_data *n)
{
  if (n == NULL)
    return NULL;

  struct move_data *m = g_slice_alloc0 (sizeof (*m));

  m->blocks_in_motion = n->blocks_in_motion;
  if (m->blocks_in_motion)
    m->blocks_in_motion->ref++;
  m->turns = n->turns;
  m->slice = n->slice;
  m->dir = n->dir;
  m->axis = n->axis;
  m->ref = 1;

  return m;
}


GType
move_get_type (void)
{
  static GType t = 0;

  if (t == 0)
    {
      t = g_boxed_type_register_static ("gnubik-move",
					(GBoxedCopyFunc) move_copy,
					(GBoxedFreeFunc) x_move_unref);
    }

  return t;
}


short
move_turns (const struct move_data *m)
{
  return m->turns;
}

short
move_dir (const struct move_data *m)
{
  return m->dir;
}

short
move_axis (const struct move_data *m)
{
  return m->axis;
}
