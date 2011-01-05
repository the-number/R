/*
  Copyright (c) 2009        John Darrington
  Copyright (c) 2004        Dale Mellor,  John Darrington
  copyright (c) 1998, 2003, 2011 John Darrington

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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <libguile.h>
#include <assert.h>

#include "cube.h"




/* Cube co-ordinates have their origin at the centre of the cube,  and their
   units are equivalent to one half of the length of one edge of a block. */
/* This func initialises the positions of the blocks which comprise the cube.
   The enumeration scheme I have chosen goes around four surfaces of the cube,
   then fills in the ends.  Thus,  for a 4x4x4 cube it looks like:

   ----------------------------------------------------------------------------
   View this diagram with 132 coloumns!

   |  60 |  61 |  62 |  63 |
   |  44 |  45 |  46 |  47 |    |  56 |  57 |  58 |  59 |
   |  28 |  29 |  30 |  31 |    |  40 |  41 |  42 |  43 |    |  52 |  53 |  54 |  55 |
   |  12 |  13 |  14 |  15 |    |  24 |  25 |  26 |  27 |    |  36 |  37 |  38 |  39 |    |  48 |  49 |  50 |  51 |
   |   8 |   9 |  10 |  11 |    |  20 |  21 |  22 |  23 |    |  32 |  33 |  34 |  35 |
   |   4 |   5 |   6 |   7 |    |  16 |  17 |  18 |  19 |
   |   0 |   1 |   2 |   3 |
*/






/* Utility function to fetch a particular face of the cube. */
static inline Face *
get_face (struct cube *cube, int block, int face)
{
  return cube->blocks[block].face + face;
}

/* Set the centre point of the face to (x0,  x1,  x2,  x3). */
static inline void
set_face_centre (Block *block,
		 int face,
		 GLfloat x0, GLfloat x1,
		 GLfloat x2, GLfloat x3)
{
  point *centre = &block->face[face].centre;

  (*centre)[0] = x0;
  (*centre)[1] = x1;
  (*centre)[2] = x2;
  (*centre)[3] = x3;
}

/* During the construction of the array of blocks,  we start by assigning them
   coordinates with components (0,  1,  2,  ...),  and then transform them to the
   range (-(cube_size-1),  -(cube_size-2),  ...,  0,  1,  2,  ...,  cube_size-1); this
   function performs the transformation. Later on we will also be needing the
   inverse transformation. */
static inline int
block_index_to_coords (const struct cube *cube, int i, int dim)
{
  return 2 * i - (cube->size[dim] - 1);
}

static inline int
block_coords_to_index (const struct cube *cube, double i, int dim)
{
  return (int) ((i + (cube->size[dim] - 1)) / 2.0);
}


/* The constructor itself. */
struct cube *
new_cube (int s0, int s1, int s2)
{
  /* Looping variables. */
  int i, k;

  /* The object that is returned to the caller. */
  struct cube *ret;

  /* Allocate memory,  and initialize constant members of the cube. */

  if (s1 <= 0 || s1 <= 0 || s2 <= 0)
    return NULL;

  if (NULL == (ret = malloc (sizeof (struct cube))))
    return NULL;

  ret->size[0] = s0;
  ret->size[1] = s1;
  ret->size[2] = s2;

  ret->number_blocks = ret->size[0] * ret->size[1] * ret->size[2];

  if (NULL == (ret->blocks = calloc (ret->number_blocks, sizeof (Block))))
    {
      perror ("Error allocating blocks");
      free (ret);
      return NULL;
    }

  /* Loop over the array of blocks,  and initialize each one. */
  for (i = 0; i < ret->number_blocks; ++i)
    {
      Block *block = ret->blocks + i;

      /* Flagging only certain faces as visible allows us to avoid rendering
         invisible surfaces,  thus slowing down animation. */

      block->visible_faces
	= (FACE_0 * (0 == i / (cube_get_size (ret, 0) * cube_get_size (ret, 1))))
	| (FACE_1 * (cube_get_size (ret,2) - 1 == i / (cube_get_size (ret, 0) * cube_get_size (ret, 1))))
	| (FACE_2 * (0 == i / cube_get_size (ret, 0) % cube_get_size (ret, 1)))
	| (FACE_3 * (cube_get_size (ret, 1) - 1 == i / cube_get_size (ret, 0) % cube_get_size (ret, 1)))
	| FACE_4 * (0 == i % cube_get_size (ret, 0))
	| FACE_5 * (cube_get_size (ret, 0) - 1 == i % cube_get_size (ret, 0))
	;

      /* Initialize all transformations to the identity matrix,  then set the
         translation part to correspond to the initial position of the
         block. */

      for (k = 0; k < 12; ++k)
	block->transformation[k] = 0;
      for (k = 0; k < 4; ++k)
	block->transformation[5 * k] = 1;

      block->transformation[12] =
	block_index_to_coords (ret, i % ret->size[0], 0);

      block->transformation[13] =
	block_index_to_coords (ret, (i / ret->size[0]) % ret->size[1], 1);

      block->transformation[14]
	= block_index_to_coords (ret, (i / (ret->size[0] * ret->size[1])) 
				 % ret->size[2], 2);


      /* Set all the face centres. */

      set_face_centre (block, 0, 0, 0, -1, 0);
      set_face_centre (block, 1, 0, 0, 1, 0);
      set_face_centre (block, 2, 0, -1, 0, 0);
      set_face_centre (block, 3, 0, 1, 0, 0);
      set_face_centre (block, 4, -1, 0, 0, 0);
      set_face_centre (block, 5, 1, 0, 0, 0);

    }				/* End of loop over blocks. */

  return ret;

}				/* End of function new_cube (). */




