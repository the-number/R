/*
    Copyright (c) 2004        Dale Mellor,  John Darrington
                  1998, 2003, 2009, 2011  John Darrington

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


#ifndef __GBK_CUBE_H__
#define __GBK_CUBE_H__

#include "move.h"
#include "quarternion.h"

/* Is the cube solved? */
enum cube_status
{
  NOT_SOLVED = 0,
  SOLVED,
  HALF_SOLVED
};

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

#include <glib-object.h>
#include "cube_i.h"

#define GBK_TYPE_CUBE                  (gbk_cube_get_type ())
#define GBK_CUBE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GBK_TYPE_CUBE, GbkCube))
#define GBK_IS_CUBE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GBK_TYPE_CUBE))
#define GBK_CUBE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GBK_TYPE_CUBE, GbkCubeClass))
#define GBK_IS_CUBE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GBK_TYPE_CUBE))
#define GBK_CUBE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GBK_TYPE_CUBE, GbkCubeClass))

typedef struct _GbkCube GbkCube;
typedef struct _GbkCubeClass GbkCubeClass;

struct _GbkCube
{
  GObject parent_instance;

  /* instance members */

  /* The number of blocks on each side of the cube */
  int size[3];

  /* cube_size ** 3 */
  int number_blocks;

  /* A set of attributes for every block (including internal ones!) */
  Block *blocks;

  Quarternion orientation;
};

struct _GbkCubeClass
{
  GObjectClass parent_class;

  /* class members */
};

/* used by GBK_TYPE_CUBE */
GType gbk_cube_get_type (void);

/*
 * Method definitions.
 */

/* Get a copy of the tranformation of the specified block. The return value is
   zero on success,  one otherwise. */
int gbk_cube_get_block_transform (const GbkCube * cube, int block,
				  Matrix transform);

int gbk_cube_get_size (const GbkCube * cube, int dim);

void gbk_cube_scramble (GbkCube * cube);
int gbk_cube_get_number_of_blocks (const GbkCube * cube);
void gbk_cube_rotate_slice (GbkCube * cube, const struct move_data *md);

gboolean gbk_cube_square_axis (const GbkCube * cube, int axis);


/* Create a new cube object with the given number of faces per side. */
GObject *gbk_cube_new (int, int, int);

void
gbk_cube_set_quadrant_vector (GbkCube * cube,
			      int block,
			      int face, int quadrant, const vector v);

void gbk_cube_get_quadrant_vector (const GbkCube * cube,
				   int block,
				   int face, int quadrant, vector v);

enum cube_status gbk_cube_get_status (const GbkCube * cube);


/* Free the resources associated with an instance of the above structure. */
void free_slice_blocks (Slice_Blocks * slice_blocks);

unsigned int gbk_cube_get_visible_faces (const GbkCube * cube, int block_id);

void gbk_cube_set_size (GbkCube * ret, int s0, int s1, int s2);

void gbk_cube_rotate (GbkCube * cube, const vector v, gfloat step);


#include <libguile.h>

SCM make_scm_cube (const GbkCube * cube);

#endif /* __CUBE_H__ */
