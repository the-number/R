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
#include "cube.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <libguile.h>

static inline int cos_quadrant (int quarters);
static inline int sin_quadrant (int quarters);

static int block_coords_to_index (const GbkCube * cube, int x, int dim);

/*
  Manufacture a SCM object which is a vector of six vectors of (cube_size *
  cube_size) elements,  each one holding the colour of a patch of the surface of
  the cube (a number from [0, 5]).
*/
SCM
make_scm_cube (const GbkCube * cube)
{
  Block *block;

  /* We can manufacture  our scheme object. */
  SCM scm_face_vector = scm_c_make_vector (6, SCM_UNSPECIFIED);

  int biggest = 0;
  int li = -1;
  int i;
  for (i = 0; i < 3; ++i)
    {
      if (biggest < gbk_cube_get_size (cube, i))
	{
	  biggest = gbk_cube_get_size (cube, i);
	  li = i;
	}
    }

  int colours[6][biggest][biggest];
  memset (colours, -1, 6 * biggest * biggest);

  /* Loop over all blocks and faces,  but only process if the face is on the
     outside of the cube. */
  for (block = cube->blocks + cube->number_blocks - 1;
       block >= cube->blocks; --block)
    {
      int face;
      for (face = 0; face < 6; ++face)
	{
	  if (!(block->visible_faces & (0x01 << face)))
	    continue;

	  /* Apply the rotation part of the block transformation to the
	     offset of the face centre from the centre of the block. This
	     will tell us the direction the face is now facing. */

	  point face_direction;
	  vector_transform (face_direction, block->face[face].normal,
			    block->transformation);

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
	       (cube, block->transformation[13], 1)]
	      [block_coords_to_index
	       (cube, block->transformation[14], 2)] = face;
	  else if (abs (face_direction[1]) > 0.1)
	    colours[face_direction[1] < 0.0 ? 2 : 3]
	      [block_coords_to_index
	       (cube, block->transformation[12], 0)]
	      [block_coords_to_index
	       (cube, block->transformation[14], 2)] = face;
	  else
	    colours[face_direction[2] < 0.0 ? 0 : 1]
	      [block_coords_to_index
	       (cube, block->transformation[12], 0)]
	      [block_coords_to_index
	       (cube, block->transformation[13], 1)] = face;
	}
    }


  int face;
  for (face = 0; face < 6; ++face)
    {
      int dimA, dimB;
      {
	switch (face)
	  {
	  case 0:
	  case 1:
	    dimA = 0;
	    dimB = 1;
	    break;
	  case 2:
	  case 3:
	    dimA = 0;
	    dimB = 2;
	    break;
	  case 4:
	  case 5:
	    dimA = 1;
	    dimB = 2;
	    break;
	  }
      }

      int i, j;
      SCM scm_block_vector
	= scm_c_make_vector (gbk_cube_get_size (cube, dimA) *
			     gbk_cube_get_size (cube, dimB),
			     SCM_UNSPECIFIED);

      for (i = 0; i < gbk_cube_get_size (cube, dimA); ++i)
	for (j = 0; j < gbk_cube_get_size (cube, dimB); ++j)
	  scm_vector_set_x (scm_block_vector,
			    scm_from_int (i +
					  gbk_cube_get_size (cube, dimA) * j),
			    scm_from_int (colours[face][i][j]));

      scm_vector_set_x (scm_face_vector,
			scm_from_int (face), scm_block_vector);

    }

  return scm_cons (scm_list_5 (scm_from_int (1),
			       scm_from_int (3),
			       scm_from_int (gbk_cube_get_size (cube, 0)),
			       scm_from_int (gbk_cube_get_size (cube, 1)),
			       scm_from_int (gbk_cube_get_size (cube, 2))),
		   scm_face_vector);
}






/* Set the normal of the face to (x0,  x1,  x2). */
static inline void
set_face_normal (Block * block, int face, GLfloat x0, GLfloat x1, GLfloat x2)
{
  point *normal = &block->face[face].normal;

  (*normal)[0] = x0;
  (*normal)[1] = x1;
  (*normal)[2] = x2;
  (*normal)[3] = 0;
}


/* Set the normal of the face to (x0,  x1,  x2). */
static inline void
set_face_up (Block * block, int face, GLfloat x0, GLfloat x1, GLfloat x2)
{
  point *up = &block->face[face].up;

  (*up)[0] = x0;
  (*up)[1] = x1;
  (*up)[2] = x2;
  (*up)[3] = 0;
}


/* During the construction of the array of blocks,  we start by assigning them
   coordinates with components (0,  1,  2,  ...),  and then transform them to the
   range (-(cube_size-1),  -(cube_size-2),  ...,  0,  1,  2,  ...,  cube_size-1); this
   function performs the transformation. Later on we will also be needing the
   inverse transformation. */
