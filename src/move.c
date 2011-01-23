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
  int slice;   /* Zero is the centre of the cube */
  short dir;   /* 0 = cw, 1 = ccw */
  short axis;  /* [0,2] */
  short turns; /* Normally 1 or 2 */

  Slice_Blocks *blocks_in_motion;
};
#endif

struct move_data * move_create (int slice, short axis, short dir)
{
  struct move_data *m = g_slice_alloc (sizeof (*m));

  m->blocks_in_motion = NULL;

  /* Default to a turn of 90 degrees */
  m->turns = 1;

  m->slice = slice;
  m->dir = dir;
  m->axis = axis;

  return m;
}



/*
 Release the resources associated with the Slice_Block.
*/
static void
free_slice_blocks (Slice_Blocks * slice)
{
  if (slice)
    {
      g_free (slice->blocks);
      g_free (slice);
    }
}

void move_free (struct move_data *m)
{
  if ( NULL == m)
    return;

  free_slice_blocks (m->blocks_in_motion);
  g_slice_free (struct move_data, m);
}

struct move_data * move_copy (const struct move_data *n)
{
  if ( n == NULL)
    return NULL;

  struct move_data *m = g_slice_alloc0 (sizeof (*m));

  m->blocks_in_motion = n->blocks_in_motion; //FIXME: Deep copy needed ???
  m->turns = n->turns;
  m->slice = n->slice;
  m->dir = n->dir;
  m->axis = n->axis;

  return m;
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
