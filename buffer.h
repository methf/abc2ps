#ifndef _bufferH
#define _bufferH
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

/* PUTn: add to buffer with n arguments */

#define PUT0(f) {sprintf(mbf,f); a2b(mbf); }
#define PUT1(f,a) {sprintf(mbf,f,a); a2b(mbf); }
#define PUT2(f,a,b) {sprintf(mbf,f,a,b); a2b(mbf); }
#define PUT3(f,a,b,c) {sprintf(mbf,f,a,b,c); a2b(mbf); }
#define PUT4(f,a,b,c,d) {sprintf(mbf,f,a,b,c,d); a2b(mbf); }
#define PUT5(f,a,b,c,d,e) {sprintf(mbf,f,a,b,c,d,e); a2b(mbf); }


void a2b (char *t);

void bskip(float h);

void init_pdims ();

void clear_buffer ();

void write_index_entry ();

void write_buffer (FILE *fp);

void buffer_eob (FILE *fp);

void check_buffer (FILE *fp, int nb);

#endif _bufferH