/*
 A cube object destructor. Provided the memory has not been corrupted,
 nothing can go wrong with this.
*/
void
free_cube (struct cube *cube)
{
  if (cube)
    {
      free (cube->blocks);

      free (cube);
    }
}








/* Quick cosine function for angles expressed in quarters of complete
   revolutions. */
static inline int
cos_quadrant (int quarters)
{
  switch (abs (quarters) % 4)
    {
    case 0:
      return 1;
    case 2:
      return -1;
    default:
      return 0;
    }
}

/* Quick sine function,  for angles expressed in quarters of complete
   revolutions. */
static inline int
sin_quadrant (int quarters)
{
  return cos_quadrant (quarters - 1);
}

/* Rotate the slice identified by a prior call to identify_blocks,  about the
   axis,  through an angle specified by turns,  which is in quarters of complete
   revolutions. */
void
rotate_slice (struct cube *cube, const struct move_data *md)
{
  int turns = md->turns;

  /* Iterator for array of blocks in the current slice. */
  const int *i;

  /* Create a matrix describing the rotation of we are about to perform.
     We do this by starting with the identity matrix... */
  Matrix rotation = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };
 
  /* Rotating backward 90 deg is the same as forward by 270 deg */
  if (md->dir == 0 && md->turns == 1)
    turns = 3;

  /* ... and then assigning values to the active elements. */

  rotation[(md->axis + 1) % 3 + 4 * ((md->axis + 1) % 3)]
    = cos_quadrant (turns);

  rotation[(md->axis + 2) % 3 + 4 * ((md->axis + 2) % 3)]
    = cos_quadrant (turns);

  rotation[(md->axis + 1) % 3 + 4 * ((md->axis + 2) % 3)]
    = sin_quadrant (turns);

  rotation[(md->axis + 2) % 3 + 4 * ((md->axis + 1) % 3)]
    = -sin_quadrant (turns);

  /* Apply the rotation matrix to all the blocks in this slice. We iterate
     backwards to avoid recalculating the end of the loop with every
     iteration. */
  for (i = md->blocks_in_motion->blocks + md->blocks_in_motion->number_blocks - 1;
       i >= md->blocks_in_motion->blocks; --i)
    pre_mult (rotation, cube->blocks[*i].transformation);
}
/* End of function rotate_slice (). */




/*
  Return a pointer to an array of block numbers which are in the slice
  identified by slice_depth (two publicly-exposed wrappers defined below allow
  for a convenient means of computing this),  and axis (an integer in [0,
  3)). The return value should eventually be destroyed with
  free_slice_blocks ().
*/

Slice_Blocks *
identify_blocks_2 (const struct cube * cube,
		   GLfloat slice_depth, int axis)
{
  /* Looping variables. */
  int dim, j = 0;
  Block *i;

  /* Allocate memory for the return object. */

  Slice_Blocks *ret = malloc (sizeof (Slice_Blocks));

  if (!ret)
    return NULL;


  ret->number_blocks = 1;
  for (dim = 0; dim < 3; ++dim)
    {
      if ( dim != axis)
	ret->number_blocks *= cube->size[dim];
    }

  ret->blocks = malloc (ret->number_blocks * sizeof (int));

  if (!ret->blocks)
    {
      free (ret);
      return NULL;
    }


  /* Iterate over all the blocks in the cube. When we find one whose
     axis-component of the location part of its transformation matrix
     corresponds to the requested slice depth,  then we make a note of its
     offset in the return->blocks array. */

  for (i = cube->blocks + cube->number_blocks - 1; i >= cube->blocks; --i)
    if (fabs (i->transformation[12 + axis] - slice_depth) < 0.1)
      ret->blocks[j++] = i - cube->blocks;

  return ret;
}


