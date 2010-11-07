/*
    GNUbik -- A 3 dimensional magic cube game.
    Copyright (C) 1998, 2003, 2004  John Darrington

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef GNUBIK_H
#define GNUBIK_H



#define ERR_CHECK(string)  error_check (__FILE__,__LINE__,string)

void error_check (const char *file, int line_no, const char *string);

extern int cube_dimension;
extern int number_of_blocks;

#ifndef M_PI
# define M_PI           3.14159265358979323846
#endif

#endif
