/*

    A little library to do matrix arithmetic
    Copyright (C) 1998  John Darrington

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
#ifndef TXFM_H
#define TXFM_H

#include <GL/gl.h>

#define MATRIX_DIM 4

typedef GLfloat Matrix[MATRIX_DIM*MATRIX_DIM];
typedef GLfloat pv[MATRIX_DIM];
typedef pv point,  vector;


/* Multiply a vector by a scalar); */
void vector_mult (vector v,  float scalar);

/* pre-multiply point/vector pv by matrix tx */
void transform ( const Matrix tx,  pv  x);

/* Pre Multiply Matrix N,  by Matrix M,   the result is left in N */
void pre_mult ( const Matrix M,  Matrix N);

/* Display Matrix M on stdout */
void showMatrix (const Matrix M);

/* Display point p's co-ordinates on stdout*/
void plot ( point p);

/* Set an element of a matrix */
void set (Matrix M,  int x,  int y,  float value);

/* Create a vector from the difference of two points */
void vector_from_points (point p1,  point p2,  vector v);

/* Return non-zero if the two vectors are equal. */
int vectors_equal (const vector v1,  const vector v2);


#endif /* TXFM_H */