/* Get the blocks in the same slice as the block with block_id. */

Slice_Blocks *
identify_blocks (const struct cube * cube, int block_id, int axis)
{
  return identify_blocks_2
    (cube, cube->blocks[block_id].transformation[12 + axis], axis);
}






/*
 Release the resources associated with the Slice_Block.
*/
void
free_slice_blocks (Slice_Blocks * slice)
{
  if (slice)
    {
      free (slice->blocks);

      free (slice);
    }
}




/*
 Set the normal vector for block/face.
*/
void
set_normal_vector (struct cube *cube,
		   int block, int face, const vector v)
{
  Matrix view;
  vector *dest = &get_face (cube, block, face)->normal;

  glGetFloatv (GL_MODELVIEW_MATRIX, view);

  transform (view, v, *dest);
}




/*
 Set the vector for block/face/quadrant to v.
*/
void
set_quadrant_vector (struct cube *cube,
		     int block,
		     int face, int quadrant, const vector v)
{
  Matrix view;
  vector *dest = get_face (cube, block, face)->quadrants + quadrant;

  glGetFloatv (GL_MODELVIEW_MATRIX, view);

  transform (view, v, *dest);
}




/* Return the quadrant vector in v. */
void
get_quadrant_vector (const struct cube *cube,
		     int block,
		     int face, int quadrant, vector v)
{
  memcpy (v,
	  get_face ((struct cube * const) cube, block,
		    face)->quadrants + quadrant, sizeof (vector));
}




/*
 The cube is solved iff for all faces the quadrant vectors point in the same
 direction.  If however all the normals point in the same direction,  but the
 quadrants do not,  then the colours are all on the right faces,  but not
 correctly orientated.
*/
enum Cube_Status
cube_get_status (const struct cube *cube)
{
  /* Loop variables for iterating over faces and blocks. */

  int face;
  Block *block;


  /* Find out if the cube is at least half solved (the colours are right,  but
     some orientations are wrong (this can be seen on a face away from an edge
     of the cube with a pixmap on it). If the cube is not at least
     half-solved,  then it is definitely unsolved and this value is
     returned. */

  for (face = 0; face < 6; ++face)
    {
      vector q0;
      const unsigned int mask = 0x01 << face;
      int x = 0;

      for (block = cube->blocks + cube->number_blocks - 1;
	   block >= cube->blocks; --block)
	{
	  if (block->visible_faces & mask)
	    {
	      vector *v0 = &block->face[face].normal;

	      if (x == 0)
		{
		  memcpy (q0, v0, sizeof (vector));

		  ++x;
		}

	      else
		{
		  if (!vectors_equal (q0, *v0))
		    return NOT_SOLVED;
		}
	    }
	}
    }


  /* The cube is at least half-solved. Check if it is fully solved by checking
     the alignments of all the quadrant vectors. If any are out,  then return
     the half-solved status to the caller. Note that it is only necessary to
     check two perpendicular quadrant vectors. */

  for (face = 0; face < 6; ++face)
    {
      vector q1;
      vector q0;
      const unsigned int mask = 0x01 << face;
      int x = 0;

      for (block = cube->blocks + cube->number_blocks - 1;
	   block >= cube->blocks; --block)
	{
	  /* Ignore faces which are inside the cube. */
	  if (block->visible_faces & mask)
	    {
	      vector *v0 = block->face[face].quadrants;
	      vector *v1 = v0 + 1;

	      if (x == 0)
		{
		  memcpy (q0, v0, sizeof (vector));
		  memcpy (q1, v1, sizeof (vector));

		  ++x;
		}

	      else if (!vectors_equal (q0, *v0) || !vectors_equal (q1, *v1))
		return HALF_SOLVED;
	    }
	}
    }

  /* struct cube is fully solved. */
  return SOLVED;
}





int
cube_get_number_of_blocks (const struct cube *cube)
{
  return cube->number_blocks;
}

int
cube_get_size (const struct cube *cube, int dim)
{
  assert (dim >= 0);
  assert (dim < 3);
  return cube->size[dim];
}



