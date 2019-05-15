#ifndef _formatH
#define _formatH
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
 
void fontspec (struct FONTSPEC *f, char name[], float size, int box);
 
int add_font (struct FONTSPEC *f);

void make_font_list (struct FORMAT *f);

void set_standard_format (struct FORMAT *f);

void set_pretty_format (struct FORMAT *f);

void set_pretty2_format (struct FORMAT *f);

void print_font (char *str, struct FONTSPEC fs);

void print_format (struct FORMAT f);

void g_unum (char *l, char *s, float *num);

void g_logv (char *l, char *s, int *v);

void g_fltv (char *l, int nch, float *v);

void g_intv (char *l, int nch, int *v);

void g_fspc (char *l, int nch, struct FONTSPEC *fn);

int interpret_format_line (char l[], struct FORMAT *f);

int read_fmt_file (char filename[], char dirname[], struct FORMAT *f);

#endif _formatH
