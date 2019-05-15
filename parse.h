#ifndef _parseH
#define _parseH
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

#include "abc2ps.h"

void syntax (char msg[], char *q);

int isnote (char c);

void zero_sym (void);

int add_sym (int type);

int insert_sym (int type, int k);

int get_xref (char str[]);

void set_meter (char str[], struct METERSTR *meter);

void set_dlen (char str[], struct METERSTR *meter);

int set_keysig(char s[], struct KEYSTR *ks, int init);

int get_halftones (struct KEYSTR key, char transpose[]);

void shift_key (int sf_old, int nht, int *sfnew, int *addt);

void set_transtab (int nht, struct KEYSTR *key);

void do_transpose (struct KEYSTR key, int *pitch, int *acc);

void gch_transpose (struct KEYSTR key);

void init_parse_params (void);

void add_text (char str[], int type);

void reset_info (struct ISTRUCT *inf);

void get_default_info (void);

int is_info_field (char str[]);

int is_end_line (char str[]);

int is_pseudocomment (char str[]);

int is_comment (char str[]);

void trim_title (char s[], char s0[]);

int find_voice (char vid[], int *new);

int switch_voice (char str[]);

int info_field (char str[]);

void append_meter (struct METERSTR meter);

void append_key_change(struct KEYSTR oldkey, struct KEYSTR newkey);

int numeric_pitch(char note);

int symbolic_pitch(int pit, char str[]);

void handle_inside_field(int type);

int parse_uint (void);
  
int parse_bar (void);
  
int parse_space (void);

int parse_esc (void);

int parse_nl (void);

int parse_gchord (void);

int parse_deco (int dtype[10]);

int parse_length (void);

int parse_grace_sequence (int pgr[], int agr[]);

void identify_note (struct SYMBOL *s, char *q);

void double_note (int i, int num, int sign, char *q);

int parse_basic_note (int *pitch, int *length, int *accidental);

int parse_note (void);

int parse_sym (void);

char *add_wd(char str[]);

int parse_vocals (char line[]);

int parse_music_line (char line[]);

int is_selected ( char xref_str[], int npat, char pat[][STRL1], int select_all, int search_field);

int rehash_selectors (char sel_str[], char xref_str[], char pat[][STRL1]);

void decomment_line (char ln[]);

int get_line (FILE *fp, char ln[]);

int read_line (FILE *fp, int do_music, char line[BSIZE]);

void do_index(FILE *fp, char xref_str[], int npat, char pat[][STRL1], int select_all, int search_field);

#endif _parseH
