#ifndef _musicH
#define _musicH
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
#include "abc2ps.h"

#define XP_START   0
#define XP_END     (maxSyms-1)
#define XP_NIL    -10

#define ABS(a)  ((a)>0) ? (a) : (-a)
#define AT_LEAST(a,b)  if((a)<(b)) a=b;

float nwid (float dur);

float xwid (float dur);

int next_note (int k, int n, struct SYMBOL symb[]);
int prec_note (int k, int n, struct SYMBOL symb[]);

int followed_by_note (int k, int n, struct SYMBOL symb[]);

int preceded_by_note (int k, int n, struct SYMBOL symb[]);

void xch (int *i, int *j);

void print_linetype (int t);

void print_syms(int num1, int num2, struct SYMBOL symb[]);

void print_vsyms (void);

void set_head_directions (struct SYMBOL *s);

void set_minsyms(int ivc);

void set_sym_chars (int n1, int n2, struct SYMBOL symb[]);

void set_beams (int n1, int n2, struct SYMBOL symb[]);

void set_stems (int n1, int n2, struct SYMBOL symb[]);

int set_sym_times (int n1, int n2, struct SYMBOL symb[], struct METERSTR meter0);

void set_sym_widths (int ns1, int ns2, struct SYMBOL symb[], int ivc);

int contract_keysigs (int ip1);

int set_initsyms (int v, float *wid0);

void print_poslist (void);

int insert_poslist (int nins);

void set_poslist (void);

void set_xpwid(void);

void set_spaces (void);

int check_overflow (int ip1, int ip2, float width);

float set_glue (int ip1, int ip2, float width);

void adjust_group(int i1, int i2);

void adjust_spacings (int n);

void adjust_rests (int n, int v);

int copy_vsyms (int v, int ip1, int ip2, float wid0);

void draw_timesig (float x, struct SYMBOL s);

int draw_keysig (float x, struct SYMBOL s);

void draw_bar (float x, struct SYMBOL s);

void draw_barnums (FILE *fp);

void update_endings (float x, struct SYMBOL s);

void set_ending (int i);

void draw_endings (void);

void draw_rest (float x, float yy, struct SYMBOL s, float *gchy);

void draw_gracenotes (float x, float w, float d, struct SYMBOL *s);

void draw_basic_note (float x, float w, float d, struct SYMBOL *s, int m);

float draw_decorations (float x, struct SYMBOL *s, float *tp);

float draw_note (float x, float w, float d, struct SYMBOL *s, int fl, float *gchy);

float vsh (float x, int dir);

float rnd3(float x);

float rnd6(float x);

float b_pos (int stem, int flags, float b);

int calculate_beam (int i0, struct BEAM *bm);
  
float rest_under_beam (float x, int head, struct BEAM *bm);

void draw_beam_num (struct BEAM *bm, int num, float xn);
  
void draw_beam (float x1, float x2, float dy, struct BEAM *bm);

void draw_beams (struct BEAM *bm);

float extreme (float s, float a, float b);

void draw_bracket (int p, int j1, int j2);

void draw_nplet_brackets (void);

float slur_direction (int k1, int k2);

void output_slur (float x1, float y1, float x2, float y2, float s, float height, float shift);

void draw_slur (int k1, int k2, int nn, int level);

int next_scut (int i);
int prev_scut(int i);

void draw_chord_slurs(int k1, int k2, int nh1, int nh2, int nslur, int mhead1[MAXHD], int mhead2[MAXHD], int job);

void draw_slurs (int k1, int k2, int job);

void draw_phrasing (int k1, int k2, int level);

void draw_all_slurs (void);

void draw_all_phrasings (void);

void check_bars1 (int ip1, int ip2);

void check_bars2 (int ip1, int ip2);

void draw_vocals (FILE *fp, int nwl, float botnote, float bspace, float *botpos);

void draw_symbols (FILE *fp, float bspace, float *bpos, int is_top);

void draw_sysbars (FILE *fp, int ip1, int ip2, float wid0, float h1, float dh);

int count_symbols (void);

int select_piece (int ip1);

int is_topvc (int jv);

int vc_select (void);

float voice_label (FILE *fp, char *label, float h, float xc, float dx0, int job);

void mstave_deco (FILE *fp, int ip1, int ip2, float wid0, float hsys, float htab[]);

void output_music (FILE *fp);

void process_textblock(FILE *fpin, FILE *fp, int job);

void process_pscomment (FILE *fpin, FILE *fp, char line[]);

void check_selected(FILE *fp, char xref_str[], int npat, char pat[][STRL1], int sel_all,int search_field);

void process_line (FILE *fp, int type, char xref_str[], int npat, char pat[][STRL1], int sel_all, int search_field);

void process_file (FILE *fpin, FILE *fpout, char xref_str[], int npat, char pat[][STRL1], int sel_all, int search_field);

#endif _musicH
