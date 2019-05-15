#ifndef _pssubsH
#define _pssubsH
/*
 *  Copyright (C) 1996,1997,1998  Michael Methfessel
 *
 *  This file is part of abc2ps.
 *
 *  abc2ps is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*  subroutines for postscript output  */


void level1_fix (FILE *fp);

void init_ps (FILE *fp, char str[], int is_epsf, float bx1, float by1, float bx2, float by2);

void close_ps (FILE *fp);

void init_page (FILE *fp);

void init_index_page (FILE *fp);

void init_index_file (void);

void close_index_page (FILE *fp);

void close_page (FILE *fp);

void init_epsf (FILE *fp);

void close_epsf (FILE *fp);

void write_pagebreak (FILE *fp);

#endif _pssubsH
