/*
    Copyright (c) 2004        Dale Mellor,  John Darrington
                  1998,  2003  John Darrington
		  2009   John Darrington

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

#ifndef CUBE_H
#define CUBE_H


#include "cube_i.h"


/* Cube co-ordinates have their origin at the centre of the cube,  and their
   units are equivalent to one half of the length of one edge of a block. */



/* Unique identifiers for the faces,  which can be used to populate a
   bit-field. */
enum
{
  FACE_0 = 0x01 << 0,
  FACE_1 = 0x01 << 1,
  FACE_2 = 0x01 << 2,
  FACE_3 = 0x01 << 3,
  FACE_4 = 0x01 << 4,
  FACE_5 = 0x01 << 5
};


/* Create a new cube object with the given number of faces per side. */
struct cube *new_cube (int, int, int);


/* Free up memory used by the cube. */
void free_cube (struct cube *cube);


/* Is the cube solved? */
enum Cube_Status
{ NOT_SOLVED = 0, SOLVED, HALF_SOLVED };

enum Cube_Status cube_get_status (const struct cube *cube);


/* Structure used to communicate which blocks of the cube lie in a certain
   slice. These objects should only be constructed by one of the identify_blocks
   functions,  and must be destroyed with the free_slice_blocks function. */
typedef struct _Slice_Blocks
{
  int *blocks;
  int number_blocks;
  int axis;

} Slice_Blocks;


/* Free the resources associated with an instance of the above structure. */
void free_slice_blocks (Slice_Blocks * slice_blocks);


/* Return the identity of all the blocks in a particular slice (or the one
   containing the block with block_id). The return object must be freed with
   free_slice_blocks. Axis is in the interval [0,  3). */
Slice_Blocks *identify_blocks (const struct cube *cube,
			       int block_id, int axis);

Slice_Blocks *identify_blocks_2 (const struct cube *cube,
				 GLfloat slice, int axis);

/*  Rotate a complete slice,  as identified by a prior call to
    identify_blocks. The return value is zero on success,  one otherwise. The
    slice_blocks object must have been previously created with one of the
    identify_blocks functions. */
int rotate_slice (struct cube *cube,
		  int turns, const Slice_Blocks *slice_blocks);


/* Get a copy of the tranformation of the specified block. The return value is
   zero on success,  one otherwise. */
int get_block_transform (const struct cube *cube,
			 int block, Matrix transform);


/* Return an int identifying all the visible faces in this block. */
unsigned int get_visible_faces (const struct cube *cube, int block);


/* Get a copy of the centre point of the face into p. */
void get_face_centre (const struct cube *cube,
		      int block, int face, point p);


/* Set the normal vector for block/face to v. This value gets transformed by
   the MODELVIEW CTM before saving. */
void set_normal_vector (struct cube *cube,
			int block, int face, const vector v);


/* Set the vector for block/face/quadrant to v. This value gets transformed by
   the MODELVIEW CTM before saving. */
void set_quadrant_vector (struct cube *cube,
			  int block,
			  int face, int quadrant, const vector v);


/* Get a copy of the direction vector for the block/face/quadrant. */
void get_quadrant_vector (const struct cube *cube,
			  int block,
			  int face, int quadrant, vector v);


int cube_get_number_of_blocks (const struct cube *cube);
int cube_get_size (const struct cube *cube, int dim);

void cube_scramble (struct cube *cube);


#include <libguile.h>

/* Create a scheme object which represents the current state of the given
   cube. */
SCM make_scm_cube (const struct cube *cube);


/***
    Concession to the non-object-oriented universe (this will go eventually).
***/
extern struct cube *the_cube;

int create_the_cube (int, int, int);

void destroy_the_cube (void);



#endif /* CUBE_H */