static inline int
block_index_to_coords (const GbkCube * cube, int i, int dim)
{
  return 2 * i - (cube->size[dim] - 1);
}

/* The inverse of the above */
static int
block_coords_to_index (const GbkCube * cube, int x, int dim)
{
  return (x + cube->size[dim] - 1) / 2;
}



static void
gbk_cube_init (GbkCube * ret)
{
  ret->blocks = NULL;
  quarternion_set_to_unit (&ret->orientation);
}

void
gbk_cube_set_size (GbkCube * cube, int s0, int s1, int s2)
{
  gint s[3];

  s[0] = s0;
  s[1] = s1;
  s[2] = s2;

  g_object_set (cube, "dimensions", s, NULL);
}

static void
cube_set_size (GbkCube * ret, int s0, int s1, int s2)
{
  int i;
  ret->size[0] = s0;
  ret->size[1] = s1;
  ret->size[2] = s2;

  ret->number_blocks = ret->size[0] * ret->size[1] * ret->size[2];

  g_free (ret->blocks);
  if (NULL == (ret->blocks = g_malloc (ret->number_blocks * sizeof (Block))))
    {
      g_error ("Error allocating blocks");
      return;
    }

  /* Loop over the array of blocks,  and initialize each one. */
  for (i = 0; i < ret->number_blocks; ++i)
    {
      int k;
      Block *block = ret->blocks + i;

      /* Flagging only certain faces as visible allows us to avoid rendering
         invisible surfaces,  thus slowing down animation. */

      block->visible_faces
	=
	(FACE_0 *
	 (0 ==
	  i / (gbk_cube_get_size (ret, 0) *
	       gbk_cube_get_size (ret,
				  1)))) | (FACE_1 * (gbk_cube_get_size (ret,
									2) -
						     1 ==
						     i /
						     (gbk_cube_get_size
						      (ret,
						       0) *
						      gbk_cube_get_size (ret,
									 1))))
	| (FACE_2 *
	   (0 ==
	    i / gbk_cube_get_size (ret, 0) % gbk_cube_get_size (ret,
								1))) | (FACE_3
									*
									(gbk_cube_get_size
									 (ret,
									  1) -
									 1 ==
									 i /
									 gbk_cube_get_size
									 (ret,
									  0) %
									 gbk_cube_get_size
									 (ret,
									  1)))
	| FACE_4 * (0 ==
		    i % gbk_cube_get_size (ret,
					   0)) | FACE_5 *
	(gbk_cube_get_size (ret, 0) - 1 == i % gbk_cube_get_size (ret, 0));

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


      /* Set all the face normals. */
      set_face_normal (block, 0, 0, 0, -1);
      set_face_normal (block, 1, 0, 0, 1);
      set_face_normal (block, 2, 0, -1, 0);
      set_face_normal (block, 3, 0, 1, 0);
      set_face_normal (block, 4, -1, 0, 0);
      set_face_normal (block, 5, 1, 0, 0);

      set_face_up (block, 0, 1, 0, 0);
      set_face_up (block, 1, 1, 0, 0);
      set_face_up (block, 2, 1, 0, 0);
      set_face_up (block, 3, 1, 0, 0);
      set_face_up (block, 4, 0, 1, 0);
      set_face_up (block, 5, 0, 1, 0);


    }				/* End of loop over blocks. */
}


enum
{
  PROP_0 = 0,
  PROP_DIMENSIONS
};

