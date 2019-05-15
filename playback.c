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
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "abc2ps.h"
#include "util.h"
#include "music.h"
#include "playback.h"

#ifdef SUN
#include <multimedia/libaudio.h>
#else
#include "sox.h"
#endif

int aufd;
struct SBUF sb;
float tsave;

#define DETUNE 0.1

#ifdef SUN

/* --- open_audio_file, Sun-specific. Adapted from playabc by Don Ward. --- */
void open_audio_file (char aunam[])
{
  int rc;
  Audio_hdr hdr;
  char auinfo[] = "abc2ps playback";

  if (vb>9) printf ("Open audio output file %s\n", aunam);
  aufd = open (aunam, O_WRONLY|O_CREAT|O_TRUNC,
               S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if (aufd < 0) rx ("Cannot open audio output file: ", aunam);

  hdr.sample_rate = SAMPLES_PER_SEC;
  hdr.samples_per_unit = 1;
  hdr.bytes_per_unit = 1;
  hdr.channels = 1;
  hdr.encoding = AUDIO_ENCODING_ULAW;
  hdr.data_size = AUDIO_UNKNOWN_SIZE;

  rc = audio_write_filehdr(aufd, &hdr, auinfo, strlen(auinfo));
  if (rc != 0) rx("Error writing audio file header","");

  au_init_sbuf (&sb);

}

#else
/* --- open_audio_file, not Sun-specific --- */
void open_audio_file (char aunam[])
{
  int rc;

  if (vb>9) printf ("Open audio output file %s\n", aunam);
  aufd = open (aunam, O_WRONLY|O_CREAT|O_TRUNC,
               S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if (aufd < 0) rx ("Cannot open audio output file: ", aunam);

  rc = write_header(aufd);
  if (rc != 0) rx("Error writing audio file header","");

  au_init_sbuf (&sb);

}

#endif


/* --- close_audio_file --- */
void close_audio_file ()
{
  int m,min;
  float sec;

  tsave += 0.5;
  au_write_sbuf (tsave, &sb);

  close (aufd);

  min = tsave/60;
  sec = tsave - 60*min;

  m=get_file_size (AUDIOFILE);
  printf ("Audio written on %s (%d:%02.0f minutes, %d title%s, %d byte%s)\n",
          AUDIOFILE,
          min,sec,
          tunenum, tunenum==1 ? "" : "s",
          m, m==1 ? "" : "s");

}


/* --- au_init_sbuf --- */
void au_init_sbuf (struct SBUF *sb)
{
  int i;

  sb->n      =  MAX_SAMPLES;
  sb->sps    =  SAMPLES_PER_SEC;
  sb->tau    =  1.0/SAMPLES_PER_SEC;
  sb->t1     =  -0.5;     /* leave some time for initial graces */
  sb->t2     =  sb->t1 + sb->n * sb->tau;
  sb->g0     =  1.0;
  sb->top    =  1.0;
  sb->new    =  1;
  for (i=0; i<MAX_SAMPLES; i++) sb->data[i]=0.0;

  if (vb>9) {
    printf ("\nInit audio buffer: %d samples, %d per second, tau=%.6f\n",
            sb->n, sb->sps, sb->tau);
    printf ("Time interval is (%.3f,%.3f)\n", sb->t1, sb->t2);
  }

}


/* --- au_agc_sbuf: scale to get consistent amplitudes --- */
void au_agc_sbuf (int nb, struct SBUF *sb)
{
  int i,m,i0,num,imax,kx;
  float g0,g1,g,s,sx,d2,tmax,amp0,gmax,top;

  num=1600;

  num=4000;

  amp0=0.70;
  g0 = sb->g0;
  top = sb->top;
  i0 = 0;
  for (;;) {

    tmax=0;
    for (i=i0; i<i0+num; i++) {
      if (i==nb) break;
      d2 = sb->data[i] * sb->data[i];
      if (d2>tmax) tmax=d2;
    }
    m = i;
    tmax=sqrt(tmax);
    top = 0.2*tmax+0.8*top;
    if (sb->new) top=tmax;
    if (top<0.25) top=0.25;
    g1  = amp0/top;
    if (sb->new) g0=g1;
    if (sb->new) sb->new=0;
    if (g1*tmax>0.90) g1=0.90/tmax;
    if (g0*tmax>0.90) g0=0.90/tmax;
/*|   printf ("%5d to %5d   max=%.2f   top=%.2f   gain %.2f to %.2f\n",   |*/
/*|            i0, m, tmax, top, g0, g1);    |*/
    s = (g1-g0)/(m-i0);
    for (i=i0; i<m; i++) {
      g=g0+s*(i-i0);
      sb->data[i] *= g;
    }

    g0=g1;
    i0 = m;
    if (i0>=nb) break;
  }

  sb->g0 = g0;
  sb->top = top;
}


/* --- au_write_sbuf: unload part of buffer below time t --- */
void au_write_sbuf (float t, struct SBUF *sb)
{
  int i0,i,rc,nb;
  unsigned char sample[MAX_SAMPLES];

  nb = sb->n;
  i0 = (t-sb->t1)/sb->tau;

  if (vb>14)
    printf ("    Write sbuf up to t=%.3f (point %d) in interval=(%.3f,%.3f)\n",
            t, i0, sb->t1, sb->t2);

  if (i0<=0) return;
  if (i0>nb) i0=nb;

  au_agc_sbuf (i0,sb);

  for (i=0; i<i0; i++) sample[i] = audio_s2u(audio_d2s(sb->data[i]));

  rc = write (aufd, sample, i0);

/*|   for (i=0;i<i0;i++) printf ("%5.3f\n", sb->data[i]); |*/


  if (rc != i0) printf ("+++ Error writing audio data, nret=%d\n", rc);

  for (i=0; i<nb-i0; i++) sb->data[i] = sb->data[i+i0];
  for (i=nb-i0; i<nb; i++) sb->data[i] = 0;

  sb->t1 = sb->t1 + i0*sb->tau;
  sb->t2 = sb->t2 + i0*sb->tau;

  if (vb>14) printf ("    New interval is (%.3f,%.3f)\n", sb->t1, sb->t2);

}


struct TRIPLE au_sp (float a, float b, float c)
{
  struct TRIPLE t;
  t.low=0;      if (a<1000) t.low=a;
  t.mid=t.low;  if (b<1000) t.mid=b;
  t.hi= t.mid;  if (c<1000) t.hi=c;
  return t;

}


/* --- au_set_quality: set parameters for tone quality ---*/
struct AU_QUALITY au_set_quality (int select)
{
  struct AU_QUALITY q;
  float x = 1001;

  q.type = 0;
  q.num  = select;

  /* C=130.8    C=261.6   c=523.2  c'=1046.4  */
  q.freq.low = 130.8;
  q.freq.mid = 261.6;
  q.freq.hi  = 523.2;

  /* pianic */
  if (select==1) {
    strcpy (q.name, "P1");
    q.type = AU_SINE;
/*|         q.type = AU_TRIANGLE;      |*/
/*|         q.sm  =  au_sp ( 0.45 , 0.50, 0.55);      |*/
    q.att  = au_sp ( 0.005, 0.003, 0.001);
    q.dec1 = au_sp ( 0.80, 0.60, 0.40 );
    q.dec2 = au_sp ( 0.04, 0.02, 0.01 );
    q.amp1 = au_sp ( 0.30, 0.3, 0.30 );
    q.amp2 = au_sp ( 0.50, 0.30,0.30 );
    q.dec3 = au_sp ( 0.10, 0.08, 0.04 );
  }

  /* another triangle. Accordion? */
  if (select==2) {
    strcpy (q.name, "Acc");
    q.type = AU_TRIANGLE;
    q.sm   = au_sp ( 0.10,  0.15, 0.20 );
    q.att  = au_sp ( 0.01,  x, x );
    q.dec1 = au_sp ( 1.00,  x, x );
    q.dec2 = au_sp ( 0.30,  x, x );
    q.amp1 = au_sp ( 0.50,  x, x );
    q.amp2 = au_sp ( 0.10,  x, x );
    q.dec3 = au_sp ( 0.02,  x, x );
  }

  /* fluty */
  if (select==3) {
    strcpy (q.name, "Fl");
    q.type = AU_TRIANGLE;
    q.sm   = au_sp ( 0.10,  0.20,   0.25 );
    q.att  = au_sp ( 0.02,  0.02,  0.02 );
    q.dec1 = au_sp ( 2.20,  2.20,   x );
    q.dec2 = au_sp ( 0.30,  0.30,   0.30 );
    q.amp1 = au_sp ( 0.50,  0.50,   x );
    q.amp2 = au_sp ( 0.005,  0.005, x );
    q.dec3 = au_sp ( 0.02,  0.02,   x );
  }

  /* sawtooth, faster decay. cemblic */
  if (select==4) {
    strcpy (q.name, "Cem");
    q.type = AU_SAWTOOTH;
    q.sm   = au_sp ( 0.03,   0.035, 0.08 );
    q.att  = au_sp ( 0.002, 0.002, x );
    q.dec1 = au_sp ( 0.60,   0.50, x );
    q.dec2 = au_sp ( 0.02,   0.01, x );
    q.amp1 = au_sp ( 0.30,   0.30, x );
    q.amp2 = au_sp ( 0.40,   0.40, x );
    q.dec3 = au_sp ( 0.10,   x, x );
  }


  /* organic, triangle */
  if (select==5) {
    strcpy (q.name, "Org");
    q.type = AU_TRIANGLE;
    q.sm   = au_sp ( 0.25,  x, x );
    q.sm   = au_sp ( 0.15,  0.25, 0.25 );
    q.att  = au_sp ( 0.005, x, x );
    q.dec1 = au_sp ( 3.00,  x, x );
    q.dec2 = au_sp ( 0.02,  x, x );
    q.amp1 = au_sp ( 0.50,  x, x );
    q.amp2 = au_sp ( 0.20,  x, x );
    q.dec3 = au_sp ( 0.10,  x, x );
  }


  /* something plucked. git, bass.. */
  if (select==6) {
    strcpy (q.name, "Git");
    q.type = AU_TRIANGLE;
    q.sm   = au_sp ( 0.20,  0.20, 0.20 );
    q.att  = au_sp ( 0.00002,  x, x);
    q.dec1 = au_sp ( 0.20,   0.15, 0.10 );
    q.dec2 = au_sp ( 0.80,   0.40, 0.30 );
    q.amp1 = au_sp ( 0.60,   x, x );
    q.amp2 = au_sp ( 0.30,   x, x );
    q.dec3 = au_sp ( 0.10,   x, x );
  }


  if (!q.type) rxi ("Undefined audio quality: ", select);

  return q;

}


/* --- au_getpar: interpolate through par values --- */
float au_getpar (struct TRIPLE p, struct TRIPLE f, float freq)
{

  if (freq < f.low) return p.low;
  if (freq > f.hi)  return p.hi;

  if (freq < f.mid)
    return p.low + (freq-f.low)*(p.mid-p.low)/(f.mid-f.low);
  else
    return p.mid + (freq-f.mid)*(p.hi-p.mid)/(f.hi-f.mid);

}


/* --- au_add_sample: add in sine-wave samples for one note --- */
void au_add_sample (float t0, float tn, float tx, float freq,
                   struct AU_QUALITY q, float amp, struct SBUF *sb)
{
  int i,n0,nn,nx,nb;
  float att,dec1,amp1,dec2,amp2,dec3;
  float tau,t1,t2;
  float t,fc,fs,fa,fd,md,mc,ms,f0,ph0;
  float md2,fd2,fa2,env,mdx;
  float tpi = 6.283185;

  att  = au_getpar(q.att,  q.freq, freq);
  dec1 = au_getpar(q.dec1, q.freq, freq);
  amp1 = au_getpar(q.amp1, q.freq, freq);
  dec2 = au_getpar(q.dec2, q.freq, freq);
  amp2 = au_getpar(q.amp2, q.freq, freq);
  dec3 = au_getpar(q.dec3, q.freq, freq);

  tau = sb->tau;
  t1  = sb->t1;
  t2  = sb->t2;
  nb  = sb->n;

  n0 = (t0-t1)/tau;
  nn = (tn-t1)/tau;
  nx = (tx-t1)/tau;
  if (vb>19)
    printf ("    Add sample:  %.1f Hz   tm %.3f %.3f %.3f   pt %d %d %d\n",
            freq, t0,tn,tx, n0,nn,nx);

  if ((n0<0) || (nn>nb) || (nx>nb))
    printf ("+++sample outside buffer: t=%.3f %.3f %.3f  i=%d %d %d\n",
            t0,tn,tx, n0,nn,nx);
  if (n0<0) n0=0; if (nn>nb) nn=nb; if (nx>nb) nx=nb;

  /* detune a bit.. helps for multiple voices */
  freq = freq+ranf(-DETUNE,DETUNE);
  md = exp(-tau/dec1);
  mc = cos(tpi*freq*tau);
  ms = sin(tpi*freq*tau);
  md2 = exp(-tau/dec2);
  t  = 0;

  fs = f0 = 0.0;
  fc = 1.0;
  fd  = amp1*amp;
  fd2 = amp2*amp;
  for (i=n0; i<nn; i++) {
    fa = t/(t+att);
    env = fa*(fd+fd2);
    sb->data[i] += f0*env;
    t  = t+tau;
    fs = f0*mc + fc*ms;
    fc = fc*mc - f0*ms;
    f0 = fs;
    fd = fd*md;
    fd2= fd2*md2;
  }

  mdx = exp(-tau/dec3);
  for (i=nn; i<nx; i++) {
    sb->data[i] += f0*env;
    env = env*mdx;
    fs = f0*mc + fc*ms;
    fc = fc*mc - f0*ms;
    f0 = fs;
  }


}

/* --- au_add_ssample: sawtooth-wave samples --- */
void au_add_ssample (float t0, float tn, float tx, float freq,
                   struct AU_QUALITY q, float amp, struct SBUF *sb)
{
  int i,n0,nn,nx,nb;
  float att,dec1,amp1,dec2,amp2,dec3,sm;
  float tau,t1,t2,tt,bet,a;
  float t,fl,fa,fd,md,mc,ms,f0,per,inc;
  float md2,fd2,fa2,env,mdx;

  att  = au_getpar(q.att,  q.freq, freq);
  dec1 = au_getpar(q.dec1, q.freq, freq);
  amp1 = au_getpar(q.amp1, q.freq, freq);
  dec2 = au_getpar(q.dec2, q.freq, freq);
  amp2 = au_getpar(q.amp2, q.freq, freq);
  dec3 = au_getpar(q.dec3, q.freq, freq);
  sm   = au_getpar(q.sm,   q.freq, freq);

  tau = sb->tau;
  t1  = sb->t1;
  t2  = sb->t2;
  nb  = sb->n;

  n0 = (t0-t1)/tau;
  nn = (tn-t1)/tau;
  nx = (tx-t1)/tau;
  if (vb>19)
    printf ("    Add tsample:  %.1f Hz   tm %.3f %.3f %.3f   pt %d %d %d\n",
            freq, t0,tn,tx, n0,nn,nx);

  if ((n0<0) || (nn>nb) || (nx>nb))
    printf ("+++sample outside buffer: t=%.3f %.3f %.3f  i=%d %d %d\n",
            t0,tn,tx, n0,nn,nx);
  if (n0<0) n0=0; if (nn>nb) nn=nb; if (nx>nb) nx=nb;

  freq = freq+ranf(-DETUNE,DETUNE);
  md = exp(-tau/dec1);
  md2 = exp(-tau/dec2);

  per = 1/freq;
  a = 1.0/sm;
  bet = 2*erf(0.5*a)/per;

  tt = 0;
  t  = 0;
  fd  = amp1*amp;
  fd2 = amp2*amp;
  for (i=n0; i<nn; i++) {
    fa = t/(t+att);
    env = fa*(fd+fd2);
    f0 = erf(a*tt/per)-bet*tt;
    sb->data[i] += f0*env;
    tt += tau;
    if (tt>0.5*per) tt -= per;
    t  = t+tau;
    fd = fd*md;
    fd2= fd2*md2;
  }

  mdx = exp(-tau/dec3);
  for (i=nn; i<nx; i++) {
    f0 = erf(a*tt/per)-bet*tt;
    sb->data[i] += f0*env;
    tt += tau;
    if (tt>0.5*per) tt -= per;
    env = env*mdx;
  }


}


/* --- au_add_tsample: triangle-wave samples --- */
void au_add_tsample (float t0, float tn, float tx, float freq,
                   struct AU_QUALITY q, float amp, struct SBUF *sb)
{
  int i,n0,nn,nx,nb;
  float att,dec1,amp1,dec2,amp2,dec3,sm;
  float tau,t1,t2,tt,bet,a,x;
  float t,fl,fa,fd,md,mc,ms,f0,per,inc;
  float md2,fd2,fa2,env,mdx;

  att  = au_getpar(q.att,  q.freq, freq);
  dec1 = au_getpar(q.dec1, q.freq, freq);
  amp1 = au_getpar(q.amp1, q.freq, freq);
  dec2 = au_getpar(q.dec2, q.freq, freq);
  amp2 = au_getpar(q.amp2, q.freq, freq);
  dec3 = au_getpar(q.dec3, q.freq, freq);
  sm   = au_getpar(q.sm,   q.freq, freq);

  tau = sb->tau;
  t1  = sb->t1;
  t2  = sb->t2;
  nb  = sb->n;

  n0 = (t0-t1)/tau;
  nn = (tn-t1)/tau;
  nx = (tx-t1)/tau;
  if (vb>19)
    printf ("    Add tsample:  %.1f Hz   tm %.3f %.3f %.3f   pt %d %d %d\n",
            freq, t0,tn,tx, n0,nn,nx);

  if ((n0<0) || (nn>nb) || (nx>nb))
    printf ("+++sample outside buffer: t=%.3f %.3f %.3f  i=%d %d %d\n",
            t0,tn,tx, n0,nn,nx);
  if (n0<0) n0=0; if (nn>nb) nn=nb; if (nx>nb) nx=nb;

  freq = freq+ranf(-DETUNE,DETUNE);
  md = exp(-tau/dec1);
  md2 = exp(-tau/dec2);

  per = 1/freq;
  a = 1.0/sm;
  bet = erf(0.5*a);

  tt = 0;
  t  = 0;
  fd  = amp1*amp;
  fd2 = amp2*amp;
  for (i=n0; i<nn; i++) {
    fa = t/(t+att);
    env = fa*(fd+fd2);
    x = tt/per;
    if (tt<0) f0 = 4*(x+0.25)*erf(2*a*(x+0.25))-bet;
    else      f0 = bet-4*(x-0.25)*erf(2*a*(x-0.25));
    sb->data[i] += f0*env;
    tt += tau;
    if (tt>0.5*per) tt -= per;
    t  = t+tau;
    fd = fd*md;
    fd2= fd2*md2;
  }

  mdx = exp(-tau/dec3);
  for (i=nn; i<nx; i++) {
    x = tt/per;
    if (tt<0) f0 = 4*(x+0.25)*erf(2*a*(x+0.25))-bet;
    else      f0 = bet-4*(x-0.25)*erf(2*a*(x-0.25));
    sb->data[i] += f0*env;
    tt += tau;
    if (tt>0.5*per) tt -= per;
    env = env*mdx;
  }


}


/* --- au_write_note --- */
void au_write_note (float t0, float tn, float tx, int hs,
                    struct AU_QUALITY q, float amp, struct SBUF *sb)
{
  float freq, middlec=261.6;

  if (hs==AU_IS_REST) return;

  freq = middlec * pow(2.0,hs/12.0);

/*|   printf ("    write note t0=%.3f tn=%.3f dur=%.3f  %.1f Hz\n",  |*/
/*|           t0,tn,tn-t0,freq); |*/

  if (q.type == AU_SINE)
    au_add_sample (t0,tn,tx,freq,q,amp,sb);
  if (q.type == AU_SAWTOOTH)
    au_add_ssample (t0,tn,tx,freq,q,amp,sb);
  if (q.type == AU_TRIANGLE)
    au_add_tsample (t0,tn,tx,freq,q,amp,sb);

}


/* --- au_clear_atab --- */
void au_clear_atab (int atab[])
{
  int i;
  for (i=0;i<7;i++) atab[i]=0;
}

/* --- au_copy_atab --- */
void au_copy_atab (int atab[], int atab1[])
{
  int i;
  for (i=0;i<7;i++) atab1[i]=atab[i];
}



/* --- au_set_atab: update atab for one pit,acc pair --- */
void au_set_atab (int pit, int acc, int atab[])
{
  int p;

  p=pit;
  while (p>22) p-=7;
  while (p<16) p+=7;
  if (acc!=0) atab[p-16]=acc;

}


/* --- au_set_atabs: update atab for one note --- */
void au_set_atabs (int v, int k, int atab[])
{
  int pit,acc,j;

  for (j=0; j<symv[v][k].gr.n; j++) {
    pit=symv[v][k].gr.p[j];
    acc=symv[v][k].gr.a[j];
    au_set_atab (pit,acc,atab);
  }

  for (j=0; j<symv[v][k].npitch; j++) {
    pit=symv[v][k].pits[j];
    acc=symv[v][k].accs[j];
    au_set_atab (pit,acc,atab);
  }

}


/* --- hs_shift: return number of halfsteps between pitch and C --- */
int hs_shift (int pit, int acc, int atab[], struct KEYSTR key)
{
  int p,k,oct,sf;
  int ktab[7] = {0,2,4,5,7,9,11};    /* whole and half steps */
  int stab[7] = {2,4,6,1,3,5,7};     /* for sharps in keysig */
  int ftab[7] = {6,4,2,7,5,3,1};     /* for flats in keysig */

  p=pit;
  while (p>22) p-=7;
  while (p<16) p+=7;

  k=ktab[p-16];
  sf = key.sf;

  if (acc==0) acc=atab[p-16];
  if (acc) {
    if (acc==A_SH) k=k+1;
    if (acc==A_DS) k=k+2;
    if (acc==A_FT) k=k-1;
    if (acc==A_DF) k=k-2;
  }
  else {
    if ((sf>0) && (sf>=stab[p-16])) k++;
    if ((sf<0) && (-sf>=ftab[p-16])) k--;
  }

  oct=(pit-p)/7;
  if (key.ktype==BASS) oct=oct-2;
  if (key.ktype==ALTO) oct=oct-1;

  k=k+12*oct;
  atab[p-16]=acc;

  return k;

}

/* --- au_play_note: write output for one note ---*/
void au_play_note (int v, int k, float t0, float tn, float tadd, float tgr,
                   int atab[], struct KEYSTR key, struct AU_QUALITY q,
                   float amp, struct SBUF *sb)
{
  int ng,j,pit,acc,hs;
  float tg,dg;

  ng = symv[v][k].gr.n;
  dg = tgr/ng;
  tg = t0-tgr;

  for (j=0; j<ng; j++) {
    pit = symv[v][k].gr.p[j];
    acc = symv[v][k].gr.a[j];
    hs = hs_shift (pit,acc,atab,key);
    au_write_note (tg,tg+dg,tg+dg+tadd,hs,q,amp,sb);
    tg += dg;
  }

  for (j=0; j<symv[v][k].npitch; j++) {
    pit = symv[v][k].pits[j];
    acc = symv[v][k].accs[j];
    hs = hs_shift (pit,acc,atab,key);
    au_write_note (t0,tn,tn+tadd,hs,q,amp,sb);
  }

}


/* --- au_flow_control: call at bar and end, returns index for xpos --- */
int au_flow_control (int i, int v, int k, struct AU_FLOWCTRL *fc)
{
  int ii;

  ii=i;

  /* Take care of missing :| at tune end */
  if (i==XP_END) {
    if ((!fc->repeat) && (fc->lrep!=XP_NIL) && (fc->lrep!=XP_START)) {
      fc->repeat=1;
      ii=xp[fc->lrep].next;
      fc->lrep=XP_NIL;
      fc->nrep++;
    }
    return ii;
  }

  if (symv[v][k].type!=BAR) return i;

  if (!fc->repeat) {
    if (symv[v][k].u==B_LREP) fc->lrep=i;
    if ((symv[v][k].u==B_RREP) || (symv[v][k].u==B_DREP)) {
      if ((fc->lrep==XP_NIL) && (fc->rrep!=XP_NIL)) fc->lrep=fc->rrep;
      fc->rrep=i;
      if (fc->lrep!=XP_NIL) {
        ii=fc->lrep;
        fc->lrep=XP_NIL;
        fc->nrep++;
        fc->repeat=1;
      }
    }
  }

  else {
    if ((symv[v][k].u==B_RREP) || (symv[v][k].u==B_DREP)) {
      fc->repeat=0;
      if (symv[v][k].u==B_DREP) fc->lrep=i;
    }
    if (symv[v][k].v==1) {
      if (fc->rrep!=XP_NIL) {
        ii=fc->rrep;
        fc->rrep=XP_NIL,
        fc->repeat=0;
      }
    }
  }

/*|   if (ii!=i) printf ("FC: jump %d to %d  v=%d k=%d\n", i,ii,v,k); |*/
  return ii;

}



/* --- find_ties: return index of last note tied on --- */
/* Algorithm: a note is tied to the one before if
   (1) the tones are exactly the same, or
   (2) the posit on the staff is the same and there is no accidental.
   This should take care of ties over barlines, hopefully.  */

int find_ties (int i, int v, int k, int n, int atab[], struct KEYSTR key,
               struct AU_FLOWCTRL fc)
{
  int j,last,t,pit1,acc1,hs1,pit2,acc2,hs2,pitl,atab2[7],tt,i2;
  struct KEYSTR key2;
  struct AU_FLOWCTRL fc2;

  if (symv[v][k].slur_st==0) return k;
  au_copy_atab (atab, atab2);
  key2=key;
  fc2=fc;

  pit1 = symv[v][k].pits[0];
  acc1 = symv[v][k].accs[0];
  hs1 = hs_shift(pit1,acc1,atab,key);
  last=k;
  pitl=pit1;

  i2=xp[i].next;
  for (;;) {
    j=xp[i2].p[v];
    t=symv[v][j].type;

    if (t==REST)   return last;

    if (t==BAR) {
      au_clear_atab (atab2);
      i2=au_flow_control (i2,v,j,&fc2);
    }

    if (t==CLEF)   key2.ktype=symv[v][j].u;
    if (t==KEYSIG) key2.sf=symv[v][j].q;

    if (t==NOTE) {
      pit2 = symv[v][j].pits[0];
      acc2 = symv[v][j].accs[0];
      hs2 = hs_shift(pit2,acc2,atab2,key2);
      if (symv[v][j].slur_end==0) return last;

      if (hs2 != hs1) {
        if (pit2 != pitl) return last;
        if (acc2 != 0)    return last;
      }

      if (symv[v][j].slur_st==0) return j;
      pitl=pit1;
      last=j;
    }

    i2=xp[i2].next;
    if (i2==XP_END) break;
  }
  return last;

}




/* --- au_play_notes ---*/
void au_play_notes (float tempo, struct AU_QUALITY q[])
{
  int v,i,n,k,t;
  float tadd,tnow,tdone,tprt,dgr0,tgr1[AU_MAXV],tgr2,tend;
  float tbar,levx,tdec,amp;
  int in_tie[AU_MAXV],kk[AU_MAXV],kpt[AU_MAXV],k1,t1;
  int atab[AU_MAXV][7], atab1[AU_MAXV][7];
  struct KEYSTR key[AU_MAXV],key1[AU_MAXV];
  float tn[AU_MAXV],td[AU_MAXV];
  struct AU_FLOWCTRL fc;

  /* change amplitude within one bar */
  levx = 0.8;
  tdec = 1.0;

  /* default length for grace notes */
  dgr0 = (QUARTER/5) *60.0/(tempo*QUARTER);

  /* time for decay after note stops */
  tadd=0.3;

  if (nvoice > AU_MAXV) rx("play_notes: increase AU_MAXV","");

  fc.rrep = XP_NIL;
  fc.lrep = XP_START;
  fc.repeat = fc.nrep = 0;
  tnow = tsave;
  tprt = tnow+20;
  tbar = tnow;

  for (v=0; v<nvoice; v++) {
    in_tie[v] = 0;
    tn[v] = td[v] = tnow;
    au_clear_atab (atab[v]);
    key[v]=voice[v].key0;
    kpt[v] = -1;
    tgr1[v] = -1;
  }

  i = xp[XP_START].next;
  for (;;) {
    for (v=0; v<nvoice; v++) {
      k=xp[i].p[v];
      /* print dots for entertainment */
      if ((vb>=1) && (tnow>tprt)) {printf("."); fflush(stdout); tprt+=20;}

      if (voice[v].draw && (k>=0)) {
        n  = voice[v].nsym;
        t  = symv[v][k].type;
        if (vb>19)
          printf ("%3d tnow=%.3f   voice %d, type=%d\n",
                  i,tnow,v,t);

        /* if this a note or a rest, output the last one at k1 */
        if (( (t==NOTE) || (t==REST)) && (kpt[v]>=0)) {
          k1 = kpt[v];
          t1 = symv[v][k1].type;
          if (tgr1[v]<0) tgr1[v]=dgr0*symv[v][k1].gr.n;
          tgr2 = dgr0*symv[v][k].gr.n;
          if (tgr2 > 0.5*(tnow-tn[v])) tgr2=0.5*(tnow-tn[v]);
          if ((t1==NOTE) && (!in_tie[v])) {
            tend = tnow-tgr2;
            if (tend+tadd >= sb.t2) au_write_sbuf (tdone, &sb);
            au_play_note (v,k1,tn[v],tend,tadd,tgr1[v],atab1[v],key1[v],
                          q[v],amp,&sb);
            tn[v] = tnow;
            td[v] = tend;
          }
          if (t1==REST) {
            tn[v] = tnow;
            td[v] = tend;
          }
          tgr1[v]=tgr2;
        }

        switch (t) {

        case NOTE:
          au_set_atabs (v,k,atab[v]);
          if (in_tie[v]) {
            if (k==kk[v]) in_tie[v]=0;
            break;
          }
          kpt[v] = k;
          au_copy_atab (atab[v],atab1[v]);
          key1[v] = key[v];
          kk[v] = find_ties (i,v,k,n,atab[v],key[v],fc);
          amp=levx+(1-levx)*exp(-(tnow-tbar)/tdec);
          if (kk[v]>k) in_tie[v]=1;
          break;

        case REST:
          kpt[v] = k;
          break;

        case BAR:
          au_clear_atab (atab[v]);
          i=au_flow_control (i,v,k,&fc);
          tbar=tnow;
          break;

        case CLEF:
          key[v].ktype=symv[v][k].u;
          break;

        case KEYSIG:
          key[v].sf=symv[v][k].q;
          break;

        }
      }
    }

    /* calculate which part of the buffer can be written out */
    tdone=tnow;
    for (v=0; v<nvoice; v++)  {
      if ((voice[v].draw) && (td[v]<tdone)) tdone=td[v];
    }

    tnow = tnow + xp[i].dur*60.0/(tempo*QUARTER);
    i=xp[i].next;
    if (i==XP_END) i=au_flow_control (i,v,k,&fc);

    /* safety check */
    if (fc.nrep>20) {
      printf ("\n+++ Implausibly many repeats... terminate audio output\n");
      break;
    }
    if (i==XP_END) break;
  }

  /* output the last note in each voice */
  for (v=0; v<nvoice; v++) {
    if (voice[v].draw && (kpt[v]>=0)) {
      k1 = kpt[v];
      if (symv[v][k1].type==NOTE) {
        if (tgr1[v]<0) tgr1[v]=dgr0*symv[v][k1].gr.n;
        if (tnow+tadd >= sb.t2) au_write_sbuf (tdone, &sb);
        au_play_note (v,k1,tn[v],tnow,tadd,tgr1[v],atab1[v],key1[v],q[v],amp,&sb);
      }
     }
  }

  if (vb>=1) printf(" ");
  tsave=tnow;

}

char* prtr (struct TRIPLE t, char str[])
{
  int num;
  float x;

  num=1;
  x=t.low-t.mid; if (x*x > 0.0000001) num=3;
  x=t.hi-t.mid;  if (x*x > 0.0000001) num=3;

  if (num==1) {
    x=sqrt(t.low*t.low);
    if (x < 0.01)
      sprintf (str,"%.5f", t.low);
    else if (x < 0.1)
      sprintf (str,"%.3f", t.low);
    else
      sprintf (str,"%.2f", t.low);
  }
  else {
    x=t.hi*t.hi;
    if (t.mid*t.mid > x) x=t.mid*t.mid;
    if (t.low*t.low > x) x=t.low*t.low;
    x=sqrt(x);
    if (x < 0.01)
      sprintf (str,"%.5f %.5f %.5f", t.low, t.mid, t.hi);
    else if (x < 0.1)
      sprintf (str,"%.3f %.3f %.3f", t.low, t.mid, t.hi);
    else
      sprintf (str,"%.2f %.2f %.2f", t.low, t.mid, t.hi);
  }
  return str;
}


/* --- au_print_quality --- */
void au_print_quality (struct AU_QUALITY q)
{
  char t[81],tt[81];

  if (q.type==AU_SINE)     printf ("  sine wave\n");
  if (q.type==AU_TRIANGLE) printf ("  triangle, smooth %s\n",prtr(q.sm,t));
  if (q.type==AU_SAWTOOTH) printf ("  sawtooth, smooth %s\n",prtr(q.sm,t));
  printf ("  attack:      %s\n", prtr(q.att,t));
  printf ("  1st decay:   %s  with amplitude  %s\n", prtr(q.dec1,t), prtr(q.amp1,tt));
  printf ("  2nd decay:   %s  with amplitude  %s\n", prtr(q.dec2,t), prtr(q.amp2,tt));
  printf ("  final decay: %s\n", prtr(q.dec3,t));




}



/* --- output_audio ---*/
void output_audio ()
{
  float tempo;
  struct AU_QUALITY q[AU_MAXV];
  int l,s,i,j,ok;
  char auq[21],t[81],tt[81];

  tempo=cfmt.playtempo;

  /* set audio quality for the voices */
  strcpy (auq, cfmt.auquality);

  l=strlen(auq);
  s=1;
  for (i=0;i<l;i++) {
    s=auq[i]-'0';
    q[i] = au_set_quality (s);
  }
  if (nvoice>l) {
    for (i=l;i<nvoice;i++) {
      q[i] = au_set_quality (s);
    }
  }

  if (vb >= 4) {
    printf ("\nPlayback: tempo=%.0f\n", tempo);
    for (i=0;i<nvoice;i++) {
      printf ("voice %d: select set %d (%s)\n", i+1, q[i].num, q[i].name);
      if (vb >= 5) {
        ok=1;
        for (j=0;j<i;j++) if (q[i].num==q[j].num) ok=0;
        if (ok) au_print_quality (q[i]);
      }
    }
  }

  au_play_notes (tempo,q);


}





