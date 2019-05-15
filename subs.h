#ifndef _subsH
#define _subsH
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

void write_help (void);

void write_version (void);

int is_xrefstr (char str[]);

int make_arglist (char str[], char *av[]);

void init_ops (int job);

void ops_into_fmt (struct FORMAT *fmt);

int parse_args (int ac, char *av[]);

void alloc_structs (void);

int set_page_format (void);

int tex_str (char *str, char s[], float *wid);

void put_str (char *str);

void set_font (FILE *fp, struct FONTSPEC font, int add_bracket);

void set_font_str (char str[], struct FONTSPEC font);

void check_margin (float new_posx);

void epsf_title (char title[], char fnm[]);

void close_output_file (void);

void open_output_file (char fnam[], char tstr[]);

void open_index_file (char fnam[]);

void close_index_file (void);

void add_to_text_block (char ln[], int add_final_nl);

void write_text_block (FILE *fp, int job);

void put_words (FILE *fp);

void put_text (FILE *fp, int type, char str[]);

void put_history (FILE *fp);

void write_inside_title (FILE *fp);

void write_tunetop(FILE *fp);

void write_tempo(FILE *fp, char tempo[], struct METERSTR meter);

void write_inside_tempo (FILE *fp);

void write_heading (FILE *fp);

void write_parts (FILE *fp);

#endif _subsH
