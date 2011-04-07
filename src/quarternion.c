/*
  A little library to do quarternion arithmetic
  Copyright (C) 2003, 2011  John Darrington

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
#include "quarternion.h"
#include "txfm.h"

#include <math.h>
#include <stdio.h>

void
quarternion_set_to_unit (Quarternion * q)
{
  q->w = 1;
  q->x = 0;
  q->y = 0;
  q->z = 0;
}

void
quarternion_to_matrix (Matrix M, const Quarternion * q)
{
  /* Diagonals ... */
  matrix_set (M, 0, 0, q->w * q->w + q->x * q->x - q->y * q->y - q->z * q->z);


  matrix_set (M, 1, 1, q->w * q->w - q->x * q->x + q->y * q->y - q->z * q->z);


  matrix_set (M, 2, 2, q->w * q->w - q->x * q->x - q->y * q->y + q->z * q->z);


  matrix_set (M, MATRIX_DIM - 1, MATRIX_DIM - 1,
	      q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);


  /* Last row */

  matrix_set (M, 0, MATRIX_DIM - 1, 0.0);
  matrix_set (M, 1, MATRIX_DIM - 1, 0.0);
  matrix_set (M, 2, MATRIX_DIM - 1, 0.0);

  /* Last Column */

  matrix_set (M, MATRIX_DIM - 1, 0, 0.0);
  matrix_set (M, MATRIX_DIM - 1, 1, 0.0);
  matrix_set (M, MATRIX_DIM - 1, 2, 0.0);


  /* Others */

  matrix_set (M, 0, 1, 2 * q->x * q->y + 2 * q->w * q->z);


  matrix_set (M, 0, 2, 2 * q->x * q->z - 2 * q->w * q->y);

  matrix_set (M, 1, 2, 2 * q->y * q->z + 2 * q->w * q->x);


  matrix_set (M, 1, 0, 2 * q->x * q->y - 2 * q->w * q->z);


  matrix_set (M, 2, 0, 2 * q->x * q->z + 2 * q->w * q->y);


  matrix_set (M, 2, 1, 2 * q->y * q->z - 2 * q->w * q->x);
}

void
quarternion_from_rotation (Quarternion * q, const vector u, float theta)
{
  const float radians = theta * M_PI / 180.0;

  q->w = cos (radians / 2.0);

  q->x = u[0] * sin (radians / 2.0);
  q->y = u[1] * sin (radians / 2.0);
  q->z = u[2] * sin (radians / 2.0);
}


void
quarternion_dump (const Quarternion * q)
{
  printf ("(%0.2f,  %0.2f,  %0.2f,  %0.2f)\n", q->w, q->x, q->y, q->z);
}


void
quarternion_get_inverse (Quarternion * inv, const Quarternion * q)
{
  float ssq = 0.0;
  ssq += q->w * q->w;
  ssq += q->x * q->x;
  ssq += q->y * q->y;
  ssq += q->z * q->z;

  inv->w = q->w / ssq;
  inv->x = -q->x / ssq;
  inv->y = -q->y / ssq;
  inv->z = -q->z / ssq;
}

/* Pre multiply q1 by q2 */
void
quarternion_pre_mult (Quarternion * q1, const Quarternion * q2)
{
  float dot_product;
  vector cross_product;
  float s1 = q1->w;
  float s2 = q2->w;

  /*
     printf ("Q mult\n");


     quarternion_print (q1);
     quarternion_print (q2);
   */

  dot_product = q1->x * q2->x + q1->y * q2->y + q1->z * q2->z;

  /*
     printf ("Dot product is %f\n",  dot_product);
   */

  q1->w = q1->w * q2->w;

  q1->w -= dot_product;


  cross_product[0] = q1->y * q2->z - q1->z * q2->y;
  cross_product[1] = q1->z * q2->x - q1->x * q2->z;
  cross_product[2] = q1->x * q2->y - q1->y * q2->x;

  /*
     printf ("Cross product is %f,  %f,  %f\n",  cross_product[0],
     cross_product[1],
     cross_product[2]);
   */

  q1->x = s1 * q2->x + s2 * q1->x + cross_product[0];
  q1->y = s1 * q2->y + s2 * q1->y + cross_product[1];
  q1->z = s1 * q2->z + s2 * q1->z + cross_product[2];
}