/*
 Get the transformation of block number `block_id' from the origin,  and store
 it in transform. Return 0 on success,  1 on error.
*/
int
get_block_transform (const struct cube *cube,
		     int block_id, Matrix transform)
{
  memcpy (transform, cube->blocks[block_id].transformation, sizeof (Matrix));

  return 0;
}


void
cube_scramble (struct cube *cube)
{
  int i;

  for (i = 0; i < 2 * cube_get_number_of_blocks (cube); i++)
    {
      int turns = rand () % 2 + 1;
      const int axis = rand () % 3;
      Slice_Blocks *slice =
	identify_blocks (cube, rand () % cube_get_number_of_blocks (cube), axis);

      /* Insist upon 180 degree turns if the section is non-square */
      if ( !cube_square_axis (cube, axis))
	turns = 2;

      //      rotate_slice (cube, turns, 0, axis, slice);

      free_slice_blocks (slice);
    }
}


/* Return true iff the section orthogonal to the axis is square. */
bool
cube_square_axis (const struct cube *cube, int axis)
{
  int other_axis_size = -1;
  int j;
  for (j = 0; j  < 3 ; ++j)
    {
      if (j == axis)
	continue;
	
      if ( other_axis_size != -1 &&
	   other_axis_size != cube_get_size (cube, j))
	return false;
      other_axis_size = cube_get_size (cube, j);
    }
  return true;
}






/*
 Manufacture a SCM object which is a vector of six vectors of (cube_size *
 cube_size) elements,  each one holding the colour of a patch of the surface of
 the cube (a number from [0, 5]).
*/
SCM
make_scm_cube (const struct cube * cube)
{
#if 0
  SCM scm_face_vector;
  Block *block;
  int face;
  char colours[6][cube->cube_size][cube->cube_size];

  /* Loop over all blocks and faces,  but only process if the face is on the
     outside of the cube. */

  for (block = cube->blocks + cube->number_blocks - 1;
       block >= cube->blocks; --block)

    for (face = 0; face < 6; ++face)

      if (block->visible_faces & (0x01 << face))
	{
	  /* Apply the rotation part of the block transformation to the
	     offset of the face centre from the centre of the block. This
	     will tell us the direction the face is now facing. */

	  point face_direction;

	  memcpy (face_direction, block->face[face].centre, sizeof (point));

	  transform (block->transformation, face_direction);


	  /* The face direction will have exactly one non-zero component;
	     this is in the direction of the normal,  so we can infer which
	     side of the cube we are on. Further,  if we look at the other
	     two components of the location part of the block
	     transformation,  we can infer the position of the block in the
	     face. We set the appropriate element in the colours array to
	     the colour of this face,  which corresponds exactly with the
	     original face the block was at. */

	  if (abs (face_direction[0]) > 0.1)
	    colours[face_direction[0] < 0.0 ? 4 : 5]
	      [block_coords_to_index
	       (cube, block->transformation[13])]
	      [block_coords_to_index
	       (cube, block->transformation[14])] = face;

	  else if (abs (face_direction[1]) > 0.1)
	    colours[face_direction[1] < 0.0 ? 2 : 3]
	      [block_coords_to_index
	       (cube, block->transformation[12])]
	      [block_coords_to_index
	       (cube, block->transformation[14])] = face;

	  else
	    colours[face_direction[2] < 0.0 ? 0 : 1]
	      [block_coords_to_index
	       (cube, block->transformation[12])]
	      [block_coords_to_index
	       (cube, block->transformation[13])] = face;
	}


  /* Now the colours array should be completely populated,  we can manufacture
     our scheme object. */

  scm_face_vector = scm_c_make_vector (6, SCM_UNSPECIFIED);

  for (face = 0; face < 6; ++face)
    {
      int i, j;

      SCM scm_block_vector
	= scm_c_make_vector (cube->cube_size * cube->cube_size,
			     SCM_UNSPECIFIED);

      for (i = 0; i < cube->cube_size; ++i)
	for (j = 0; j < cube->cube_size; ++j)
	  scm_vector_set_x (scm_block_vector,
			    scm_from_int (i + cube->cube_size * j),
			    scm_from_int (colours[face][i][j]));

      scm_vector_set_x (scm_face_vector,
			scm_from_int (face), scm_block_vector);
    }

  return scm_cons (scm_list_5 (scm_from_int (1),
			       scm_from_int (3),
			       scm_from_int (cube->cube_size),
			       scm_from_int (cube->cube_size),
			       scm_from_int (cube->cube_size)),
		   scm_face_vector);

#endif

  return SCM_UNDEFINED;
}
