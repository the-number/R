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

#ifndef MOVE_H
#define MOVE_H


/* Structure used to communicate which blocks of the cube lie in a certain
   slice. These objects should only be constructed by one of the identify_blocks
   functions,  and must be destroyed with the free_slice_blocks function. */
typedef struct _Slice_Blocks
{
  int *blocks;
  int number_blocks;

} Slice_Blocks;

/* A structure containing information about a movement taking place. */
struct move_data;


#define MD_OPAQUE 0

#if !MD_OPAQUE
/* A structure containing information about a movement taking place. */
struct move_data
{
  int slice;
  short dir;   /* 0 = cw, 1 = ccw */
  short axis;  /* [0,2] */
  short turns; /* Normally 1 or 2 */

  Slice_Blocks *blocks_in_motion;
};
#endif


struct move_data * move_create (int slice, short axis, short dir);
void move_free (struct move_data *);
struct move_data * move_copy (const struct move_data *);

short move_turns (const struct move_data *);
short move_dir (const struct move_data *);
short move_axis (const struct move_data *);


#endif