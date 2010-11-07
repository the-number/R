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
#ifndef QUARTERNION_H
#define QUARTERNION_H

#include "txfm.h"

struct quarternion {
  float w;
  float x;
  float y;
  float z;
};

typedef struct quarternion Quarternion;


void quarternion_from_rotation (Quarternion *q,  vector u,  float theta) ;

void quarternion_print (const Quarternion *q) ;

void quarternion_to_matrix (Matrix M,  const Quarternion *q) ;

/* Pre multiply q1 by q2 */
void quarternion_pre_mult (Quarternion *q1,  const Quarternion *q2) ;

void quarternion_set_to_unit (Quarternion *q) ;


#endif /* QUARTERNION_H */


