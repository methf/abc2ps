#ifndef _symsH
#define _symsH
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

#include <stdio.h>
 
/*  subroutines to define postscript macros which draw symbols  */

void def_misc (FILE *fp);

void def_typeset(FILE *fp);

void define_font (FILE *fp, char name[], int num);

void def_tsig (FILE *fp);

void add_cv (FILE *fp, float f1, float f2, float p[][2], int i0, int ncv);

void add_sg (FILE *fp, float f1, float f2, float p[][2], int i0, int nseg);

void add_mv (FILE *fp, float f1, float f2, float p[][2], int i0);

void def_stems (FILE *fp);

void def_dot (FILE *fp);

void def_deco (FILE *fp);

void def_deco1 (FILE *fp);

void def_hl (FILE *fp);

void def_beam (FILE *fp);

void def_flags1 (FILE *fp);

void def_flags2 (FILE *fp);

void def_xflags (FILE *fp);

void def_acc (FILE *fp);

void def_rests (FILE *fp);

void def_bars (FILE *fp);

void def_ends (FILE *fp);
       
void def_gchord (FILE *fp);

void def_sl (FILE *fp);

void def_hd1 (FILE *fp);

void def_hd2 (FILE *fp);

void def_hd3 (FILE *fp);

void def_gnote (FILE *fp);

void def_csg (FILE *fp);

void def_tclef (FILE *fp);

void def_t8clef (FILE *fp);

void def_bclef (FILE *fp);

void def_cclef (FILE *fp);

void def_brace (FILE *fp);

void def_staff (FILE *fp);

void def_sep (FILE *fp);

void define_symbols (FILE *fp);

#endif _symsH
