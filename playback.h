#ifndef _playbackH
#define _playbackH
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

#define SAMPLES_PER_SEC   8000
#define MAX_SAMPLES       48000
#define AU_IS_REST        9999
#define AU_IS_NONE        8888
#define AU_MAXV           50


#define AU_SINE       1
#define AU_TRIANGLE   2
#define AU_SAWTOOTH   3



struct TRIPLE {
  float low,mid,hi;
};

struct AU_QUALITY {
  struct TRIPLE att;        /* time when attack envelope reaches 1/2 */ 
  struct TRIPLE dec1;       /* time when 1st decay enelope reaches 1/e */
  struct TRIPLE amp1;       /* amplitude */
  struct TRIPLE dec2;       /* 2nd decay */
  struct TRIPLE amp2;       /* 2nd amp   */
  struct TRIPLE dec3;       /* fast decay after offical end of note */
  struct TRIPLE freq;       /* low, mid, hi frequencies */
  struct TRIPLE sm;         /* smoothing for triangle or sawtooth */
  int type;                 /* type of waveform */
  int num;                  /* number used to select this set */
  char name[21];            /* string to name this set */

};


struct SBUF {
  int sps;                             /* samples per second */
  double t1;                           /* start time */
  double t2;                           /* end time   */
  double tau;                          /* time between samples */
  double g0;                           /* current AGC factor */
  double top;                          /* for averaging */
  int new;                             /* flag if nothing written */
  int n;                               /* number of samples */
  float data[MAX_SAMPLES];             /* data */
};

struct AU_FLOWCTRL {
  int repeat;
  int lrep;
  int rrep;
  int nrep;
};


void open_audio_file (char aunam[]);

void close_audio_file ();

void au_init_sbuf (struct SBUF *sb);

void au_agc_sbuf (int nb, struct SBUF *sb);

void au_write_sbuf (float t, struct SBUF *sb);

struct AU_QUALITY au_set_quality (int select);

float au_getpar (struct TRIPLE p, struct TRIPLE f, float freq);

/*| void au_add_sample (float t0, float tn, float tx, float freq,  |*/
/*|                    struct AU_QUALITY q, struct SBUF *sb); |*/

/*| void au_write_note (float t0, float tn, float tx, int hs,  |*/
/*|                     struct AU_QUALITY q, struct SBUF *sb); |*/

void au_clear_atab (int atab[]);

void au_copy_atab (int atab[], int atab1[]);

void au_set_atab (int pit, int acc, int atab[]);

void au_set_atabs (int v, int k, int atab[]);

int hs_shift (int pit, int acc, int atab[], struct KEYSTR key);

/*| int find_ties (int v, int k, int n, int atab[], struct KEYSTR key, |*/
/*|                struct AU_FLOWCTRL fc); |*/


/*| void au_play_note (int v, int k, float t0, float tn, float tadd, float tgr,  |*/
/*|                    int atab[], struct KEYSTR key, struct AU_QUALITY q, |*/
/*|                    struct SBUF *sb); |*/

/*| void au_play_notes (float tempo, struct AU_QUALITY q); |*/

void output_audio ();

#endif _playbackH

