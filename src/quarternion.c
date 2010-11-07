/*

    A little library to do quarternion arithmetic
    Copyright (C) 2003  John Darrington

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
static const char RCSID[]="$Id: quarternion.c,v 1.2 2008/01/16 10:35:13 jmd Exp $";

#include <config.h>
#include "quarternion.h"
#include "txfm.h"

#include <math.h>
#include <stdio.h>

void
quarternion_set_to_unit (Quarternion *q)
{
  q->w = 1;
  q->x = 0;
  q->y = 0;
  q->z = 0;
}

void
quarternion_to_matrix (Matrix M,  const Quarternion *q)
{

  int i;
  for (i = 0 ; i < MATRIX_DIM*MATRIX_DIM ; ++i )
    M[i]=99;

  /* Diagonals ... */
  set (M, 0, 0,
      q->w * q->w +
      q->x * q->x -
      q->y * q->y -
      q->z * q->z );


  set (M, 1, 1,
      q->w * q->w -
      q->x * q->x +
      q->y * q->y -
      q->z * q->z );


  set (M, 2, 2,
      q->w * q->w -
      q->x * q->x -
      q->y * q->y +
      q->z * q->z );


  set (M,  MATRIX_DIM-1,  MATRIX_DIM-1,
      q->w * q->w +
      q->x * q->x +
      q->y * q->y +
      q->z * q->z );


  /* Last row */

  set (M, 0,  MATRIX_DIM -1, 0.0);
  set (M, 1,  MATRIX_DIM -1, 0.0);
  set (M, 2,  MATRIX_DIM -1, 0.0);

  /* Last Column */

  set (M,  MATRIX_DIM -1, 0, 0.0);
  set (M,  MATRIX_DIM -1, 1, 0.0);
  set (M,  MATRIX_DIM -1, 2, 0.0);


  /* Others */

  set (M, 0, 1,
      2*q->x*q->y + 2*q->w * q->z);


  set (M, 0, 2,
      2*q->x*q->z - 2*q->w * q->y);

  set (M, 1, 2,
      2*q->y*q->z + 2*q->w * q->x);




  set (M, 1, 0,
      2*q->x*q->y - 2*q->w * q->z);


  set (M, 2, 0,
      2*q->x*q->z + 2*q->w * q->y);


  set (M, 2, 1,
      2*q->y*q->z - 2*q->w * q->x);

}

void
quarternion_from_rotation (Quarternion *q,  vector u,  float theta)
{
  float radians = theta * M_PI / 180.0;

  float s = cos (radians/2.0);

  vector v;
  int i;

  for ( i = 0 ; i < MATRIX_DIM; ++i ) {
    v[i] = u[i] * sin (radians/2.0);
  }

  q->w = s;
  q->x = v[0];
  q->y = v[1];
  q->z = v[2];

}


void
quarternion_print (const Quarternion *q)
{
  printf ("(%0.2f,  %0.2f,  %0.2f,  %0.2f)\n",  q->w,  q->x,  q->y,  q->z);
}


/* Pre multiply q1 by q2 */
void
quarternion_pre_mult (Quarternion *q1,  const Quarternion *q2)
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

  dot_product = q1->x*q2->x +
    q1->y*q2->y +
    q1->z*q2->z ;

  /*
  printf ("Dot product is %f\n",  dot_product);
  */

  q1->w = q1->w * q2->w ;

  q1->w -= dot_product;


  cross_product[0] = q1->y*q2->z - q1->z*q2->y;
  cross_product[1] = q1->z*q2->x - q1->x*q2->z;
  cross_product[2] = q1->x*q2->y - q1->y*q2->x;

  /*
  printf ("Cross product is %f,  %f,  %f\n",  cross_product[0],
	 cross_product[1],
	 cross_product[2]);
  */

  q1->x = s1*q2->x + s2*q1->x + cross_product[0];
  q1->y = s1*q2->y + s2*q1->y + cross_product[1];
  q1->z = s1*q2->z + s2*q1->z + cross_product[2];

}