static void
cube_set_property (GObject * object,
		   guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GbkCube *cube = GBK_CUBE (object);

  switch (prop_id)
    {
    case PROP_DIMENSIONS:
      {
	gint *s = g_value_get_pointer (value);

	cube_set_size (cube, s[0], s[1], s[2]);
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    };
}


static void
cube_get_property (GObject * object,
		   guint prop_id, GValue * value, GParamSpec * pspec)
{
  GbkCube *cube = GBK_CUBE (object);

  switch (prop_id)
    {
    case PROP_DIMENSIONS:
      g_value_set_pointer (value, cube->size);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    };
}


enum
{
  MOVED,
  ROTATED,
  n_SIGNALS
};

static guint signals[n_SIGNALS];


static void
gbk_cube_class_init (GbkCubeClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  GParamSpec *gbk_param_spec;

  gobject_class->set_property = cube_set_property;
  gobject_class->get_property = cube_get_property;

  gbk_param_spec = g_param_spec_pointer ("dimensions",
					 "Dimensions",
					 "An array of 3 ints specifying how many cubelets along each side of the cube",
					 G_PARAM_READWRITE);

  g_object_class_install_property (gobject_class,
				   PROP_DIMENSIONS, gbk_param_spec);


  signals[MOVED] =
    g_signal_new ("move",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  0,
		  NULL, NULL,
		  g_cclosure_marshal_VOID__BOXED,
		  G_TYPE_NONE, 1, move_get_type ());

  signals[ROTATED] =
    g_signal_new ("rotate",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  0,
		  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


G_DEFINE_TYPE (GbkCube, gbk_cube, G_TYPE_OBJECT);



int
gbk_cube_get_size (const GbkCube * cube, int dim)
{
  gint *s;
  g_assert (dim >= 0);
  g_assert (dim < 3);

  g_object_get ((GbkCube *) cube, "dimensions", &s, NULL);

  return s[dim];
}


void
gbk_cube_scramble (GbkCube * cube)
{
  int i;

  for (i = 0; i < 2 * gbk_cube_get_number_of_blocks (cube); i++)
    {
      struct move_data *move;
      const short axis = rand () % 3;
      int slice;
      const int size = gbk_cube_get_size (cube, axis);

      slice = rand () % size;
      slice *= 2;
      slice -= size - 1;

      move = move_create (slice, axis, 0);

      gbk_cube_rotate_slice (cube, move);

      move_unref (move);
    }
}


int
gbk_cube_get_number_of_blocks (const GbkCube * cube)
{
  return cube->number_blocks;
}


/*
  Get the transformation of block number `block_id' from the origin,  and store
  it in transform. Return 0 on success,  1 on error.
*/
int
gbk_cube_get_block_transform (const GbkCube * cube,
			      int block_id, Matrix transform)
{
  memcpy (transform, cube->blocks[block_id].transformation, sizeof (Matrix));

  return 0;
}


GObject *
gbk_cube_new (int x, int y, int z)
{
  gint s[3];

  s[0] = x;
  s[1] = y;
  s[2] = z;

  return g_object_new (gbk_cube_get_type (), "dimensions", s, NULL);
}


/* Utility function to fetch a particular face of the cube. */
static inline Face *
gbk_cube_get_face (GbkCube * cube, int block, int face)
{
  return cube->blocks[block].face + face;
}

/*
  Set the vector for block/face/quadrant to v.
*/
void
gbk_cube_set_quadrant_vector (GbkCube * cube,
			      int block,
			      int face, int quadrant, const vector v)
{
  Matrix view;
  vector *dest = gbk_cube_get_face (cube, block, face)->quadrants + quadrant;

  glGetFloatv (GL_MODELVIEW_MATRIX, view);

  vector_transform (*dest, v, view);
}

static Slice_Blocks *cube_identify_blocks_2 (const GbkCube * cube,
					     GLfloat slice_depth, int axis);


/* Rotate the CUBE according to move M */
void
gbk_cube_rotate_slice (GbkCube * cube, const struct move_data *m)
{
  struct move_data *md = move_copy (m);
  int turns;

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

  /* If rotating about a non-square axis, then only 180 deg turns are
     permitted */
  if (!gbk_cube_square_axis (cube, move_axis (md)))
    {
      move_set_turns (md, 2);
    }

  /* Rotating backward 90 deg is the same as forward by 270 deg */
  turns = move_turns (md);
  if (move_dir (md) == 0 && move_turns (md) == 1)
    {
      turns = 3;
    }

  /* ... and then assigning values to the active elements. */
  rotation[(move_axis (md) + 1) % 3 + 4 * ((move_axis (md) + 1) % 3)]
    = cos_quadrant (turns);

  rotation[(move_axis (md) + 2) % 3 + 4 * ((move_axis (md) + 2) % 3)]
    = cos_quadrant (turns);

  rotation[(move_axis (md) + 1) % 3 + 4 * ((move_axis (md) + 2) % 3)]
    = sin_quadrant (turns);

  rotation[(move_axis (md) + 2) % 3 + 4 * ((move_axis (md) + 1) % 3)]
    = -sin_quadrant (turns);


  if (NULL == md->blocks_in_motion)
    ((struct move_data *) md)->blocks_in_motion =
      cube_identify_blocks_2 (cube, md->slice, move_axis (md));

  g_assert (md->blocks_in_motion);


  /* Apply the rotation matrix to all the blocks in this slice. We iterate
     backwards to avoid recalculating the end of the loop with every
     iteration. */
  for (i =
       md->blocks_in_motion->blocks + md->blocks_in_motion->number_blocks - 1;
       i >= md->blocks_in_motion->blocks; --i)
    matrix_pre_mult (cube->blocks[*i].transformation, rotation);

  g_signal_emit (cube, signals[MOVED], 0, md);
  move_unref (md);
}

/* End of function rotate_slice (). */


/* Return true iff the section orthogonal to the axis is square. */
gboolean
gbk_cube_square_axis (const GbkCube * cube, int axis)
{
  int other_axis_size = -1;
  int j;
  for (j = 0; j < 3; ++j)
    {
      if (j == axis)
	continue;

      if (other_axis_size != -1 &&
	  other_axis_size != gbk_cube_get_size (cube, j))
	return FALSE;
      other_axis_size = gbk_cube_get_size (cube, j);
    }
  return TRUE;
}


/* Return the quadrant vector in v. */
void
gbk_cube_get_quadrant_vector (const GbkCube * cube,
			      int block, int face, int quadrant, vector v)
{
  memcpy (v,
	  gbk_cube_get_face ((GbkCube *) cube, block,
			     face)->quadrants + quadrant, sizeof (vector));
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

/*
  Return a pointer to an array of block numbers which are in the slice
  identified by slice_depth (two publicly-exposed wrappers defined below allow
  for a convenient means of computing this),  and axis (an integer in [0,
  3)). The return value should eventually be destroyed with
  free_slice_blocks ().
*/

static Slice_Blocks *
cube_identify_blocks_2 (const GbkCube * cube, GLfloat slice_depth, int axis)
{
  /* Looping variables. */
  int dim, j = 0;
  Block *i;

  /* Allocate memory for the return object. */

  Slice_Blocks *ret = g_malloc (sizeof *ret);

  ret->ref = 1;
  ret->number_blocks = 1;
  for (dim = 0; dim < 3; ++dim)
    {
      if (dim != axis)
	ret->number_blocks *= cube->size[dim];
    }

  ret->blocks = g_malloc (ret->number_blocks * sizeof (int));

  if (!ret->blocks)
    {
      g_free (ret);
      return NULL;
    }

  /* Iterate over all the blocks in the cube. When we find one whose
     axis-component of the location part of its transformation matrix
     corresponds to the requested slice depth,  then we make a note of its
     offset in the return->blocks array. */

  for (i = cube->blocks + cube->number_blocks - 1; i >= cube->blocks; --i)
    {
      if (fabs (i->transformation[12 + axis] - slice_depth) < 0.1)
	{
	  ret->blocks[j++] = i - cube->blocks;
	}
    }

  return ret;
}


/*
  The cube is solved iff for all faces the quadrant vectors point in the same
  direction.  If however all the normals point in the same direction,  but the
  quadrants do not,  then the colours are all on the right faces,  but not
  correctly orientated.
*/
enum cube_status
gbk_cube_get_status (const GbkCube * cube)
{
  /* Loop variables for iterating over faces and blocks. */

  int face;
  Block *block;

  gboolean directions_uniform = TRUE;

  /* Find out if the cube is at least half solved (the colours are right,  but
     some orientations are wrong (this can be seen on a face away from an edge
     of the cube with a pixmap on it). If the cube is not at least
     half-solved,  then it is definitely unsolved and this value is
     returned. */

  for (face = 0; face < 6; ++face)
    {
      vector q_n;
      vector q_u;
      const unsigned int mask = 0x01 << face;
      int x = 0;

      for (block = cube->blocks + cube->number_blocks - 1;
	   block >= cube->blocks; --block)
	{
	  if (block->visible_faces & mask)
	    {
	      vector v_n;
	      vector v_u;

	      if (x == 0)
		{
		  vector_transform (q_n, block->face[face].normal,
				    block->transformation);

		  if (directions_uniform)
		    vector_transform (q_u, block->face[face].up,
				      block->transformation);

		  ++x;
		}
	      else
		{
		  vector_transform (v_n, block->face[face].normal,
				    block->transformation);

		  if (directions_uniform)
		    vector_transform (v_u, block->face[face].up,
				      block->transformation);

		  if (!vectors_equal (q_n, v_n))
		    return NOT_SOLVED;

		  if (directions_uniform && !vectors_equal (q_u, v_u))
		    directions_uniform = FALSE;
		}
	    }
	}
    }

  /* struct cube is fully solved. */
  return directions_uniform ? SOLVED : HALF_SOLVED;
}


/* Return an int identifying all the visible faces in this block. */
unsigned int
gbk_cube_get_visible_faces (const GbkCube * cube, int block_id)
{
  return cube->blocks[block_id].visible_faces;
}



void
gbk_cube_rotate (GbkCube * cube, const vector v, gfloat step)
{
  Quarternion rot;
  quarternion_from_rotation (&rot, v, step);
  quarternion_pre_mult (&cube->orientation, &rot);

  g_signal_emit (cube, signals[ROTATED], 0);
}
