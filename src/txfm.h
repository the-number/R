/*
    A little library to do matrix arithmetic
    Copyright (C) 1998, 2011  John Darrington

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

#include <stdbool.h>
#include <GL/gl.h>

#define MATRIX_DIM 4

typedef GLfloat Matrix[MATRIX_DIM * MATRIX_DIM];
typedef GLfloat pv[MATRIX_DIM];
typedef pv point, vector;


/* Multiply a vector by a scalar); */
void vector_mult (vector v, float scalar);

/* pre-multiply point/vector pv by matrix tx */
void vector_transform_in_place (pv x, const Matrix tx);

void vector_transform (pv q, const pv x, const Matrix M);

/* Pre Multiply Matrix N,  by Matrix M,   the result is left in N */
void matrix_pre_mult (Matrix N, const Matrix M);

/* Display Matrix M on stdout */
void matrix_dump (const Matrix M);

/* Display point p's co-ordinates on stdout*/
void point_dump (const point p);

/* Set an element of a matrix */
void matrix_set (Matrix M, int x, int y, float value);

/* Create a vector from the difference of two points */
void vector_from_points (vector v, const point p1, const point p2);

/* Return non-zero if the two vectors are equal. */
bool vectors_equal (const vector v1, const vector v2);


#endif /* TXFM_H */
