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

#include <config.h>
#include <stdio.h>
#include <string.h>
#include "txfm.h"
#include <assert.h>

/* print out the location of a point.  For debug only */
void
point_dump (const point p)
{
  int i;

  for (i = 0; i < MATRIX_DIM; i++)
    {
      printf ("%f\t", p[i]);
    }
  putchar ('\n');
}



/* Pre-multiply a point or vector x,  by matrix M */
void
vector_transform_in_place (pv x, const Matrix M)
{
  int i, j;

  /* Temporary point variable for result */
  pv q = { 0, 0, 0, 0 };

  /* for each column */
  for (j = 0; j < MATRIX_DIM; ++j)
    {
      /* for each row */
      for (i = 0; i < MATRIX_DIM; ++i)
	q[i] += M[i + MATRIX_DIM * j] * x[j];
    }

  /* Now copy  q into x */
  for (i = 0; i < MATRIX_DIM; i++)
    x[i] = q[i];
}

/* Pre-multiply a point or vector x,  by matrix M, placing the result into q */
void
vector_transform (pv q, const pv x, const Matrix M)
{
  int i, j;

  memset (q, 0, sizeof (*q) * MATRIX_DIM);

  /* for each column */
  for (j = 0; j < MATRIX_DIM; ++j)
    {
      /* for each row */
      for (i = 0; i < MATRIX_DIM; ++i)
	q[i] += M[i + MATRIX_DIM * j] * x[j];
    }
}



/* print a matrix on the stdout.  For debugging */
void
matrix_dump (const Matrix M)
{
  int i, j;

  /* for each row */
  for (i = 0; i < MATRIX_DIM; i++)
    {
      /* for each column */
      for (j = 0; j < MATRIX_DIM; j++)
	{
	  printf ("%f\t", (float) M[i + (MATRIX_DIM * j)]);
	}
      putchar ('\n');
    }
}




/* Pre-multiply a matrix N,  by matrix M */
void
matrix_pre_mult (Matrix N, const Matrix M)
{
  int i, j;
  int k;

  /* Temporary Matrix variable for result */
  Matrix T = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
  };

  for (k = 0; k < MATRIX_DIM; k++)
    {
      for (i = 0; i < MATRIX_DIM; i++)
	{
	  for (j = 0; j < MATRIX_DIM; j++)
	    {
	      T[i + k * MATRIX_DIM] +=
		M[i + MATRIX_DIM * j] * N[j + MATRIX_DIM * k];
	    }
	}
    }

  /* Now copy  T into N */
  for (i = 0; i < MATRIX_DIM * MATRIX_DIM; i++)
    N[i] = T[i];
}



/* Set matrix element x,  y to value */
void
matrix_set (Matrix M, int x, int y, float value)
{
  assert (x <= MATRIX_DIM);
  assert (y <= MATRIX_DIM);

  M[x + MATRIX_DIM * y] = value;

}

/* Create a vector by taking the difference of 2 points */
void
vector_from_points (vector v, const point p1, const point p2)
{
  int i;

  for (i = 0; i < MATRIX_DIM; ++i)
    v[i] = p2[i] - p1[i];

  assert (v[MATRIX_DIM - 1] == 0.0);
}


/* Return zero if any corresponding components of the two vectors are not equal,
   otherwise return one to indicate the vectors are identical. */
bool
vectors_equal (const vector v1, const vector v2)
{
  int i;

  for (i = 0; i < 4; ++i)
    if (v1[i] != v2[i])
      return 0;

  return 1;
}
