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

#include <string.h>
#include <ctype.h>

#include "abc2ps.h"
#include "buffer.h"
#include "parse.h"
#include "subs.h"
#include "util.h"
#include "pssubs.h"
#include "style.h"
#include "format.h"

#include "music.h"

/*  subroutines connected with output of music  */

/* ----- nwid ----- */
/* Sets the prefered width for a note depending on the duration.
   Return value is default space on right and left side.
   Function is determined by values at 0, 1/2, 1.
   Return value is 1.0 for quarter note. */
float nwid (float dur)
{
  float a,b,x,p,x0,p0;

  a=2*(f1p-2*f5p+f0p);
  b=f1p-f0p-a;
  x = dur/(float)BASE;
  p = a*x*x + b*x + f0p;

  x0 = 0.25;
  p0 = a*x0*x0 + b*x0 + f0p;
  p  = p/p0;
  return p;
}

/* ----- xwid -- --- */
/* same as nwid but for stretched system */
float xwid (float dur)
{
  float a,b,x,p,x0,p0;

  a=2*(f1x-2*f5x+f0x);
  b=f1x-f0x-a;
  x = dur/(float)BASE;
  p = a*x*x + b*x + f0x;

  x0 = 0.25;
  p0 = a*x0*x0 + b*x0 + f0x;
  p  = p/p0;

  return p;
}

/* ----- next_note, prec_note ------ */
int next_note (k,n,symb)
int k,n;
struct SYMBOL symb[];
{
  int i;
  for (i=k+1;i<n;i++) {
    if ((symb[i].type==NOTE)||(symb[i].type==REST)) return i;
  }
  return -1;
}

int prec_note (k,n,symb)
int k,n;
struct SYMBOL symb[];
{
  int i;
  for (i=k-1;i>=0;i--) {
    if ((symb[i].type==NOTE)||(symb[i].type==REST)) return i;
  }
  return -1;
}


/* ----- followed_by_note ------ */
int followed_by_note (k,n,symb)
int k,n;
struct SYMBOL symb[];
{
  int i;

  for (i=k+1;i<n;i++) {
    switch (symb[i].type) {
    case INVISIBLE:
      break;
    case NOTE:
    case REST:
      return i;
      break;
    default:
      return -1;
    }
  }
  return -1;
}

/* ----- preceded_by_note ------ */
int preceded_by_note (k,n,symb)
int k,n;
struct SYMBOL symb[];
{
  int i;

  for (i=k-1;i>=0;i--) {
    switch (symb[i].type) {
    case INVISIBLE:
      break;
    case NOTE:
    case REST:
      return i;
      break;
    default:
      return -1;
    }
  }
  return -1;
}


void xch (i,j)  /* sub to exchange two integers */
int *i,*j;
{
  int k;
  k=*i;
  *i=*j;
  *j=k;
}



/* ----- print_linetype ----------- */
void print_linetype (t)
int t;
{

  if      (t== COMMENT)           printf("COMMENT\n");
  else if (t== MUSIC)             printf("MUSIC\n");
  else if (t== TO_BE_CONTINUED)   printf("TO_BE_CONTINUED\n");
  else if (t== E_O_F)             printf("E_O_F\n");
  else if (t== INFO)              printf("INFO\n");
  else if (t== TITLE)             printf("TITLE\n");
  else if (t== METER)             printf("METER\n");
  else if (t== PARTS)             printf("PARTS\n");
  else if (t== KEY)               printf("KEY\n");
  else if (t== XREF)              printf("XREF\n");
  else if (t== DLEN)              printf("DLEN\n");
  else if (t== HISTORY)           printf("HISTORY\n");
  else if (t== TEMPO)             printf("TEMPO\n");
  else if (t== BLANK)             printf("BLANK\n");
  else if (t== VOICE)             printf("VOICE\n");
  else                            printf("UNKNOWN LINE TYPE\n");
}

/* ----- print_syms: show sym properties set by parser ------ */
void print_syms(num1,num2,symb)
int num1,num2;
struct SYMBOL symb[];
{
  int i,t,j,y;
  char dsym[21] = {' ','~','.','J','M','H','u','v','R','T','K'};
  char bsym[10]  = {'-', '1' ,'2', '3', '4', '5', '6', '7', '8', '9'};
  char str[21];

  printf ("\n---------- Symbol list ----------\n");
  printf ("word  slur  eol  num  description\n");

  for (i=num1;i<num2;i++) {
    printf (" %c %c   %c %c   %c  ",
            bsym[symb[i].word_st], bsym[symb[i].word_end],
            bsym[symb[i].slur_st], bsym[symb[i].slur_end],
            bsym[symb[i].eoln] );
    printf ("%4d  ", i);
    t=symb[i].type;
    switch (t) {

    case NOTE:
    case REST:
      if (t==NOTE) printf ("NOTE ");
      if (t==REST) printf ("REST ");
      if (symb[i].npitch>1) printf (" [");
      for (j=0;j<symb[i].npitch;j++) {
        printf (" ");
        if (symb[i].accs[j]==A_SH) printf ("^");
        if (symb[i].accs[j]==A_NT) printf ("=");
        if (symb[i].accs[j]==A_FT) printf ("_");
        if (symb[i].accs[j]==A_DS) printf ("^^");
        if (symb[i].accs[j]==A_DF) printf ("__");
        y=3*(symb[i].pits[j]-18)+symb[i].yadd;
        strcpy (str,"z");
        if (t==NOTE) symbolic_pitch (symb[i].pits[j],str);
        printf ("%s(%d)", str, symb[i].lens[j]);
      }
      if (symb[i].npitch>1) printf (" ]");
      if (symb[i].p_plet)
        printf ("  (%d:%d:%d", symb[i].p_plet,symb[i].q_plet,symb[i].r_plet);
      if (strlen(symb[i].text)>0) printf ("  \"%s\"", symb[i].text);
      if (symb[i].dc.n>0) {
        printf ("  deco ");
        for (j=0;j<symb[i].dc.n;j++)
          printf ("%c",dsym[symb[i].dc.t[j]]);
      }
      if (symb[i].gr.n>0) {
        printf ("  grace ");
        for (j=0;j<symb[i].gr.n;j++) {
          if (j>0) printf ("-");
          if (symb[i].gr.a[j]==A_SH) printf ("^");
          if (symb[i].gr.a[j]==A_NT) printf ("=");
          if (symb[i].gr.a[j]==A_FT) printf ("_");
          symbolic_pitch (symb[i].gr.p[j],str);
          printf ("%s",str);
        }
      }
      break;

    case BAR:
      printf ("BAR  ======= ");
      if (symb[i].u==B_SNGL)  printf ("single");
      if (symb[i].u==B_DBL)   printf ("double");
      if (symb[i].u==B_LREP)  printf ("left repeat");
      if (symb[i].u==B_RREP)  printf ("right repeat");
      if (symb[i].u==B_DREP)  printf ("double repeat");
      if (symb[i].u==B_FAT1)  printf ("thick-thin");
      if (symb[i].u==B_FAT2)  printf ("thin-thick");
      if (symb[i].u==B_INVIS) printf ("invisible");
      if (symb[i].v) printf (", ending %d", symb[i].v);
      if (strlen(symb[i].text)>0) printf (", label \"%s\"",symb[i].text);
      break;

    case CLEF:
      if (symb[i].u==TREBLE)  printf ("CLEF  treble");
      if (symb[i].u==TREBLE8) printf ("CLEF  treble8");
      if (symb[i].u==BASS)    printf ("CLEF  bass");
      if (symb[i].u==ALTO)    printf ("CLEF  alto");
      break;

    case TIMESIG:
      printf ("TIMESIG  ");
      if (symb[i].w==1)      printf ("C");
      else if (symb[i].w==2) printf ("C|");
      else                   printf ("%d/%d", symb[i].u,symb[i].v);
      break;

    case KEYSIG:
      printf ("KEYSIG   ");
      if (symb[i].t==A_SH) printf ("sharps ");
      if (symb[i].t==A_FT) printf ("flats ");
      printf ("%d to %d", symb[i].u,symb[i].v);
      if (symb[i].w <= symb[i].v)
        printf (", neutrals from %d", symb[i].w);
      break;

    case INVISIBLE:
      printf ("INVIS   ");
      break;

    default:
      printf ("UNKNOWN ");
      break;
    }
    printf ("\n");

  }
  printf ("\n");
}

/* ----- print_vsyms: print symbols for all voices ----- */
void print_vsyms ()
{
  int i;

  printf("\n");
  for (i=0;i<nvoice;i++) {
    if (nvoice>1)
      printf ("Voice <%s> (%d of %d)", voice[i].id,i,nvoice);
    print_syms (0,voice[i].nsym,symv[i]);
  }
}


/* ----- set_head_directions ----------- */

/* decide whether to shift heads to other side of stem on chords */
/* also position accidentals to avoid too much overlap */
void set_head_directions (s)
struct SYMBOL *s;
{
  int i,n,nx,sig,d,da,shift,nac;
  int i1,i2,m;
  float dx,xx,xmn;

  n=s->npitch;
  sig=-1;
  if (s->stem>0) sig=1;
  for (i=0;i<n;i++) {
    s->shhd[i]=0;
    s->shac[i]=8;
    if (s->head==H_OVAL) s->shac[i]+=3;
    s->xmn=0;
    s->xmx=0;
  }
  if (n<2) return;

  /* sort heads by pitch */
  for (;;) {
    nx=0;
    for (i=1;i<n;i++) {
      if ( (s->pits[i]-s->pits[i-1])*sig>0 ) {
        xch (&s->pits[i],&s->pits[i-1]);
        xch (&s->lens[i],&s->lens[i-1]);
        xch (&s->accs[i],&s->accs[i-1]);
        xch (&s->sl1[i],&s->sl1[i-1]);
        xch (&s->sl2[i],&s->sl2[i-1]);
        xch (&s->ti1[i],&s->ti1[i-1]);
        xch (&s->ti2[i],&s->ti2[i-1]);
        nx++;
      }
    }
    if (!nx) break;
  }

  shift=0;                                 /* shift heads */
  for (i=n-2;i>=0;i--) {
    d=s->pits[i+1]-s->pits[i];
    if (d<0) d=-d;
    if ((d>=2) || (d==0))
      shift=0;
    else {
      shift=1-shift;
      if (shift) {
        dx=7.8;
        if (s->head==H_EMPTY) dx=7.8;
        if (s->head==H_OVAL)  dx=10.0;
        if (s->stem==-1) s->shhd[i]=-dx;
        else             s->shhd[i]=dx;
      }
    }
    if (s->shhd[i] < s->xmn) s->xmn = s->shhd[i];
    if (s->shhd[i] > s->xmx) s->xmx = s->shhd[i];
  }

  shift=0;                                 /* shift accidentals */
  i1=0; i2=n-1;
  if (sig<0) { i1=n-1; i2=0; }
  for (i=i1; ; i=i+sig) {          /* count down in terms of pitch */
    xmn=0;                         /* left-most pos of a close head */
    nac=99;                        /* relative pos of next acc above */
    for (m=0;m<n;m++) {
      xx=s->shhd[m];
      d=s->pits[m]-s->pits[i];
      da=ABS(d);
      if ((da<=5) && (s->shhd[m]<xmn)) xmn=s->shhd[m];
      if ((d>0) && (da<nac) && s->accs[m]) nac=da;
    }
    s->shac[i]=8.5-xmn+s->shhd[i];   /* aligns accidentals in column */
    if (s->head==H_EMPTY) s->shac[i] += 1.0;
    if (s->head==H_OVAL)  s->shac[i] += 3.0;
    if (s->accs[i]) {
      if (nac>=6)                        /* no overlap */
        shift=0;
      else if (nac>=4) {                 /* weak overlap */
        if (shift==0) shift=1;
        else          shift=shift-1;
      }
      else {                             /* strong overlap */
        if      (shift==0) shift=2;
        else if (shift==1) shift=3;
        else if (shift==2) shift=1;
        else if (shift==3) shift=0;
      }

      while (shift>=4) shift -=4;
      s->shac[i] += 3*shift;
    }
    if (i==i2) break;
  }

}


/* ----- set_minsyms: want at least one symbol in each voice --- */
void set_minsyms(ivc)
int ivc;
{
  int n2;

  n2=voice[ivc].nsym;
  printf ("set_minsyms:  n2=%d\n",n2);
  if (n2>0) return;

  symv[ivc][n2]=zsym;
  symv[ivc][n2].type = INVISIBLE;
  symv[ivc][n2].u    = 3;
  symv[ivc][n2].v    = 3;
  symv[ivc][n2].w    = 3;
  voice[ivc].nsym++;

}


/* ----- set_sym_chars: set symbol characteristics --- */
void set_sym_chars (n1,n2,symb)
int n1,n2;
struct SYMBOL symb[];
{
  int i,np,m,ymn,ymx;
  float yav,yy;

  for (i=n1;i<n2;i++) {
    if ((symb[i].type==NOTE)||(symb[i].type==REST)) {
      symb[i].y=3*(symb[i].pits[0]-18)+symb[i].yadd;
      if (symb[i].type==REST) symb[i].y=12;
      yav=0;
      ymn=1000;
      ymx=-1000;
      np=symb[i].npitch;
      for (m=0;m<np;m++) {
        yy=3*(symb[i].pits[m]-18)+symb[i].yadd;
        yav=yav+yy/np;
        if (yy<ymn) ymn=yy;
        if (yy>ymx) ymx=yy;
      }
      symb[i].ymn=ymn;
      symb[i].ymx=ymx;
      symb[i].yav=yav;
      symb[i].ylo=ymn;
      symb[i].yhi=ymx;

    }
  }
}

/* ----- set_beams: decide on beams ---- */
void set_beams (n1,n2,symb)
int n1,n2;
struct SYMBOL symb[];
{
  int j,lastnote,start_flag;

  /* separate words at notes without flags */
  start_flag=0;
  lastnote=-1;
  for (j=n1;j<n2;j++) {
    if (symb[j].type==NOTE) {
      if (start_flag) {
        symb[j].word_st=1;
        start_flag=0;
      }
      if (symb[j].flags==0) {
        if (lastnote>=0) symb[lastnote].word_end=1;
        symb[j].word_st=1;
        symb[j].word_end=1;
        start_flag=1;
      }
      lastnote=j;
    }
  }

}

/* ----- set_stems: decide on stem directions and lengths ---- */
void set_stems (n1,n2,symb)
int n1,n2;
struct SYMBOL symb[];
{
  int beam,j,k,n,stem,laststem;
  float avg,slen,lasty,dy;

  /* set stem directions; near middle, use previous direction */
  beam=0;
  laststem=0;
  for (j=n1; j<n2; j++) {
    if (symb[j].type!=NOTE) laststem=0;

    if (symb[j].type==NOTE) {

      symb[j].stem=0;
      if (symb[j].len<WHOLE) symb[j].stem=1;
      if (symb[j].yav>=12) symb[j].stem=-symb[j].stem;
      if ((symb[j].yav>11) && (symb[j].yav<13) && (laststem!=0)) {
        dy=symb[j].yav-lasty;
        if ((dy>-7) && (dy<7)) symb[j].stem=laststem;
      }

      if (symb[j].word_st && (!symb[j].word_end)) {   /* start of beam */
        avg=0;
        n=0;
        for (k=j;k<n2;k++) {
          if (symb[k].type==NOTE) {
            avg=avg+symb[k].yav;
            n++;
          }
          if (symb[k].word_end) break;
        }
        avg=avg/n;
        stem=1;
        if (avg>=12) stem=-1;
        if ((avg>11) && (avg<13) && (laststem!=0)) stem=laststem;
        beam=1;
      }

      if (beam) symb[j].stem=stem;
      if (symb[j].word_end) beam=0;
      if (bagpipe) symb[j].stem=-1;
      if (symb[j].len>=WHOLE) symb[j].stem=0;
      laststem=symb[j].stem;
      if (symb[j].len>=HALF) laststem=0;
      lasty=symb[j].yav;
    }
  }

  /* shift notes in chords (need stem direction to do this) */
  for (j=n1;j<n2;j++)
    if (symb[j].type==NOTE) set_head_directions (&symb[j]);

  /* set height of stem end, without considering beaming for now */
  for (j=n1; j<n2; j++) if (symb[j].type==NOTE) {
    slen=STEM;
    if (symb[j].npitch>1) slen=STEM_CH;
    if (symb[j].flags==3) slen += 4;
    if (symb[j].flags==4) slen += 9;
    if ((symb[j].flags>2) && (symb[j].stem==1)) slen -= 1;
    if (symb[j].stem==1) {
      symb[j].y=symb[j].ymn;
      symb[j].ys=symb[j].ymx+slen;
    }
    else if (symb[j].stem==-1) {
      symb[j].y=symb[j].ymx;
      symb[j].ys=symb[j].ymn-slen;
    }
    else {
      symb[j].y=symb[j].ymx;
      symb[j].ys=symb[j].ymx;
    }
  }
}

/* ----- set_sym_times: set time axis; also count through bars ----- */
int set_sym_times (n1,n2,symb,meter0)
int n1,n2;
struct SYMBOL symb[];
struct METERSTR meter0;
{
  int i,pp,qq,rr,meter1,meter2,count,bnum,lastb,bsave;
  int qtab[] = {0,0,3,2,3,0,2,0,3,0};
  float time,factor,fullmes;

  meter1=meter0.meter1;
  meter2=meter0.meter2;

  lastb=bnum=0;
  bsave=1;

  time=0.0;
  factor=1.0;
  for (i=n1;i<n2;i++) {
    symb[i].time=time;

    /* count through bar numbers, put into symb.t */
    if (symb[i].type==BAR) {
      fullmes=(WHOLE*meter1)/meter2;
      if (bnum==0) {
        bnum=barinit;
        if (fullmes<time+0.001) bnum++;
        symb[i].t=bnum;
        lastb=i;
      }
      else if (time-symb[lastb].time>=fullmes-0.001) {
        bnum++;
        while (symb[i+1].type==BAR) {i++; symb[i].time=time; }
        symb[i].t=bnum;
        lastb=i;
      }
      if (symb[i].v>1) {
        bnum=bsave;
        symb[i].t=0;
      }
      if (symb[i].v==1) bsave=bnum;
    }

    if ((symb[i].type==NOTE)||(symb[i].type==REST)) {
      if (symb[i].p_plet) {
        pp=symb[i].p_plet;
        qq=symb[i].q_plet;
        rr=symb[i].r_plet;
        if (qq==0) {
          qq=qtab[pp];
          if (qq==0) {
            qq=2;
            if (meter1%3==0) qq=3;
          }
        }
        factor=((float)qq)/((float)pp);
        count=rr;
      }
      time=time+factor*symb[i].len;
      if (count>0) count--;
      if (count==0) factor=1;
    }

    if (symb[i].type==TIMESIG) {     /* maintain meter as we go along */
      meter1=symb[i].u;
      meter2=symb[i].v;
    }

  }

  return bnum;

}

/* ----- set_sym_widths: set widths and prefered space --- */
/* This routine sets the minimal left and right widths wl,wr
   so that successive symbols are still separated when
   no extra glue is put between them. It also sets the prefered
   spacings pl,pr for good output and xl,xr for expanded layout.
   All distances in pt relative to the symbol center. */

void set_sym_widths (ns1,ns2,symb,ivc)
int ns1,ns2,ivc;
struct SYMBOL symb[];
{
  int i,n,j0,j,m,n1,n2,bt,k,sl,k1,k2,got_note,ok;
  float xx,w,swfac,spc,dur,yy;
  char t[81],tt[81];

  swfac=1.0;
  if (strstr(cfmt.vocalfont.name,"Times-Roman"))    swfac=1.00;
  if (strstr(cfmt.vocalfont.name,"Times-Bold"))     swfac=1.05;
  if (strstr(cfmt.vocalfont.name,"Helvetica"))      swfac=1.10;
  if (strstr(cfmt.vocalfont.name,"Helvetica-Bold")) swfac=1.15;

  got_note=0;
  for (i=ns1;i<ns2;i++) {
    switch (symb[i].type) {

    case INVISIBLE:   /* empty space; shrink,space,stretch from u,v,w */
      symb[i].wl=symb[i].wr=0.5*symb[i].u;
      symb[i].pl=symb[i].pr=0.5*symb[i].v;
      symb[i].xl=symb[i].xr=0.5*symb[i].w;
      break;

    case NOTE:
    case REST:
      got_note=1;
      dur=symb[i].len;
      symb[i].wl=symb[i].wr=4.5;
      if (symb[i].head==H_EMPTY) {symb[i].wl=6.0; symb[i].wr=14.0;}
      if (symb[i].head==H_OVAL)  {symb[i].wl=8.0; symb[i].wr=18.0;}
      symb[i].pl=symb[i].xl=symb[i].wl;
      symb[i].pr=symb[i].xr=symb[i].wr;

      /* room for shifted heads and accidental signs */
      for (m=0;m<symb[i].npitch;m++) {
        xx=symb[i].shhd[m];
        AT_LEAST (symb[i].wr, xx+6);
        AT_LEAST (symb[i].wl, -xx+6);
        if (symb[i].accs[m]) {
          xx=xx-symb[i].shac[m];
          AT_LEAST (symb[i].wl, -xx+3);
        }
        AT_LEAST (symb[i].xl, -xx+3);
        symb[i].pl=symb[i].wl;
        symb[i].xl=symb[i].wl;
      }

      /* room for slide */
      for (k=0;k<symb[i].dc.n;k++) {
        if (symb[i].dc.t[k]==D_SLIDE) symb[i].wl += 10;
      }

      /* room for grace notes */
      if (symb[i].gr.n>0) {
        xx=GSPACE0;
        if (symb[i].gr.a[0]) xx=xx+3.5;
        for (j=1;j<symb[i].gr.n;j++) {
          xx=xx+GSPACE;
          if (symb[i].gr.a[j]) xx=xx+4;
        }
        symb[i].wl = symb[i].wl + xx+1;
        symb[i].pl = symb[i].pl + xx+1;
        symb[i].xl = symb[i].xl + xx+1;
      }

      /* space for flag if stem goes up on standalone note */
      if (symb[i].word_st && symb[i].word_end)
        if ((symb[i].stem==1) && symb[i].flags>0)
          AT_LEAST (symb[i].wr, 12);

      /* leave room for dots */
      if (symb[i].dots>0) {
        AT_LEAST (symb[i].wr,12+symb[i].xmx);
        if (symb[i].dots>=2) symb[i].wr=symb[i].wr+3.5;
        n=(int) symb[i].y;
        /* special case: standalone with up-stem and flags */
        if (symb[i].flags && (symb[i].stem==1) && !(n%6))
          if ((symb[i].word_st==1) && (symb[i].word_end==1)) {
            symb[i].wr=symb[i].wr+DOTSHIFT;
            symb[i].pr=symb[i].pr+DOTSHIFT;
            symb[i].xr=symb[i].xr+DOTSHIFT;
          }
      }

      /* extra space when down stem follows up stem */
      j0=preceded_by_note(i,ns2,symb);
      if ((j0>=0) && (symb[j0].stem==1) && (symb[i].stem==-1))
        AT_LEAST (symb[i].wl, 7);

      /* make sure helper lines don't overlap */
      if ((j0>=0) && (symb[i].y>27) && (symb[j0].y>27))
        AT_LEAST (symb[i].wl, 7.5);

      /* leave room guitar chord */
      if (strlen(symb[i].text)>0) {
        /* special case: guitar chord under ending 1 or 2 */
        /* leave some room to the left of the note */
        if ((i>0) && (symb[i-1].type==BAR)) {
          bt=symb[i-1].v;
          if (bt) AT_LEAST (symb[i].wl, 18);
        }
        /* rest is same for all guitar chord cases */
        tex_str(symb[i].text,t,&w);
        xx=cfmt.gchordfont.size*w;
        xx=xx*1.05;     /* extra space mainly for helvetica font */
        spc=xx*GCHPRE;
        k1=prec_note(i,ns2,symb);
        k2=next_note(i,ns2,symb);
        if (spc>8.0) spc=8.0;
        if ((k1) && (strlen(symb[k1].text)>0))
          AT_LEAST (symb[i].wl, spc);
        if ((k2>0) && (strlen(symb[k2].text)>0))
          AT_LEAST (symb[i].wr, xx-spc);
      }

      /* leave room for vocals under note */
      for (j=0;j<NWLINE;j++) {
        if (symb[i].wordp[j]) {
          sl=tex_str(symb[i].wordp[j],t,&w);
          xx=swfac*cfmt.vocalfont.size*(w+2*cwid(' '));
          AT_LEAST (symb[i].wl,xx*VOCPRE);
          AT_LEAST (symb[i].wr,xx*(1.0-VOCPRE));
        }
      }

      AT_LEAST (symb[i].pl, symb[i].wl);
      AT_LEAST (symb[i].xl, symb[i].wl);
      AT_LEAST (symb[i].pr, symb[i].wr);
      AT_LEAST (symb[i].xr, symb[i].wr);
      break;

    case BAR:
      symb[i].wl=symb[i].wr=0;
      if (symb[i].u==B_SNGL)         symb[i].wl=symb[i].wr=3;
      else if (symb[i].u==B_DBL)   { symb[i].wl=7; symb[i].wr=4;  }
      else if (symb[i].u==B_LREP)  { symb[i].wl=3; symb[i].wr=12; }
      else if (symb[i].u==B_RREP)  { symb[i].wl=12; symb[i].wr=3; }
      else if (symb[i].u==B_DREP)    symb[i].wl=symb[i].wr=12;
      else if (symb[i].u==B_FAT1)  { symb[i].wl=3;  symb[i].wr=9; }
      else if (symb[i].u==B_FAT2)  { symb[i].wl=9;  symb[i].wr=3; }

      if (ivc==ivc0) {       /* bar numbers and labels next */
        ok=0;
        if ((cfmt.barnums>0) && (symb[i].t%cfmt.barnums==0)) ok=1;
        if (strlen(symb[i].text)>0) ok=1;
        if (ok) {
          if (strlen(symb[i].text)>0) {
            tex_str(symb[i].text,t,&w);
            xx=cfmt.barlabelfont.size*w*0.5;
          }
          else {
            sprintf (tt,"%d",symb[i].t);
            tex_str(tt,t,&w);
            xx=cfmt.barnumfont.size*w*0.5;
          }
          yy=60;
          if (!got_note) yy=0;
          if (symb[i-1].type==NOTE) {
            yy=symb[i-1].ymx;
            if (symb[i-1].stem==1) yy=yy+26;
          }
          if (strlen(symb[i-1].text)>0) yy=60;
          AT_LEAST (symb[i].wl, 2);
          if (yy>BNUMHT-4.0) AT_LEAST (symb[i].wl, xx+1);
          if (!got_note) AT_LEAST (symb[i].wl, xx+1);
          yy=0;
          if (symb[i+1].type==NOTE) yy=symb[i+1].ymx;
          if (yy>BNUMHT-4.0) AT_LEAST (symb[i].wr, xx+2);
          if (strlen(symb[i+1].text)>0) AT_LEAST (symb[i].wr, xx+4);
        }
      }

      symb[i].pl=symb[i].wl;
      symb[i].pr=symb[i].wr;
      symb[i].xl=symb[i].wl;
      symb[i].xr=symb[i].wr;
      break;

    case CLEF:
      symb[i].wl=symb[i].wr=symb[i].xl=13.5;
      symb[i].pl=symb[i].pr=symb[i].xr=11.5;
      break;

    case KEYSIG:
      n1=symb[i].u;
      n2=symb[i].v;
      if (n2>=n1) {
        symb[i].wl=2;
        symb[i].wr=5*(n2-n1+1)+2;
        symb[i].pl=symb[i].wl;
        symb[i].pr=symb[i].wr;
        symb[i].xl=symb[i].wl;
        symb[i].xr=symb[i].wr;
      }
      else {
        symb[i].wl=symb[i].pl=symb[i].xl=3;
        symb[i].wr=symb[i].pr=symb[i].xr=3;
      }
      break;

    case TIMESIG:
      symb[i].wl=8+4*(strlen(symb[i].text)-1);
      symb[i].wr=symb[i].wl+4;
      symb[i].pl=symb[i].wl;
      symb[i].pr=symb[i].wr;
      symb[i].xl=symb[i].wl;
      symb[i].xr=symb[i].wr;
      break;

    default:
      printf (">>> cannot set width for sym type %d\n", symb[i].type);
      symb[i].wl=symb[i].wr=symb[i].xl=0;
      symb[i].pl=symb[i].pr=symb[i].xr=0;
      break;
    }

  }
}



/* ----- contract_keysigs: delete duplicate keysigs at staff start ---- */
/* Depending on line breaks, a key and/or meter change can come
   at the start of a staff. Solution: scan through symbols from start
   of line as long as sym is a CLEF, KEYSIG, or TIMESIG. Remove all
   these, but set flags so that set_initsyms will draw the
   key and meter which result at the end of the scan. */
int contract_keysigs (ip1)
int ip1;
{
  int i,k,v,t,n3,sf,jp1,m,mtop;

  mtop=10000;
  for (v=0;v<nvoice;v++) {
    i=ip1;
    m=0;
    for (;;) {
      m++;
      k=xp[i].p[v];
      if (k>=0) {
        if (symv[v][k].type==CLEF) {
          voice[v].key.ktype=symv[v][k].u;
          symv[v][k].type=INVISIBLE;
        }
        else if (symv[v][k].type==KEYSIG) {
          sf=symv[v][k].v;
          n3=symv[v][k].w;
          if (n3-1<sf) sf=n3-1;
          t =symv[v][k].t;
          if (t==A_FT) sf=-sf;
          if (t==A_NT) sf=0;
          voice[v].key.sf=sf;
          symv[v][k].type=INVISIBLE;
        }
        else if (symv[v][k].type==TIMESIG) {
          voice[v].meter.insert=1;
          voice[v].meter.meter1=symv[v][k].u;
          voice[v].meter.meter2=symv[v][k].v;
          voice[v].meter.mflag =symv[v][k].w;
          strcpy(voice[v].meter.top,symv[v][k].text);
          symv[v][k].type=INVISIBLE;
        }
        else
          break;
      }
      i=xp[i].next;
      if (i==XP_END) {
        i=xp[i].prec;
        break;
      }
    }
    if (m<mtop && i>=0) {mtop=m; jp1=i; }
  }

  /* set glue for first symbol */
  xp[jp1].shrink = xp[jp1].wl+3;
  xp[jp1].space  = xp[jp1].wl+9;
  xp[jp1].stretch= xp[jp1].wl+16;

  return jp1;

}

/* ----- set_initsyms: set symbols at start of staff ----- */
int set_initsyms (v,wid0)
int v;
float *wid0;
{
  int   k,t,n;
  float x;

  k=0;

  /* add clef */
  sym_st[v][k]=zsym;
  sym_st[v][k].type = CLEF;
  sym_st[v][k].u    = voice[ivc].key.ktype;
  sym_st[v][k].v    = 0;
  k++;

  /* add keysig */
  sym_st[v][k]=zsym;
  sym_st[v][k].type = KEYSIG;
  n=voice[ivc].key.sf;
  t=A_SH;
  if (n<0) { n=-n; t=A_FT; }
  sym_st[v][k].u = 1;
  sym_st[v][k].v = n;
  sym_st[v][k].w = 100;
  sym_st[v][k].t = t;
  k++;

  /* add timesig */
  if (voice[ivc].meter.insert) {
    sym_st[v][k]=zsym;
    sym_st[v][k].type = TIMESIG;
    sym_st[v][k].u    = voice[ivc].meter.meter1;
    sym_st[v][k].v    = voice[ivc].meter.meter2;
    sym_st[v][k].w    = voice[ivc].meter.mflag;
    strcpy(sym_st[v][k].text,voice[ivc].meter.top);
    k++;
    voice[ivc].meter.insert=0;
  }

  if (voice[ivc].insert_btype) {
    sym_st[v][k].type = BAR;
    sym_st[v][k].u=voice[ivc].insert_btype;
    sym_st[v][k].v=voice[ivc].insert_num;
    sym_st[v][k].t=voice[ivc].insert_bnum;
    strcpy(sym_st[v][k].text,voice[ivc].insert_text);
    voice[ivc].insert_btype=0;
    voice[ivc].insert_bnum=0;
    k++;
  }

  n=k;
  set_sym_widths (0,n,sym_st[v],ivc);

  x=0;
  for (k=0;k<n;k++) {
    x=x+sym_st[v][k].wl;
    sym_st[v][k].x=x;
    x=x+sym_st[v][k].wr;
  }

  *wid0=x+voice[v].insert_space;
  return n;

}


/* ----- print_poslist ----- */
void print_poslist ()
{
  int i,n,typ,vv;

  printf ("\n----------- xpos list -----------\n");
  printf (" num ptr  type      time   dur     width    tfac"
          "   shrk  spac  stre  eol  vptr\n");

  n=0;
  i=xp[XP_START].next;
  for (;;) {
    typ=xp[i].type;
    printf ("%3d %3d  %d ", n,i,typ);
    if      (typ==NOTE)       printf ("NOTE");
    else if (typ==REST)       printf ("REST");
    else if (typ==KEYSIG)     printf ("KEY ");
    else if (typ==TIMESIG)    printf ("TSIG");
    else if (typ==BAR)        printf ("BAR ");
    else if (typ==CLEF)       printf ("CLEF");
    else if (typ==INVISIBLE)  printf ("INVS");
    else                      printf ("????");
    printf ("  %7.2f %6.2f  %4.1f %4.1f %5.2f  %5.1f %5.1f %5.1f",
            xp[i].time,xp[i].dur,xp[i].wl,xp[i].wr,xp[i].tfac,
            xp[i].shrink,xp[i].space,xp[i].stretch);
    if (xp[i].eoln)
      printf("  %d  ",xp[i].eoln);
    else
      printf("  -  ");
    for (vv=0;vv<nvoice;vv++) {
      if (xp[i].p[vv]>=0)
        printf(" %2d",xp[i].p[vv]);
      else
        printf("  -");
    }
    printf ("\n");
    i=xp[i].next;
    n++;
    if (i==XP_END) break;
  }

}




/* ----- insert_poslist: insert new element after element nins ----- */
int insert_poslist (nins)
int nins;
{
  int new,nxt,vv;

  new=ixpfree;
  ixpfree++;
  if (new>=XP_END)
    rxi("Too many symbols; use -maxs to increase limit, now ",maxSyms);

  nxt=xp[nins].next;
  xp[new].prec=nins;
  xp[new].next=nxt;
  xp[nins].next=new;
  xp[nxt].prec=new;
  for (vv=0;vv<nvoice;vv++) xp[new].p[vv]=-1;

  return new;

}

/* ----- set_poslist: make list of horizontal posits to align voices --- */
void set_poslist ()
{
  int i,n,v,vv,typ,nok,nins;
  float tol=0.01;
  float d,tim;

  /* initialize xp with data from top nonempty voice, ivc0 */
  v=ivc0;
  n=0;
  xp[0].next=1;
  for (i=0;i<voice[v].nsym;i++) {
    n++;
    xp[n].prec=n-1;
    xp[n].next=n+1;
    symv[v][i].p=n;
    for (vv=0;vv<nvoice;vv++) xp[n].p[vv]=-1;
    xp[n].p[v]=i;
    typ=symv[v][i].type;
    if (typ==REST) typ=NOTE;
    xp[n].type = typ;
    xp[n].time = symv[v][i].time;
    xp[n].dur = xp[n].tfac = 0;
    xp[n].shrink = xp[n].space = xp[n].stretch =0;
    xp[n].wl = xp[n].wr = 0;
    xp[n].eoln=symv[v][i].eoln;
  }
  xp[n].next=XP_END;
  xp[XP_END].prec=n;
  ixpfree=n+1;

  /* find or insert syms for other voices */
  for (v=0;v<nvoice;v++) {
    if (voice[v].draw && v!=ivc0) {
      n=XP_START;
      for (i=0;i<voice[v].nsym;i++) {
        tim=symv[v][i].time;
        typ=symv[v][i].type;
        if (typ==REST) typ=NOTE;
        nok=-1;
        nins=n;
        for (;;) {
          n=xp[n].next;
          if (n==XP_END) break;
          d=xp[n].time-tim;
          if (xp[n].time<tim-tol) nins=n;
          if (d*d<tol*tol && xp[n].type==typ) {
            nok=n;
            break;
          }
          if (xp[n].time>tim+tol) break;
        }
        if (nok>0)
          n=nok;
        else {
          n=insert_poslist (nins);
          xp[n].type=typ;
          xp[n].time=tim;
          xp[n].dur = xp[n].tfac = 0;
          xp[n].shrink = xp[n].space = xp[n].stretch =0;
          xp[n].wl = xp[n].wr = 0;
          xp[n].eoln=0;
        }
        symv[v][i].p=n;
        xp[n].p[v]=i;
      }
    }
  }

/*|   print_poslist (); |*/

}


/* ----- set_xpwid: set symbol widths and tfac in xp list ----- */
void set_xpwid()
{
  int i,j,k,i1,i2,k1,k2,v,nsm;
  float fac,dy,ff,wv,wx;

  /* set all tfacs to 1.0 */
  i=i1=xp[XP_START].next;
  for (;;) {
    xp[i].tfac=1.0;
    xp[i].wl=xp[i].wr=WIDTH_MIN;
    i2=i;
    i=xp[i].next;
    if (i==XP_END) break;
  }

  /* loop over voices. first voice last, assumed most important */
  for (v=nvoice-1;v>=0;v--) {
    nsm=voice[v].nsym;

    /* first symbol and last symbol */
    k1=symv[v][0].p;
    k2=symv[v][nsm-1].p;
    if (k1==i1 && symv[v][0].wl>xp[k1].wl)     xp[k1].wl=symv[v][0].wl;
    if (k2==i2 && symv[v][nsm-1].wr>xp[k2].wr) xp[k2].wr=symv[v][nsm-1].wr;

    /* loop over symbol nn pairs */
    for (i=1;i<nsm;i++) {
      j=i-1;
      k1=symv[v][j].p;
      k2=symv[v][i].p;
      if (xp[k1].next==k2) {
        if (symv[v][j].wr>xp[k1].wr) xp[k1].wr=symv[v][j].wr;
        if (symv[v][i].wl>xp[k2].wl) xp[k2].wl=symv[v][i].wl;

        if (symv[v][j].type==NOTE && symv[v][i].type==NOTE) {
          fac=1.0;
          /* reduce distance under a beam */
          if (symv[v][i].word_st==0) fac=fac*fnnp;
          /* reduce distance for large jumps in pitch */
          dy=symv[v][i].y-symv[v][j].y;
          if (dy<0) dy=-dy;
          ff=1-0.010*dy;
          if (ff<0.9) ff=0.9;
          fac=fac*ff;
          xp[k2].tfac=fac;
        }
      }
    }
  }

  /* check for all nn pairs in voice, in case some syms very wide */
  for (v=nvoice-1;v>=0;v--) {
    nsm=voice[v].nsym;
    for (i=1;i<nsm;i++) {
      j=i-1;
      k1=symv[v][j].p;
      k2=symv[v][i].p;
      if (xp[k1].next!=k2) {
        wv=symv[v][j].wr+symv[v][i].wl;
        wx=xp[k1].wr+xp[k2].wl;
        k=k1;
        for (;;) {
          k=xp[k].next;
          if (k==k2) break;
          wx=wx+xp[k].wl+xp[k].wr;
        }
        if(wx<wv) {
          fac=wv/wx;
          xp[k1].wr=fac*xp[k1].wr;
          xp[k2].wl=fac*xp[k2].wl;
          k=k1;
          for (;;) {
            k=xp[k].next;
            if (k==k2) break;
            xp[k].wl=fac*xp[k].wl;
            xp[k].wr=fac*xp[k].wr;
          }

        }
      }
    }
  }

}


/* ----- set_spaces: set the shrink,space,stretch distances ----- */
void set_spaces ()
{
  int i,j,n,nxt,typ,typl,meter1,meter2;
  float w0,w1,w2;
  float vbnp,vbnx,vnbp,vnbx;

  /* note lengths for spaces at bars. Use dcefault meter for now */
  meter1=default_meter.meter1;
  meter2=default_meter.meter2;
  vbnp=(rbnp*meter1*BASE)/meter2;
  vbnx=(rbnx*meter1*BASE)/meter2;
  vnbp=(rnbp*meter1*BASE)/meter2;
  vnbx=(rnbx*meter1*BASE)/meter2;

  /* redefine durations as differences in start times */
  n=0;
  i=xp[XP_START].next;
  for (;;) {
    nxt=xp[i].next;
    if (nxt!=XP_END) xp[i].dur=xp[nxt].time-xp[i].time;
    i=nxt;
    n++;
    if (i==XP_END) break;
  }

  i=xp[XP_START].next;
  j=-1;
  typl=0;
  for (;;) {
    nxt=xp[i].next;
    typ=xp[i].type;

    /* shrink distance is sum of left and right widths */
    if (j>=0)
      xp[i].shrink=xp[j].wr+xp[i].wl;
    else
      xp[i].shrink=xp[i].wl;
    xp[i].space=xp[i].stretch=xp[i].shrink;

    if (xp[i].type==NOTE) {

      if (typl==NOTE) {             /* note after another note */
        w1 = lnnp*nwid(xp[j].dur);
        w2 = lnnp*nwid(xp[i].dur);
        xp[i].space = bnnp*w1 + (1-bnnp)*0.5*(w1+w2);
        w1 = lnnx*xwid(xp[j].dur);
        w2 = lnnx*xwid(xp[i].dur);
        xp[i].stretch = bnnx*w1 + (1-bnnx)*0.5*(w1+w2);
      }

      else {                        /* note at start of bar */
        w1 = lbnp*nwid(xp[i].dur);
        w0 = lbnp*nwid(vbnp);
        if (w0>w1) w0=w1;
        xp[i].space = bbnp*w1 + (1-bbnp)*w0 + xp[j].wr;
        if (xp[i].space<14.0) xp[i].space=14.0;
        w1 = lbnx*xwid(xp[i].dur);
        w0 = lbnx*xwid(vbnp);
        if (w0>w1) w0=w1;
        xp[i].stretch = bbnx*w1 + (1-bbnx)*w0 + xp[j].wr;
        if (xp[i].stretch<18.0) xp[i].stretch=18.0;
        if (xp[i].shrink<12.0) xp[i].shrink=12.0;
      }
    }

    else {                         /* some other symbol after note */
      if (typl==NOTE) {
        w1 = lnbp*nwid(xp[j].dur);
        w0 = lnbp*nwid(vnbp);
        xp[i].space = bnbp*w1 + (1-bnbp)*w0 + xp[i].wl;
        if (xp[i].space<13.0) xp[i].space=13.0;
        w1 = lnbx*xwid(xp[j].dur);
        w0 = lnbx*xwid(vnbx);
        xp[i].stretch = bnbx*w1 + (1-bnbx)*w0 + xp[i].wl;
        if (xp[i].stretch<17.0) xp[i].stretch=17.0;
      }
    }

    /* multiply space and stretch by factors tfac */
    xp[i].space   = xp[i].space*xp[i].tfac;
    xp[i].stretch = xp[i].stretch*xp[i].tfac;

    /* make sure that shrink < space < stretch */
    if (xp[i].space<xp[i].shrink)  xp[i].space=xp[i].shrink;
    if (xp[i].stretch<xp[i].space) xp[i].stretch=xp[i].space;

    j=i;
    typl=typ;
    i=nxt;

    if (i==XP_END) break;
  }

  if (verbose>=11) print_poslist ();

}

/* ----- check_overflow: returns upper limit which fits on staff ------ */
int check_overflow (int ip1, int ip2, float width)
{

  int i,jp2,lastcut,nbar,need_note;
  float space,shrink,stretch,alfa,alfa0;

  /* max shrink is alfa0 */
  alfa0=ALFA_X;
  if (cfmt.continueall) alfa0=cfmt.maxshrink;
  if (gmode==G_SHRINK)  alfa0=1.0;
  if (gmode==G_SPACE)   alfa0=0.0;
  if (gmode==G_STRETCH) alfa0=1.0;

  jp2=ip2;
  space=shrink=stretch=0;
  lastcut=-1;
  nbar=0;
  need_note=1;
  i=ip1;
  for (;;) {
    space=space+xp[i].space;
    shrink=shrink+xp[i].shrink;
    stretch=stretch+xp[i].stretch;
    alfa=0;
    if (space>shrink) {
      alfa=(space-width)/(space-shrink);
      if (xp[i].type!=BAR)
        alfa=((space+8)-width)/(space-shrink);
    }

    if (alfa>alfa0) {
      if (!cfmt.continueall) {
        if (verbose<=3) printf ("\n");
        printf ("+++ Overfull after %d bar%s in staff %d\n",
                nbar, nbar==1 ? "" : "s", mline);
      }
      jp2=i;
      if (i==ip1) jp2=xp[i].next;
      if (lastcut>=0) jp2=xp[lastcut].next;
      break;
    }
    /* The need_note business is to cut at the first of consecutive bars */
    if (xp[i].type==NOTE)  need_note=0;
    if (xp[i].type==BAR && need_note==0) {lastcut=i; need_note=1; nbar++; }
    if (xp[i].type==KEYSIG) lastcut=i;
    i=xp[i].next;
    if (i==ip2) break;
  }

  return jp2;

}


/* ----- set_glue --------- */
float set_glue (int ip1, int ip2, float width)
{

  int i,j;
  float space,shrink,stretch,alfa,beta,glue,w,x,d1,d2,w1;
  float alfa0,beta0;

  alfa0=ALFA_X;                       /* max shrink and stretch */
  if (cfmt.continueall) alfa0=cfmt.maxshrink;
  if (gmode==G_SHRINK)  alfa0=1.0;
  if (gmode==G_SPACE)   alfa0=0.0;
  if (gmode==G_STRETCH) alfa0=1.0;
  beta0=BETA_X;
  if (cfmt.continueall) beta0=BETA_C;


  space=shrink=stretch=0;
  i=ip1;
  for (;;) {
    space=space+xp[i].space;
    shrink=shrink+xp[i].shrink;
    stretch=stretch+xp[i].stretch;
    j=i;
    i=xp[i].next;
    if (i==ip2) break;
  }

  /* add extra space if last symbol is not a bar */
  if (xp[j].type!=BAR) {
    d1=d2=xp[j].wr+3;
    if (xp[j].type==NOTE) d2 = lnbp*nwid(xp[j].dur)+xp[j].wr;
    if (d2<d1) d2=d1;
    shrink  = shrink  + d1;
    space   = space   + d2;
    stretch = stretch + d2;
  }

  /* select alfa and beta */
  alfa=beta=0;
  if (space>width) {
    alfa=99;
    if (space>shrink) alfa=(space-width)/(space-shrink);
  }
  else {
    beta=99;
    if (stretch>space) beta=(width-space)/(stretch-space);
    if (!cfmt.stretchstaff) beta=0;
  }

  if (gmode==G_SHRINK)  { alfa=1; beta=0;}     /* force minimal spacing */
  if (gmode==G_STRETCH) { alfa=0; beta=1;}     /* force stretched spacing */
  if (gmode==G_SPACE)   { alfa=beta=0;   }     /* force natural spacing */

/*|   if (alfa>alfa0) { alfa=alfa0; beta=0; } |*/

  if (beta>beta0) {
    if (!cfmt.continueall) {
      if (verbose<=3) printf ("\n");
      printf ("+++ Underfull (%.0fpt of %.0fpt) in staff %d\n",
              (beta0*stretch+(1-beta0)*space)*cfmt.scale,
              cfmt.staffwidth,mline);
    }
    alfa=0;
    if (!cfmt.stretchstaff) beta=0;
    if ((!cfmt.stretchlast) && (ip2==XP_END)) {
      w1=alfa_last*shrink+beta_last*stretch+(1-alfa_last-beta_last)*space;
      if (w1<width) {
        alfa=alfa_last;    /* shrink undefull last line same as previous */
        beta=beta_last;
      }
    }
  }

  w=alfa*shrink+beta*stretch+(1-alfa-beta)*space;
  if (verbose>=7) {
    if (alfa>0)      printf ("Shrink staff %.0f%%",  100*alfa);
    else if (beta>0) printf ("Stretch staff %.0f%%", 100*beta);
    else             printf ("No shrink or stretch");
    printf (" to width %.0f (%.0f,%.0f,%.0f)\n",w,shrink,space,stretch);
  }

  /* now calculate the x positions */
  x=0;
  i=ip1;
  for (;;) {
    glue=alfa*xp[i].shrink+beta*xp[i].stretch+(1-alfa-beta)*xp[i].space;
    x=x+glue;
    xp[i].x=x;
    if (verbose>22) printf ("pos[%d]: type=%d  pos=%.2f\n", i,xp[i].type,x);
    i=xp[i].next;
    if (i==ip2) break;
  }

  alfa_last=alfa;
  beta_last=beta;
  return w;

}


/* ----- adjust_group: even out spacings for one group of notes --- */
/* Here we repeat the whole glue thing, in rudimentary form */
void adjust_group(i1,i2)
int i1,i2;
{
  int j;
  float dx,x,spa,shr,str,hp,hx,alfa,beta,dur1;

  dx=sym[i2].x-sym[i1].x;
  shr=sym[i1].wr+sym[i2].wl;
  for (j=i1+1;j<i2;j++) shr=shr+sym[j].wl+sym[j].wr;
  dur1=sym[i1].len;
  hp=lnnp*nwid(dur1);
  hx=lnnx*xwid(dur1);
  spa = (i2-i1)*hp;
  str = (i2-i1)*hx;

  alfa=beta=0;
  if (dx>spa)
    beta=(dx-spa)/(str-spa);
  else
    alfa=(dx-spa)/(shr-spa);

  x=sym[i1].x;
  for (j=i1+1;j<=i2;j++) {
    x=x+alfa*(sym[j-1].wr+sym[j].wl)+beta*hx+(1-alfa-beta)*hp;
    sym[j].x=x;
  }

}


/* ----- adjust_spacings: even out triplet spacings etc --- */
void adjust_spacings (n)
int n;
{
  int i,i1,count,beam,num;

  /* adjust the n-plets */
   count=0;
   for (i=1;i<n;i++) {
     if ((sym[i].type==NOTE)||(sym[i].type==REST)) {
       if (sym[i].p_plet) {
         i1=i;
         count=sym[i].r_plet;
       }
       if (count>0 && sym[i].len!=sym[i1].len) count=0;
       if (count==1) adjust_group (i1,i);
       if (count>0) count--;
     }
     else
       count=0;
   }

  /* adjust beamed notes of equal duration */
  beam=0;
  for (i=1;i<n;i++) {
    if ((sym[i].type==NOTE)||(sym[i].type==REST)) {
      if (sym[i].word_st && (!sym[i].word_end)) {
        i1=i;
        beam=1;
        if (sym[i].p_plet) beam=0;          /* don't do nplets here */
      }
      if (beam && sym[i].len!=sym[i1].len) beam=0;
      if (beam && sym[i].word_end) {
        num=i-i1+1;
        if (num>2 && num<4) adjust_group (i1,i);
      }
      if (sym[i].word_end) beam=0;
    }
    else
      beam=0;
  }

}


/* ----- adjust_rests: position single rests in bar center */
void adjust_rests (n,v)
int n,v;
{
  int i,ok;

  for (i=2;i<n-1;i++) {
    if ((sym[i].type==REST) && sym[i].fullmes) {

      ok=1;
      if ((sym[i-1].type==REST) || (sym[i-1].type==NOTE)) ok=0;
      if ((sym[i+1].type==REST) || (sym[i+1].type==NOTE)) ok=0;

      if (ok) {
        sym[i].head = H_OVAL;
        sym[i].dots = 0;
        sym[i].x = 0.5*(sym[i-1].x+sym[i+1].x);
      }

    }
  }

}



/* ----- copy_vsyms: copy selected syms for voice to v sym --- */
int copy_vsyms (int v, int ip1,int ip2, float wid0)
{
  int i,n,m,k;
  float d1,d2,r,x;

  /* copy staff initialization symbols */
  n=0;
  for (i=0;i<nsym_st[v];i++) {
    sym[n]=sym_st[v][i];
    n++;
  }

  /* copy real symbols, shifted by wid0 */
  i=ip1;
  m=0;
  for (;;) {
    k=xp[i].p[v];
    if (k >= 0) {
      sym[n]=symv[v][k];
      sym[n].x=xp[i].x+wid0;
      n++;
      m++;
    }
    i=xp[i].next;
    if (i==ip2) break;
  }

  /* adjust things for more pretty output.. */
  adjust_rests (n,v);
  if (mvoice>1) adjust_spacings (n);

  /* small random shifts make the output more human... */
  for (i=1;i<n-1;i++) {
    if ((sym[i].type==NOTE) || (sym[i].type==REST)) {
      d1=sym[i].x-sym[i-1].x;
      d2=sym[i+1].x-sym[i].x;
      r=RANFAC*d1;
      if (d2<d1) r=RANFAC*d2;
      if (r>RANCUT) r=RANCUT;
      x=ranf(-r,r);
      sym[i].x=sym[i].x+x;
    }
  }

  return n;

}


/* ----- draw_timesig ------- */
void draw_timesig (float x, struct SYMBOL s)
{
  if (s.w==1)
    PUT1("%.1f csig\n", x)
  else if (s.w==2)
    PUT1("%.1f ctsig\n", x)
  else
/*    PUT3("%.1f (%d) (%d) tsig\n", x, s.u, s.v) */
    PUT3("%.1f (%s) (%d) tsig\n", x, s.text, s.v)
}

/* ----- draw_keysig: return sf for this key ----- */
int draw_keysig (float x, struct SYMBOL s)
{
  float p;
  int i,n1,n2,n3,t,yad,sf;
  int sh_pos[8]={0,  24,15,27,18,9,21,15};
  int ft_pos[8]={0,  12,21,9,18,6,15,3};

  n1=s.u;               /* which symbol to start with */
  n2=s.v;               /* up to which symbol to go */
  n3=s.w;               /* draw neutrals instead starting from this one */
  t =s.t;               /* type of symbol: sharp or flat */

  yad = 0;
  if (voice[ivc].key.ktype==BASS) yad = -6;
  if (voice[ivc].key.ktype==ALTO) yad = -3;

  if (n2>7) {
    printf ("+++ Keysig seems to have %d symbols ???\n", n2);
    return 0;
  }

  sf=0;
  if (t==A_SH) {
    p=x;
    for (i=n1;i<=n2;i++) {
      if (i>=n3)
        PUT2("%.1f %d nt0 ",p,sh_pos[i]+yad)
      else {
        sf++;
        PUT2("%.1f %d sh0 ",p,sh_pos[i]+yad)
      }
      p=p+5;
    }
    PUT0("\n")
  }
  else if (t==A_FT) {
    p=x;
    for (i=n1;i<=n2;i++) {
      if (i>=n3)
        PUT2("%.1f %d nt0 ", p, ft_pos[i]+yad)
      else {
        sf--;
        PUT2("%.1f %d ft0 ", p, ft_pos[i]+yad)
      }
      p=p+5;
    }
    PUT0("\n")
  }
  else
    bug ("wrong type in draw_keysig", 0);

  return sf;
}


/* ----- draw_bar ------- */
void draw_bar (float x, struct SYMBOL s)
{

  if (s.u==B_SNGL)                        /* draw the bar */
    PUT1("%.1f bar\n", x)
  else if (s.u==B_DBL)
    PUT1("%.1f dbar\n", x)
  else if (s.u==B_LREP)
    PUT2("%.1f fbar1 %.1f rdots\n", x, x+10)
  else if (s.u==B_RREP) {
    PUT2("%.1f fbar2 %.1f rdots\n", x, x-10)
  }
  else if (s.u==B_DREP) {
    PUT2("%.1f fbar1 %.1f rdots\n", x-1, x+9)
    PUT2("%.1f fbar2 %.1f rdots\n", x+1, x-9)
  }
  else if (s.u==B_FAT1)
    PUT1("%.1f fbar1\n", x)
  else if (s.u==B_FAT2)
    PUT1("%.1f fbar2\n", x)
  else if (s.u==B_INVIS)
    ;
  else
    printf (">>> dont know how to draw bar type %d\n", s.u);

  PUT0("\n")

}


/* ----- draw_barnums ------- */
void draw_barnums (fp)
FILE *fp;
{
  int i,last,ok,got_note;

  last=0;
  got_note=0;
  for (i=0;i<nsym;i++) {
    if ((sym[i].type==NOTE)||(sym[i].type==REST)) got_note=1;

    if ((sym[i].type==BAR) && (strlen(sym[i].text)>0)) {
      if (last != 2) set_font (fp, cfmt.barlabelfont, 0);
      PUT3 (" %.1f %.1f M (%s) cshow ", sym[i].x, BNUMHT, sym[i].text)
      last=2;
    }

    if ((sym[i].type==BAR) && sym[i].t) {
      ok=0;
      if ((cfmt.barnums>0) && (sym[i].t%cfmt.barnums==0)) ok=1;
      if ((cfmt.barnums==0) && (!got_note)) ok=1;
      if ((cfmt.barnums!=0) && ((strlen(sym[i].text)>0))) ok=0;

      if (ok) {
        if (last != 1) set_font (fp, cfmt.barnumfont, 0);
/*|     if ((mvoice>1) && (cfmt.barnums==0))  |*/
        if (cfmt.barnums==0)
          PUT1 (" 0 38 M (%d) rshow ", sym[i].t)
        else
          PUT3 (" %.1f %.1f M (%d) cshow ", sym[i].x, BNUMHT, sym[i].t)
        last=1;
      }
    }
  }
  PUT0("\n");

}


/* ----- update_endings: remember where to draw endings ------- */
void update_endings (float x,struct SYMBOL s)
{
  int i;

  if (num_ending>0) {
    i=num_ending-1;
    if (ending[i].num==1)
      mes1++;
    else {
      mes2++;
      if (mes2==mes1) ending[i].b=x;
    }
  }

  if (s.v) {
    if (num_ending>0)
      if (ending[num_ending-1].num==1) ending[num_ending-1].b=x-3;
    ending[num_ending].a=x;
    ending[num_ending].b=-1;
    ending[num_ending].num=s.v;
    if (s.v==1) mes1=0;
    else        mes2=0;
    num_ending++;
  }

}



/* ----- set_ending: determine limits of ending box ------- */
void set_ending (i)
int i;
{
  int num,j,j0,j1,mes,mesmax;
  float top;

  num=sym[i].v;
  mesmax=0;
  if (num==2) mesmax=mes1;

  mes=0;
  j0=j1=-1;
  for (j=i+1;j<nsym;j++) {
    if (sym[j].type==BAR) {
      if (sym[j].u!=B_INVIS)  mes++;
      if (mes==1) j1=j;
      if (sym[j].u==B_RREP || sym[j].u==B_DREP || sym[j].u==B_FAT2 ||
          sym[j].u==B_LREP || sym[j].u==B_FAT1 || sym[j].v>0) {
        j0=j;
        break;
      }
      if (mes==mesmax) {
        j0=j;
        break;
      }
    }
  }
  top=-1;
  if (j0==-1) j0=j1;
  if (j0>=0) top=sym[j0].x;

  ending[num_ending].num=num;
  ending[num_ending].a=sym[i].x;
  ending[num_ending].b=top;
  if (num==1) ending[num_ending].b=top-3;
  ending[num_ending].type=E_CLOSED;
  if (sym[j0].type==BAR && sym[j0].u==B_SNGL) ending[num_ending].type=E_OPEN;
  num_ending++;

  if (num==1) mes1=mes;
  if (num==2) mes1=0;

}


/* ----- draw_endings ------- */
void draw_endings ()
{
  int i;

  for (i=0;i<num_ending;i++) {
    if (ending[i].b<0)
      PUT3("%.1f %.1f (%d) end2\n",
           ending[i].a, ending[i].a+50, ending[i].num)
    else {
      if (ending[i].type==E_CLOSED) {
        PUT3("%.1f %.1f (%d) end1\n",
             ending[i].a, ending[i].b, ending[i].num)
      }
      else {
        PUT3("%.1f %.1f (%d) end2\n",
             ending[i].a, ending[i].b, ending[i].num)
      }
    }
  }
  num_ending=0;

}

/* ----- draw_rest ----- */
void draw_rest (float x, float yy, struct SYMBOL s, float *gchy)
{

  int y,i;
  float dotx,doty;

  *gchy=38;
  if (s.invis) return;

  y=(int) s.y;
  PUT2("%.2f %.0f", x, yy)

  if (s.head==H_OVAL)       PUT0(" r1")
  else if (s.head==H_EMPTY) PUT0(" r2")
  else {
    if (s.flags==0)      PUT0(" r4")
    else if (s.flags==1) PUT0(" r8")
    else if (s.flags==2) PUT0(" r16")
    else if (s.flags==3) PUT0(" r32")
    else                 PUT0(" r64")
  }

  if (y%6) { dotx=6.5; doty=0; }                   /* dots */
  else     { dotx=6.5; doty=3; }
  if (s.head==H_OVAL)  { dotx=8; doty=-3; }
  if (s.head==H_EMPTY) { dotx=8; doty=3;  }
  for (i=0;i<s.dots;i++) {
    PUT2(" %.1f %.1f dt", dotx, doty)
    dotx=dotx+3.5;
  }


  PUT0("\n")
}

/* ----- draw_gracenotes ----- */
void draw_gracenotes (float x, float w, float d, struct SYMBOL *s)
{
  int i,n,y,acc,ii,m;
  float xg[20],yg[20],lg,px[20],py[20],xx,yy;
  float s1,sx,sy,sxx,sxy,a,b,delta,lmin;
  float x0,y0,x1,y1,x2,y2,x3,y3,bet1,bet2,dy1,dy2,dx,dd,fac,facx;

  n=s->gr.n;
  if (n==0) return;

  facx=0.3;
  fac=d/w-1;
  if (fac<0) fac=0;
  fac=1+(fac*facx)/(fac+facx);

  dx=0;
  for (m=0;m<s->npitch;m++) {              /* room for accidentals */
    dd=-s->shhd[m];
    if (s->accs[m]) dd=-s->shhd[m]+s->shac[m];
    if ((s->accs[m]==A_FT)||(s->accs[m]==A_NT)) dd=dd-2;
    if (dx<dd) dx=dd;
  }

  xx=x-fac*(dx+GSPACE0);
  for (i=n-1;i>=0;i--) {                   /* set note positions */
    yg[i]=3*(s->gr.p[i]-18)+s->yadd;
    if (i==n-1) {                             /* some subtle shifts.. */
      if(yg[i]>=s->ymx)  xx=xx+1;              /* gnote above a bit closer */
      if((yg[i]<s->ymn-7)&&(n==1)) xx=xx-2;   /* below with flag further */
    }

    if (i<n-1) {
      if (yg[i]>yg[i+1]+8) xx=xx+fac*1.8;
    }

    xg[i]=xx;
    xx=xx-fac*GSPACE;
    if (s->gr.a[i]) xx=xx-3.5;
  }

  if (n>1) {
    s1=sx=sy=sxx=sxy=0;                    /* linear fit through stems */
    for (i=0;i<n;i++) {
      px[i]=xg[i]+GSTEM_XOFF;
      py[i]=yg[i]+GSTEM;
      s1 += 1; sx += px[i]; sy += py[i];
      sxx += px[i]*px[i]; sxy += px[i]*py[i];
    }
    delta=s1*sxx-sx*sx;                   /* beam fct: y=ax+b */
    a=(s1*sxy-sx*sy)/delta;
    if (a>BEAM_SLOPE) a=BEAM_SLOPE;
    if (a<-BEAM_SLOPE) a=-BEAM_SLOPE;
    b=(sy-a*sx)/s1;

    if (bagpipe) { a=0; b=35; }

    lmin=100;                           /* shift to get min stems */
    for (i=0;i<n;i++) {
      px[i]=xg[i]+GSTEM_XOFF;
      py[i]=a*px[i]+b;
      lg=py[i]-yg[i];
      if (lg<lmin) lmin=lg;
    }
    if (lmin<10) b=b+10-lmin;
  }

  for (i=0;i<n;i++) {                     /* draw grace notes */
    if (n>1) {
      px[i]=xg[i]+GSTEM_XOFF;
      py[i]=a*px[i]+b;
      lg=py[i]-yg[i];
      PUT3("%.1f %.1f %.1f gnt ", xg[i],yg[i],lg)
    }
    else {
      lg=GSTEM;
      PUT3("%.1f %.1f %.1f gn1 ", xg[i],yg[i],lg)
    }

    acc=s->gr.a[i];
    if (acc==A_SH) PUT2("%.1f %.1f gsh0 ",xg[i]-4.5,yg[i])
    if (acc==A_FT) PUT2("%.1f %.1f gft0 ",xg[i]-4.5,yg[i])
    if (acc==A_NT) PUT2("%.1f %.1f gnt0 ",xg[i]-4.5,yg[i])
    if (acc==A_DS) PUT2("%.1f %.1f gds0 ",xg[i]-4.5,yg[i])
    if (acc==A_DF) PUT2("%.1f %.1f gdf0 ",xg[i]-4.5,yg[i])

    y = (int)yg[i];                         /* helper lines */
    if (y<=-6) {
      if (y%6) PUT2("%.1f %d ghl ",xg[i], y+3)
      else     PUT2("%.1f %d ghl ",xg[i], y)
    }
    if (y>=30) {
      if (y%6) PUT2("%.1f %d ghl ",xg[i], y-3)
      else     PUT2("%.1f %d ghl ",xg[i], y)
    }
  }

  if (n>1)                                /* beam */
    if (bagpipe)
      PUT4("%.1f %.1f %.1f %.1f gbm3 ", px[0],py[0],px[n-1],py[n-1])
    else
      PUT4("%.1f %.1f %.1f %.1f gbm2 ", px[0],py[0],px[n-1],py[n-1])


  bet1=0.2;                            /* slur */
  bet2=0.8;
  yy=1000;
  for (i=n-1;i>=0;i--) if (yg[i]<=yy) {yy=yg[i]; ii=i;}
  x0=xg[ii];
  y0=yg[ii]-5;
  if (ii>0) { x0=x0-4; y0=y0+1; }
  x3=x-1;
  y3=s->ymn-5;
  dy1=(x3-x0)*0.4;
  if (dy1>3) dy1=3;
  dy2=dy1;

  if (yg[ii]>s->ymn+7){
    x0=xg[ii]-1;
    y0=yg[ii]-4.5;
    y3=s->ymn+1.5;
    x3=x-dx-5.5;
    dy2=(y0-y3)*0.2;
    dy1=(y0-y3)*0.8;
    bet1=0.0;
  }

  if (y3>y0+4) {
    y3=y0+4;
    x0=xg[ii]+2;
    y0=yg[ii]-4;
  }

  x1=bet1*x3+(1-bet1)*x0;
  y1=bet1*y3+(1-bet1)*y0-dy1;
  x2=bet2*x3+(1-bet2)*x0;
  y2=bet2*y3+(1-bet2)*y0-dy2;

  PUT4(" %.1f %.1f %.1f %.1f", x1,y1,x2,y2);
  PUT4(" %.1f %.1f %.1f %.1f gsl\n", x3,y3,x0,y0);

}

/* ----- draw_basic_note: draw m-th head with accidentals and dots -- */
void draw_basic_note (float x, float w, float d, struct SYMBOL *s, int m)
{
  int y,i,yy;
  float dotx,doty,xx,dx,avail,add,fac;

  y=3*(s->pits[m]-18)+s->yadd;                    /* height on staff */

  xx=x+s->shhd[m];                              /* draw head */
  PUT2("%.1f %d", xx, y)
  if (s->head==H_OVAL)  PUT0(" HD")
  if (s->head==H_EMPTY) PUT0(" Hd")
  if (s->head==H_FULL)  PUT0(" hd")
  if (s->shhd[m]) {
    yy=0;
    if (y>=30) { yy=y; if (yy%6) yy=yy-3; }
    if (y<=-6) { yy=y; if (yy%6) yy=yy+3; }
    if (yy) PUT1(" %d hl", yy)
  }

  if (s->dots) {                                /* add dots */
    if (y%6) { dotx=8; doty=0; }
    else     { dotx=8; doty=3; }
    if (s->stem==-1)
      dotx=dotx+s->xmx-s->shhd[m];
    else
      dotx=dotx+s->xmx-s->shhd[m];
    if (s->dots && s->flags && (s->stem==1) && !(y%6))
      if ((s->word_st==1) && (s->word_end==1) && (s->npitch==1))
        dotx=dotx+DOTSHIFT;
    if (s->head==H_EMPTY) dotx=dotx+1;
    if (s->head==H_OVAL)  dotx=dotx+2;
    for (i=0;i<s->dots;i++) {
      PUT2(" %.1f %.1f dt", dotx, doty)
      dotx=dotx+3.5;
    }
  }

  if (s->accs[m]) {                          /* add accidentals */
    fac=1.0;
    avail=d-w-3;
    add=0.3*avail;
    fac=1+add/s->wl;
    if (fac<1) fac=1;
    if (fac>1.2) fac=1.2;
    dx=fac*s->shac[m];
    if (s->accs[m]==A_SH) PUT1(" %.1f sh", dx)
    if (s->accs[m]==A_NT) PUT1(" %.1f nt", dx)
    if (s->accs[m]==A_FT) PUT1(" %.1f ft", dx)
    if (s->accs[m]==A_DS) PUT1(" %.1f dsh", dx)
    if (s->accs[m]==A_DF) PUT1(" %.1f dft", dx)
  }
}


/* ----- draw_decorations ----- */
float draw_decorations (float x, struct SYMBOL *s, float *tp)
{
  int y,sig,k,deco,m;
  float yc,xc,y1,top,top1,dx,dy;


  top=-1000;
  for (k=s->dc.n-1;k>=0;k--) {                 /*  decos close to head */
    deco=s->dc.t[k];

/*  if ((deco==D_STACC)||(deco==D_EMBAR)) { */      /* dot or bar mark */
    if (deco==D_STACC) {                           /* dot */
      sig=1; if (s->stem==1) sig=-1;
      y=s->y+6*sig;
      if (y<top+3) y=top+3;
      if (!(y%6) && (y>=0) && (y<=24)) y+=3*sig;
      if (top<y) top=y;
      if (deco==D_STACC) PUT1(" %d stc",y)
      else               PUT1(" %d emb",y)
    }

    if (deco==D_SLIDE) {                           /* slide */
      yc=s->ymn;
      xc=5;
      for (m=0;m<s->npitch;m++) {
        dx=5-s->shhd[m];
        if (s->head==H_OVAL) dx=dx+2.5;
        if (s->accs[m]) dx=4-s->shhd[m]+s->shac[m];
        dy=3*(s->pits[m]-18)+s->yadd-yc;
        if ((dy<10) && (dx>xc)) xc=dx;
      }
      yc=s->ymn;
      PUT2(" %.1f %.1f sld", yc, xc)
    }
  }

  top1=top;
  for (k=s->dc.n-1;k>=0;k--) {                 /*  decos further away */
    deco=s->dc.t[k];

    if (deco==D_EMBAR) {                         /* bar */
      yc=s->ymx+6;
      if (s->stem==1) yc=s->ys+4;
      if (yc<28) yc=28;
      if (yc<top+3) yc=top+3;
      if (top<yc+2) top=yc+2;
      PUT1(" %.2f emb", yc)
    }

    if ((deco==D_GRACE)||(deco==D_HAT)||(deco==D_ATT)) { /* gracing,hat,att */
      yc=s->ymx+9;
      if (s->stem==1) yc=s->ys+5;
      if (yc<30) yc=30;
      if (yc<top+4) yc=top+4;
      if (top<yc+2) top=yc+2;
      if      (deco==D_GRACE) PUT1(" %.2f grm", yc)
      else if (deco==D_HAT)   PUT1(" %.2f hat", yc)
      else                    PUT1(" %.2f att", yc)
    }

    if (deco==D_ROLL) {                            /* roll sign */
      y=s->y;
      if (s->stem==1) {
        yc=s->y-5;
        if (yc>-2) yc=-2;
        PUT1(" %.2f cpd", yc)
      }
      else {
        yc=s->y+5;
        if (s->dots && (!(y%6))) yc=s->y+6;
        if (yc<26) yc=26;
        if (yc<top+1) yc=top+1;
        if (top<yc+8) top=yc+8;
        PUT1(" %.2f cpu", yc)
      }
    }

    if (deco==D_HOLD) {                           /* hold sign */
      yc=27;
      if (s->stem==1)
        y1=s->ys+4;
      else
        y1=s->ymx+6;
      if (yc<y1) yc=y1;
      if (yc<top+4) yc=top+4;
      if (top<yc+12) top=yc+12;
      PUT1(" %.1f hld", yc)
    }

    if (deco==D_TRILL) {                          /* trill sign */
      yc=30;
      if (s->stem==1)
        y1=s->ys+5;
      else
        y1=s->ymx+7;
      if (yc<y1) yc=y1;
      if (yc<top+1) yc=top+1;
      if (top<yc+8) top=yc+8;
      PUT1(" %.1f trl", yc)
    }

    if ((deco==D_UPBOW)||(deco==D_DOWNBOW)) {     /* bowing signs */
      yc=21;
      if (s->stem==1)
        y1=s->ys+4;
      else
        y1=s->ymx+8;
      if (yc<y1) yc=y1;
      if (yc<top+4) yc=top+4;
      if (top<yc+10) top=yc+10;
      if (deco==D_UPBOW)   PUT1(" %.1f upb", yc)
      if (deco==D_DOWNBOW) PUT1(" %.1f dnb", yc)
    }
  }
  *tp=top;
  return top1;
}


/* ----- draw_note ----- */
float draw_note (float x, float w, float d, struct SYMBOL *s, int fl,
                 float *gchy)
{
  char c,cc;
  int y,i,m,k;
  float yc,slen,slen0,top,top2,xx;
  slen0=STEM;

  draw_gracenotes (x, w, d, s);                /* draw grace notes */

  c = 'd'; cc='u';
  if (s->stem==1) { c='u'; cc='d'; }
  slen=s->stem*(s->ys-s->y);

  for (m=0;m<s->npitch;m++) {
    if (m>0) PUT0(" ")
    draw_basic_note (x,w,d,s,m);             /* draw note heads */
    xx=3*(s->pits[m]-18)+s->yadd-s->y;
    xx=xx*xx;
    if (xx<0.01) {                                 /* add stem */
      if (s->stem) PUT2(" %.1f s%c",slen,c)
      if (fl && (s->flags>0))                      /* add flags */
        PUT3(" %.1f f%d%c",slen,s->flags,c)
    }
    if ((m>0) && (s->pits[m]==s->pits[m-1])) {     /* unions */
      if (s->stem) PUT2(" %.2f s%c",slen0,cc)
      if (s->flags>0)
        PUT3(" %.1f f%d%c",slen0,s->flags,cc)
    }
  }

  top=draw_decorations (x,s,&top2);                /* add decorations */

  y = s->ymn;                                      /* lower helper lines */
  if (y<=-6) {
    for (i=-6;i>=y;i=i-6) PUT1(" %d hl", i)
    if (s->head==H_OVAL) PUT0("1")
  }
  y = s->ymx;                                      /* upper helper lines */
  if (y>=30) {
    for (i=30;i<=y;i=i+6) PUT1(" %d hl", i)
    if (s->head==H_OVAL) PUT0("1")
  }

  *gchy=38;
  if (strlen(s->text)>0) {                         /* position guitar chord */
    yc=*gchy;
    if (yc<y+8) yc=y+8;
    if (yc<s->ys+4) yc=s->ys+4;
    for (k=0;k<s->dc.n;k++) {
      if ((s->dc.t[k]==D_GRACE) && (yc<y+12)) yc=y+12;
    }
    if (yc<top2) yc=top2;
    *gchy=yc;
  }

  PUT0("\n")

  return top;

}


/* ----- vsh: up/down shift needed to get k*6  ----- */
float vsh (float x, int dir)
{
  int ix,iy,ir;
  float x1,xx;
  x1=x*dir;
  ix=x1+600.999;
  ir=ix%6;
  iy=ix-600;
  if (ir>0) iy=iy+6-ir;
  xx=iy*dir;
  return xx-x;
}


/* ----- rnd3: up/down shift needed to get k*3  ----- */
float rnd3(float x)
{
  int ix,iy,ir;
  float xx;

  ix=x+600.999-1.5;
  ir=ix%3;
  iy=ix-600;
  if (ir>0) iy=iy+3-ir;
  xx=iy;
  return xx-x;
}


/* ----- rnd6: up/down shift needed to get k*6  ----- */
float rnd6(float x)
{
  int ix,iy,ir;
  float xx;

  ix=x+600.999-3.0;
  ir=ix%6;
  iy=ix-600;
  if (ir>0) iy=iy+6-ir;
  xx=iy;
  return xx-x;
}


/* ----- b_pos ----- */
float b_pos (int stem, int flags, float b)
{
  float bb,d1,d2,add;
  float top,bot;

  if (stem==1) {
    top=b;
    bot=b-(flags-1)*BEAM_SHIFT-BEAM_DEPTH;
    if (bot>26) return b;
  }
  else {
    bot=b;
    top=b+(flags-1)*BEAM_SHIFT+BEAM_DEPTH;
    if (top<-2) return b;
  }

  d1=rnd6(top-BEAM_OFFSET);
  d2=rnd6(bot+BEAM_OFFSET);
  add=d1;
  if (d1*d1>d2*d2) add=d2;
  bb=b+add;

/*  printf ("stem %d top %.1f, bot%.1f, choices %.1f %.1f => %.1f\n",
          stem, top,bot, d1,d2, add); */
/*  printf ("b_pos(%d) b=%.1f to %.1f\n", stem,b,bb); */

  return bb;
}


/* ----- calculate_beam ----- */
int calculate_beam (int i0, struct BEAM *bm)
{
  int j,j1,j2,i,stem,notes,flags;
  float x,y,ys,a,b,max_stem_err,stem_err,min_stem,slen,yyg,yg,try;
  float s,sx,sy,sxx,sxy,syy,delta,hh,dev,dev2,a0;

  j1=i0;                      /* find first and last note in beam */
  j2=-1;
  stem=sym[j1].stem;
  for (j=i0;j<nsym;j++)
    if (sym[j].word_end) {
      j2=j;
      break;
    }
  if (j2==-1) {
    return 0;
  }

  notes=flags=0;                /* set x positions, count notes and flags */
  for (j=j1;j<=j2;j++) {
    if(sym[j].type==NOTE) {
      sym[j].xs=sym[j].x+stem*STEM_XOFF;
      sym[j].stem=stem;
      if (sym[j].flags>flags) flags=sym[j].flags;
      notes++;
    }
  }

  s=sx=sy=sxx=sxy=syy=0;              /* linear fit through stem ends */
  for (j=j1;j<=j2;j++) if (sym[j].type==NOTE) {
    x=sym[j].xs;
    y=sym[j].ymx+STEM*stem;
    s += 1; sx += x; sy += y;
    sxx += x*x; sxy += x*y; syy += y*y;
  }

  delta=s*sxx-sx*sx;                  /* beam fct: y=ax+b */
  a=(s*sxy-sx*sy)/delta;
  b=(sy-a*sx)/s;

  /* the next few lines modify the slope of the beam */
  if (notes>=3) {
    hh=syy-a*sxy-b*sy;                /* flatten if notes not in line */
    dev=0;
    if (hh>0) {
      dev2=hh/(notes-2);
      if (dev2>0.5) a=BEAM_FLATFAC*a;
    }
  }


  if (a>=0) a=BEAM_SLOPE*a/(BEAM_SLOPE+a);   /* max steepness for beam */
  else      a=BEAM_SLOPE*a/(BEAM_SLOPE-a);


  /* to decide if to draw flat etc. use normalized slope a0 */
  a0=a*(sym[j2].xs-sym[j1].xs)/(20*(notes-1));

  if ((a0<BEAM_THRESH) && (a0>-BEAM_THRESH)) a=0;  /* flat below threshhold */

  b=(sy-a*sx)/s;                        /* recalculate b for new slope */

/*  if (flags>1) b=b+2*stem;*/      /* leave a bit more room if several beams */

  if (bagpipe) { b=-11; a=0; }

  max_stem_err=0;                        /* check stem lengths */
  for (j=j1;j<=j2;j++) if (sym[j].type==NOTE) {
    if (sym[j].npitch==1) {
      min_stem=STEM_MIN;
      if (sym[j].flags==2) min_stem=STEM_MIN2;
      if (sym[j].flags==3) min_stem=STEM_MIN3;
      if (sym[j].flags==4) min_stem=STEM_MIN4;
    }
    else {
      min_stem=STEM_CH_MIN;
      if (sym[j].flags==2) min_stem=STEM_CH_MIN2;
      if (sym[j].flags==3) min_stem=STEM_CH_MIN3;
      if (sym[j].flags==4) min_stem=STEM_CH_MIN4;
    }
    min_stem=min_stem+BEAM_DEPTH+BEAM_SHIFT*(sym[j].flags-1);
    ys=a*sym[j].xs+b;
    if (stem==1) slen=ys-sym[j].ymx;
    else         slen=sym[j].ymn-ys;
    stem_err=min_stem-slen;
    if (stem_err>max_stem_err) max_stem_err=stem_err;
  }

  if (max_stem_err>0)                   /* shift beam if stems too short */
    b=b+stem*max_stem_err;

  for (j=j1+1;j<=j2;j++) if (sym[j].type==NOTE) {  /* room for gracenotes */
    for (i=0;i<sym[j].gr.n;i++) {
      yyg=a*(sym[j].x-GSPACE0)+b;
      yg=3*(sym[j].gr.p[i]-18)+sym[j].yadd;
      if (stem==1) {
        try=(yg+GSTEM)-(yyg-BEAM_DEPTH-2);
        if (try>0) b=b+try;
      }
      else {
        try=(yg)-(yyg+BEAM_DEPTH+7);
        if (try<0) b=b+try;
      }
    }
  }

  if ((a<0.01) && (a>-0.01))       /* shift flat beams onto staff lines */
    b=b_pos (stem,flags,b);

  for (j=j1;j<=j2;j++) if (sym[j].type==NOTE) {    /* final stems */
    sym[j].ys=a*sym[j].xs+b;
  }

  bm->i1=j1;                      /* save beam parameters in struct */
  bm->i2=j2;
  bm->a=a;
  bm->b=b;
  bm->stem=stem;
  bm->t=stem*BEAM_DEPTH;
  return 1;
}


/* ----- rest_under_beam ----- */
float rest_under_beam (float x, int head, struct BEAM *bm)
{
  float y,tspace,bspace;
  int j1,j2,j,nf,iy;

  tspace=9;
  bspace=11;
  if ((head==H_OVAL)||(head==H_EMPTY)) tspace=bspace=4;

  j1=bm->i1;
  j2=bm->i2;
  nf=0;
  for (j=j1;j<=j2;j++)
    if ((sym[j].type==NOTE)||(sym[j].flags>nf)) nf=sym[j].flags;

  if (bm->stem==1) {
    y=bm->a*x+bm->b;
    y=y-BEAM_DEPTH-(nf-1)*BEAM_SHIFT;
    y=y-tspace;
    if (y>12) y=12;
  }
  else {
    y=bm->a*x+bm->b;
    y=y+BEAM_DEPTH+(nf-1)*BEAM_SHIFT;
    y=y+bspace;
    if (y<12) y=12;
  }

  if ((head==H_OVAL)||(head==H_EMPTY)) {
    iy=(y+3.0)/6.0;
    y=6*iy;
  }

  return y;
}

/* ----- draw_beam_num: draw number on a beam ----- */
void draw_beam_num (struct BEAM *bm, int num, float xn)
{
  float yn;

  if (bm->stem==-1)
    yn=bm->a*xn+bm->b-12;
  else
    yn=bm->a*xn+bm->b+4;

  PUT3("%.1f %.1f (%d) bnum\n", xn, yn, num)

}


/* ----- draw_beam: draw a single beam ----- */
void draw_beam (float x1, float x2, float dy, struct BEAM *bm)
{
  float y1,y2;

  y1=bm->a*x1+bm->b-bm->stem*dy;
  y2=bm->a*x2+bm->b-bm->stem*dy;
  PUT5("%.1f %.1f %.1f %.1f %.1f bm\n", x1,y1,x2,y2,bm->t)
}

/* ----- draw_beams: draw the beams for one word ----- */
void draw_beams (bm)
struct BEAM *bm;
{
  int j,j1,j2,j3,inbeam,k1,k2,num,p,r;
  float x1,x2,xn;

  j1=bm->i1;
  j2=bm->i2;

  /* make first beam over whole word */
  x1=sym[j1].xs;
  x2=sym[j2].xs;
  num=sym[j1].u;

  for (j=j1;j<=j2;j++) {    /* numbers for nplets on same beam */
    if (sym[j].p_plet>0) {
      p=sym[j].p_plet;
      r=sym[j].r_plet;
      j3=j+r-1;
      if (j3<=j2) {
        xn=0.5*(sym[j].xs+sym[j3].xs);
        draw_beam_num (bm,p,xn);
        sym[j].p_plet=0;
      }
    }
  }

  draw_beam (x1,x2,0.0,bm);

  /* second beams where two or more flags */
  k1=0;
  inbeam=0;
  for (j=j1;j<=j2;j++) {
    if (sym[j].type!=NOTE) continue;
    if ((!inbeam) && (sym[j].flags>=2)) {
      k1=j;
      inbeam=1;
    }
    if (inbeam && ((sym[j].flags<2) || (j==j2))) {
      if ((sym[j].flags>=2) && (j==j2)) k2=j;
      x1=sym[k1].xs;
      x2=sym[k2].xs;
      inbeam=0;
      if (k1==k2) {
        if (k1==j1) draw_beam (x1+BEAM_STUB,x1,BEAM_SHIFT,bm);
        else        draw_beam (x1-BEAM_STUB,x1,BEAM_SHIFT,bm);
      }
      else
        draw_beam (x1,x2,BEAM_SHIFT,bm);
      inbeam=0;
    }
    k2=j;
  }

  /* third beams where three or more flags */
  k1=0;
  inbeam=0;
  for (j=j1;j<=j2;j++) {
    if (sym[j].type!=NOTE) continue;
    if ((!inbeam) && (sym[j].flags>=3)) {
      k1=j;
      inbeam=1;
    }
    if (inbeam && ((sym[j].flags<3) || (j==j2))) {
      if ((sym[j].flags>=3) && (j==j2)) k2=j;
      x1=sym[k1].xs;
      x2=sym[k2].xs;
      inbeam=0;
      if (k1==k2) {
        if (k1==j1) draw_beam (x1+BEAM_STUB,x1,2*BEAM_SHIFT,bm);
        else        draw_beam (x1-BEAM_STUB,x1,2*BEAM_SHIFT,bm);
      }
      else
        draw_beam (x1,x2,2*BEAM_SHIFT,bm);
      inbeam=0;
    }
    k2=j;
  }

  /* fourth beams where four or more flags */
  k1=0;
  inbeam=0;
  for (j=j1;j<=j2;j++) {
    if (sym[j].type!=NOTE) continue;
    if ((!inbeam) && (sym[j].flags>=4)) {
      k1=j;
      inbeam=1;
    }
    if (inbeam && ((sym[j].flags<4) || (j==j2))) {
      if ((sym[j].flags>=4) && (j==j2)) k2=j;
      x1=sym[k1].xs;
      x2=sym[k2].xs;
      inbeam=0;
      if (k1==k2) {
        if (k1==j1) draw_beam (x1+BEAM_STUB,x1,3*BEAM_SHIFT,bm);
        else        draw_beam (x1-BEAM_STUB,x1,3*BEAM_SHIFT,bm);
      }
      else
        draw_beam (x1,x2,3*BEAM_SHIFT,bm);
      inbeam=0;
    }
    k2=j;
  }

}

/* ----- extreme: return min or max, depending on s ----- */
float extreme (float s, float a, float b)
{

  if (s>0) {
    if (a>b) return a;
    return b;
  }
  else {
    if (a<b) return a;
    return b;
  }
}

/* ----- draw_bracket  ----- */
void draw_bracket (int p, int j1, int j2)
{
  float x1,x2,y1,y2,xm,ym,s,s0,xx,yy,yx,dy;
  int j;

  x1=sym[j1].x-4;
  x2=sym[j2].x+4;
  y1=sym[j1].ymx+10;
  y2=sym[j2].ymx+10;

  if (sym[j1].stem==1) { y1=sym[j1].ys+4; x1=x1+3; }
  if (sym[j2].stem==1) { y2=sym[j2].ys+4; x2=x2+3; }

  if (y1<30) y1=30;
  if (y2<30) y2=30;

  xm=0.5*(x1+x2);
  ym=0.5*(y1+y2);

  s=(y2-y1)/(x2-x1);
  s0=(sym[j2].ymx-sym[j1].ymx)/(x2-x1);
  if (s0>0) {
    if (s<0) s=0; if (s>s0) s=s0;
  }
  else {
    if (s>0) s=0; if (s<s0) s=s0;
  }
  if (s*s < 0.2*0.2) s=0;   /* flat if below limit */


  dy=0;           /* shift up bracket if needed */
  for (j=j1;j<=j2;j++) {
    if ((sym[j].type==NOTE) || (sym[j].type==REST)) {
      xx=sym[j].x;
      yy=ym+(xx-xm)*s;
      yx=sym[j].ymx+10;
      if (sym[j].stem==1) yx=sym[j].ys+5;
      if (yx-yy>dy) dy=yx-yy;
    }
  }
  ym=ym+dy;
  y1=ym+s*(x1-xm);
  y2=ym+s*(x2-xm);

  /* shift up guitar chords, if needed */
  for (j=j1;j<=j2;j++) {
    if ((sym[j].type==NOTE) || (sym[j].type==REST)) {
      xx=sym[j].x;
      yy=ym+(xx-xm)*s;
      if (sym[j].gchy<yy+4) sym[j].gchy=yy+4;
    }
  }

  xx=xm-6;
  yy=ym+s*(xx-xm);
  PUT4("%.1f %.1f %.1f %.1f hbr ",  x1,y1,xx,yy)

  xx=xm+6;
  yy=ym+s*(xx-xm);
  PUT4("%.1f %.1f %.1f %.1f hbr ",  x2,y2,xx,yy)

  yy=0.5*(y1+y2);
  PUT3("%.1f %.1f (%d) bnum\n", xm, yy-4, p)

}

/* ----- draw_nplet_brackets  ----- */
void draw_nplet_brackets ()
{
  int i,j,k,p,r,c;

  for (i=0;i<nsym;i++) {
    if ((sym[i].type==NOTE) || (sym[i].type==REST)) {
      if (sym[i].p_plet>0) {
        p=sym[i].p_plet;
        r=sym[i].r_plet;
        c=r;
        k=i;
        for (j=i;j<nsym;j++) {
          if ((sym[j].type==NOTE) || (sym[j].type==REST)) {
            c--;
            k=j;
            if (c==0) break;
          }
        }
        draw_bracket (p,i,k);
      }
    }
  }
}


/* ----- slur_direction: decide whether slur goes up or down --- */
float slur_direction (k1,k2)
int k1,k2;
{
  float s;
  int i,are_stems,are_downstems,are_bars,y_max,notes;

  are_stems=are_downstems=are_bars=0;
  notes=0;
  y_max=300;
  for (i=k1;i<=k2;i++) {
    if (sym[i].type==BAR) are_bars=1;
    if (sym[i].type==NOTE) {
      notes++;
      if (sym[i].stem != 0 )  are_stems=1;
      if (sym[i].stem == -1 ) are_downstems=1;
      if (sym[i].ymn<y_max) y_max=sym[i].ymn;
    }
  }
  s=-1;
  if (are_downstems) s=1;
  if (!are_stems) {
    s=1;
    if (y_max<12) s=-1;
  }

  /* next line tries to put long phrasings on top */
  if (are_bars && (notes>4)) s=1;

  return s;
}

/* ----- output_slur: output slur -- --- */
void output_slur (float x1, float y1, float x2, float y2, float s,
                  float height, float shift)
{
  float alfa,beta,mx,my,xx1,yy1,xx2,yy2,dx,dy,dz,a,add;

  alfa=0.3;
  beta=0.45;

  /* for wide flat slurs, make shape more square */
  dy=y2-y1;
  if (dy<0) dy=-dy;
  dx=x2-x1;
  if (dx<0) dx=-dx;
  a=dy/dx;
  if ((a<0.7) && dx>40) {
    add=0.2*(dx-40)/100;
    alfa=0.3+add;
    if (alfa>0.7) alfa=0.7;
  }


  /* alfa, beta, and height determine Bezier control points pp1,pp2
   *
   *           X====alfa===|===alfa=====X
   *          /            |             \
   *        pp1            |              pp2
   *        /            height            \
   *      beta             |                beta
   *      /                |                 \
   *    p1                 m                  p2
   *
   */


  mx=0.5*(x1+x2);
  my=0.5*(y1+y2);

  xx1=mx+alfa*(x1-mx);
  yy1=my+alfa*(y1-my)+height;
  xx1=x1+beta*(xx1-x1);
  yy1=y1+beta*(yy1-y1);

  xx2=mx+alfa*(x2-mx);
  yy2=my+alfa*(y2-my)+height;
  xx2=x2+beta*(xx2-x2);
  yy2=y2+beta*(yy2-y2);

  dx=0.03*(x2-x1);
  if (dx>10.0) dx=10.0;
  dy=1.0;
  dz=0.20;
  if (x2-x1>100) dz=dz+0.001*(x2-x1);
  if (dz>0.6) dz=0.6;

  PUT4("%.1f %.1f %.1f %.1f ",
       xx2-dx, yy2+shift+s*dy, xx1+dx, yy1+shift+s*dy)
  PUT3("%.1f %.1f 0 %.1f ", x1,y1+shift+s*dz,s*dz)
  PUT4("%.1f %.1f %.1f %.1f ", xx1,yy1+shift,xx2,yy2+shift)
  PUT4("%.1f %.1f %.1f %.1f SL\n", x2,y2+shift, x1,y1+shift)


/*PUT4("%.2f %.2f %.2f %.2f ", xx1,yy1+shift,xx2,yy2+shift)
  PUT4("%.2f %.2f %.2f %.2f sl\n", x2,y2+shift, x1,y1+shift)*/

  return;
}

/* ----- draw_slur (not a pretty routine, this) ----- */
void draw_slur (int k1, int k2, int nn, int level)
{
  float x01,x02,y01,y02;
  float x1,y1,x2,y2,yy,height,addx,addy;
  float s,shift,hmin,a;
  float x,y,z,h,dx,dy;
  int i;

  s=slur_direction (k1,k2);

  /* fix endpoints */
  if (sym[k1].type==NOTE) {            /* here if k1 points to note */
    x01=sym[k1].x;
    yy=sym[k1].ymn; if (s>0) yy=sym[k1].ymx;
    y01=extreme(s,yy+s*6,sym[k1].ys+s*2);
    if (sym[k1].word_end) {
      yy=sym[k1].ymn; if (s>0) yy=sym[k1].ymx;
      y01=yy+s*6;
      if ((sym[k1].stem==1)&&(s==1)) x01=x01+4;
    }
    if ((s>0) && (y01<sym[k1].dc.top+2.5)) y01=sym[k1].dc.top+2.5;
  }

  if (sym[k2].type==NOTE) {            /* here if k2 points to note */
    x02=sym[k2].x;
    yy=sym[k2].ymn; if (s>0) yy=sym[k2].ymx;
    y02=extreme(s,yy+s*6,sym[k2].ys+s*2);
    if (sym[k2].word_st) {
      yy=sym[k2].ymn; if (s>0) yy=sym[k2].ymx;
      y02=yy+s*6;
      if ((sym[k2].stem==-1)&&(s==-1)) x02=x02-3;
    }
    if ((s>0) && (y02<sym[k2].dc.top+2.5)) y02=sym[k2].dc.top+2.5;
  }

  if (sym[k1].type!=NOTE) {
    x01=sym[k1].x+sym[k1].wr;
    y01=y02+1.2*s;
    if (nn>1) {
      if(s==1) { if (y01<28) y01=28; }
      else     { if (y01>-4) y01=-4; }
    }
  }

  if (sym[k2].type!=NOTE) {
    x02=sym[k2].x;
    y02=y01+1.2*s;
    if (nn>1) {
      if (s==1) {if (y02<28) y02=28; }
      else      {if (y02>-4) y02=-4; }
    }
  }

  /* shift endpoints */
  addx=0.04*(x02-x01);
  if (addx>3.0) addx=3.0;
  addy=0.02*(x02-x01);
  if (addy>3.0) addy=3.0;
  x1 = x01+addx;
  x2 = x02-addx;
  y1=y01+s*addy;
  y2=y02+s*addy;

  a=(y2-y1)/(x2-x1);                    /* slur steepness */
  if (a >  SLUR_SLOPE) a= SLUR_SLOPE;
  if (a < -SLUR_SLOPE) a=-SLUR_SLOPE;
  if (a>0) {
    if (s ==  1) y1=y2-a*(x2-x1);
    if (s == -1) y2=y1+a*(x2-x1);
  }
  else {
    if (s == -1) y1=y2-a*(x2-x1);
    if (s ==  1) y2=y1+a*(x2-x1);
  }

  /* for big vertical jump, shift endpoints */
  y=y2-y1; if (y>8) y=8; if (y<-8) y=-8;
  z=y; if(z<0) z=-z; dx=0.5*z; dy=0.3*z;
  if (y>0) {
    if (s==1) { x2=x2-dx; y2=y2-dy; }
    if (s==-1) { x1=x1+dx; y1=y1+dy; }
  }
  else {
    if (s==1) { x1=x1+dx; y1=y1-dy; }
    if (s==-1) { x2=x2-dx; y2=y2+dy; }
  }

  h=0;
  for (i=k1+1; i<k2; i++)
    if (sym[i].type==NOTE) {
      x = sym[i].x;
      yy = sym[i].ymn; if (s>0) yy=sym[i].ymx;
      y = extreme (s, yy+6*s, sym[i].ys+2*s);
      z = (y2*(x-x1)+y1*(x2-x))/(x2-x1);
      h = extreme (s, h, y-z);
    }

  y1=y1+0.4*h;
  y2=y2+0.4*h;
  h=0.6*h;

  hmin=s*(0.03*(x2-x1)+8);
  if (nn>3) hmin=s*(0.12*(x2-x1)+12);
  height = extreme (s, hmin, 3.0*h);
  height = extreme (-s, height, s*50);

  y=y2-y1; if (y<0) y=-y;
  if ((s==1)  && (height< 0.8*y)) height=0.8*y;
  if ((s==-1) && (height>-0.8*y)) height=-0.8*y;

  shift=3*s*level;

  output_slur (x1,y1,x2,y2,s,height,shift);

  return;
}


/* ----- prev_scut, next_scut: find place to terminate/start slur --- */
int next_scut (int i)
{
  int j,cut,ok;

  cut=nsym-1;
  for (j=i+1;j<nsym;j++) {
    ok=0;
    if (sym[j].type==BAR) {
      if (sym[j].u==B_RREP) ok=1;
      if (sym[j].u==B_DREP) ok=1;
      if (sym[j].u==B_FAT1) ok=1;
      if (sym[j].u==B_FAT2) ok=1;
      if (sym[j].v==2)      ok=1;
    }
    if(ok) {
      cut=j;
      break;
    }
  }
  return cut;
}

int prev_scut(i)
int i;
{
  int j,cut,ok;

  cut=-1;
  for (j=i;j>=0;j--) {
    ok=0;
    if (sym[j].type==BAR) {
      if (sym[j].u==B_LREP) ok=1;
      if (sym[j].u==B_DREP) ok=1;
      if (sym[j].u==B_FAT1) ok=1;
      if (sym[j].u==B_FAT2) ok=1;
      if (sym[j].v==2)      ok=1;
    }
    if(ok) {
      cut=j;
      break;
    }
  }

  if (cut==-1) {    /* return sym before first note */
    cut=0;
    for (j=0;j<nsym;j++) {
      if((sym[j].type==REST) || (sym[j].type==NOTE)) {
        cut=j-1;
        break;
      }
    }
  }

  return cut;
}


/* ----- draw_chord_slurs ----- */
void draw_chord_slurs(int k1, int k2, int nh1, int nh2, int nslur,
                      int mhead1[], int mhead2[], int job)
{

  int i,pbot,ptop,m1,m2,p1,p2,y,cut;
  float s,x1,y1,x2,y2,height,shift,addx,addy;

  if (nslur==0) return;

  pbot=1000;
  ptop=-1000;
  for (i=0;i<sym[k1].npitch;i++) {
    p1=sym[k1].pits[i];
    if (p1<pbot) pbot=p1;
    if (p1>ptop) ptop=p1;
  }

  for (i=0;i<nslur;i++) {
    m1=mhead1[i];
    m2=mhead2[i];
    p1=sym[k1].pits[m1];
    p2=sym[k2].pits[m2];
    s=slur_direction (k1,k2);
    if (p1==pbot) s=-1;
    if (p1==ptop) s=1;

    x1=sym[k1].x;
    x2=sym[k2].x;
    if (job==2) {
      cut=next_scut(k1);
      x2=sym[cut].x;
      if (cut==k1) x2=x1+30;
    }

    if (job==1) {
      cut=prev_scut(k1);
      x1=sym[cut].x;
      if (cut==k1) x2=x1-30;
    }

    addx=0.04*(x2-x1);
    if (addx>3.0) addx=3.0;
    addy=0.02*(x2-x1);
    if (addy>3.0) addy=3.0;

    x1=x1+3+addx;
    x2=x2-3-addx;
    if ((s==1)  && (sym[k1].stem==1))  x1=x1+1.5;
    if ((s==-1) && (sym[k2].stem==-1)) x2=x2-1.5;

    y=3*(p1-18)+sym[k1].yadd;
    y1=y2=y+s*(4+addy);
    y=3*(p2-18)+sym[k2].yadd;
    y2=y+s*(4+addy);

    if ((s==1) && !(y%6) && (sym[k1].dots>0)) {
      y2=y1=y+s*(5.5+addy);
      x1=x1-2;
      x2=x2+2;
    }
    height=s*(0.04*(x2-x1)+5);
    shift=0;
    output_slur (x1,y1,x2,y2,s,height,shift);
  }

}



/* ----- draw_slurs: draw slurs/ties between neighboring notes/chords */
void draw_slurs (k1,k2,job)
int k1,k2,job;
{
  int i,m1,m2;
  int mhead1[MAXHD],mhead2[MAXHD],nslur,nh1,nh2;

  if (nbuf+100>BUFFSZ)
    rx ("PS output exceeds reserved space per staff",
        " -- increase BUFFSZ1");

  nslur=0;

  if (job==2) {                    /* half slurs from last note in line */
    nh1=sym[k1].npitch;
    for (i=1;i<=nh1;i++) {
      for (m1=0;m1<nh1;m1++) {
        if (sym[k1].sl1[m1]==i) {
          nslur=nslur+1;
          mhead1[nslur-1]=m1;
        }
        if (sym[k1].ti1[m1]) {
          nslur=nslur+1;
          mhead1[nslur-1]=m1;
        }
      }
    }
    draw_chord_slurs(k1,k1,nh1,nh1,nslur,mhead1,mhead1,job);
    return;
  }

  if (job==1) {                    /* half slurs to first note in line */
    nh1=sym[k1].npitch;
    for (i=1;i<=nh1;i++) {
      for (m1=0;m1<nh1;m1++) {
        if (sym[k1].sl2[m1]==i) {
          nslur=nslur+1;
          mhead1[nslur-1]=m1;
        }
        if (sym[k1].ti2[m1]) {
          nslur=nslur+1;
          mhead1[nslur-1]=m1;
        }
      }
    }
    draw_chord_slurs(k1,k1,nh1,nh1,nslur,mhead1,mhead1,job);
    return;
  }

  /* real 2-note case: set up list of slurs/ties to draw */
  if ((sym[k1].type==NOTE) && (sym[k2].type==NOTE)) {
    nh1=sym[k1].npitch;
    nh2=sym[k2].npitch;

    for (m1=0;m1<nh1;m1++) {
      if (sym[k1].ti1[m1]) {
        for (m2=0;m2<nh2;m2++) {
          if (sym[k2].pits[m2]==sym[k1].pits[m1]) {
            nslur++;
            mhead1[nslur-1]=m1;
            mhead2[nslur-1]=m2;
            break;
          }
        }
      }
    }

    for (i=1;i<=nh1;i++) {
      for (m1=0;m1<nh1;m1++) {
        if (sym[k1].sl1[m1]==i) {
          nslur++;
          mhead1[nslur-1]=m1;
          mhead2[nslur-1]=-1;
          for (m2=0;m2<nh2;m2++) {
            if (sym[k2].sl2[m2]==i) mhead2[nslur-1]=m2;
          }
          if (mhead2[nslur-1]==-1) nslur--;
        }
      }
    }
  }

  draw_chord_slurs(k1,k2,nh1,nh2,nslur,mhead1,mhead2,job);
}



/* ----- draw_phrasing: draw phrasing slur between two symbols --- */
void draw_phrasing (k1,k2,level)
int k1,k2,level;
{
  int nn,i;

  if (k1==k2) return;
  if (nbuf+100>BUFFSZ)
    rx ("PS output exceeds reserved space per staff",
        " -- increase BUFFSZ1");
  nn=0;
  for (i=k1;i<=k2;i++)
    if ((sym[i].type==NOTE)||(sym[i].type==REST)) nn++;

  draw_slur (k1,k2,nn,level);

}

/* ----- draw_all_slurs: draw all slurs/ties between neighboring notes  */
void draw_all_slurs ()
{
  int i,i1,i2;

  i1=-1;
  for (i=0;i<nsym;i++) {
    if (sym[i].type==NOTE) {
      i1=i;
      break;
    }
  }
  if (i1<0) return;
  draw_slurs(i1,i1,1);

  for (;;) {
    i2=-1;
    for (i=i1+1;i<nsym;i++) {
      if (sym[i].type==NOTE) {
        i2=i;
        break;
      }
    }
    if (i2<0) break;
    draw_slurs(i1,i2,0);
    i1=i2;
  }

  draw_slurs(i1,i1,2);

}

/* ----- draw_all_phrasings: draw all phrasing slurs for one staff ----- */
void draw_all_phrasings ()
{
  int i,j,k,cut,pass,num;

  for (pass=0;;pass++) {
    num=0;
    for (i=0;i<nsym;i++) {

      if (sym[i].slur_st) {
        k=-1;                       /* find matching slur end */
        for (j=i+1;j<nsym;j++) {
          if (sym[j].slur_st && (!sym[j].slur_end)) break;
          if (sym[j].slur_end) {
            k=j;
            break;
          }
        }
        if (k>=0) {
          cut=next_scut(i);
          if (cut<k) {
            draw_phrasing (i,cut,pass);
            cut=prev_scut(k);
            draw_phrasing (cut,k,pass);
          }
          else {
            draw_phrasing (i,k,pass);
          }
          num++;
          sym[i].slur_st--;
          sym[k].slur_end--;
        }
      }
    }
    if (num==0) break;
  }

  /* do unbalanced slurs still left over */

  for (i=0;i<nsym;i++) {
    if (sym[i].slur_end) {
      cut=prev_scut(i);
      draw_phrasing (cut,i,0);
    }
    if (sym[i].slur_st) {
      cut=next_scut(i);
      draw_phrasing (i,cut,0);
    }
  }

}

/* ----- check_bars1 ---------- */
void check_bars1 (ip1,ip2)
int ip1,ip2;
{
  int v,i,j,k1,k2;
  float dx;

  /* check for inelegant bar combinations within one line */
  i=j=ip1;
  for (;;) {
    if (xp[i].type==BAR && xp[j].type==BAR && i!=j) {
      dx=0;
      for (v=0;v<nvoice;v++) {
        k1=xp[j].p[v];
        k2=xp[i].p[v];
        if (k1>=0 && k2>=0) {
          if (symv[v][k1].u==B_RREP && symv[v][k2].u==B_LREP) {
            symv[v][k2].u=B_DREP;
            symv[v][k1].u=B_INVIS;
            dx=-4.0;
          }
        }
      }
      xp[i].x=xp[i].x+dx;
    }
    j=i;
    i=xp[i].next;
    if (i==ip2) break;
  }

}


/* ----- check_bars2 ---------- */
void check_bars2 (ip1,ip2)
int ip1,ip2;
{
  int i,ip,v;

  /* check whether to split up last bar over two lines */
  ip=xp[ip2].prec;
  for (v=0;v<nvoice;v++) {
    strcpy(voice[v].insert_text,"");
    voice[v].insert_bnum  = 0;
    voice[v].insert_space = 0;
    voice[v].insert_num   = 0;
    voice[v].insert_btype = 0;

    i=xp[ip].p[v];
    if (i>=0) {
      if (symv[v][i].type==BAR) {
        if (symv[v][i].u==B_LREP) {
          symv[v][i].u=B_SNGL;
          voice[v].insert_btype=B_LREP;
          voice[v].insert_num=0;
        }
        else if (symv[v][i].u==B_DREP) {
          symv[v][i].u=B_RREP;
          voice[v].insert_btype=B_LREP;
          voice[v].insert_num=0;
        }
        else if ((symv[v][i].u==B_RREP) && (symv[v][i].v!=0)) {
          voice[v].insert_btype=B_INVIS;
          voice[v].insert_num=symv[v][i].v;
          symv[v][i].v=0;
        }
        else if ((symv[v][i].u==B_SNGL) && (symv[v][i].v!=0)) {
          voice[v].insert_btype=B_INVIS;
          voice[v].insert_num=symv[v][i].v;
          symv[v][i].v=0;
        }

        /* if number or label on last bar, move to next line */
        if (symv[v][i].t || (strlen(symv[v][i].text)>0)) {
          if (symv[v][i+1].type==BAR) {
            if (symv[v][i+1].t==0) symv[v][i+1].t=symv[v][i].t;
            if (strlen(symv[v][i+1].text)==0)
              strcpy(symv[v][i+1].text,symv[v][i].text);
          }
          else {
            if (!voice[v].insert_btype) voice[v].insert_btype=B_INVIS;
            voice[v].insert_space=symv[v][i].wr;
            voice[v].insert_bnum=symv[v][i].t;
            strcpy(voice[v].insert_text,symv[v][i].text);
            strcpy(symv[v][i].text,"");
            symv[v][i].t=0;
          }
        }


      }
    }
  }


}



/* ----- draw_vocals ----- */
void draw_vocals (FILE *fp, int nwl, float botnote, float bspace,
                  float *botpos)
{
  int i,hyflag,l,j;
  float x,x0,yword,lastx,spc,vfsize,w,swfac,lskip;
  char word[81],t[81];

  if (nwl<=0) return;
  vfsize=cfmt.vocalfont.size;
  lskip=1.1*vfsize;
  set_font (fp, cfmt.vocalfont, 0);
  yword=-cfmt.vocalspace;
  swfac=1.05;
  if (strstr(cfmt.vocalfont.name,"Helvetica")) swfac=1.10;
  if (botnote-cfmt.vocalfont.size<yword)
    yword=botnote-cfmt.vocalfont.size;

  for (j=0;j<nwl;j++) {
    hyflag=0;
    lastx=-10;
    for (i=0;i<nsym;i++) {
      if (sym[i].wordp[j]) {
        strcpy(word,sym[i].wordp[j]);
        x0=sym[i].x;
        l=strlen(word);

        if (hyflag) {
          tex_str (word,t,&w);
          spc=x0-VOCPRE*vfsize*swfac*w-lastx;
          x = lastx+0.5*spc-0.5*swfac*vfsize*cwid('-');
          PUT2("%.1f %.1f whf ",x,yword)
          hyflag=0;
        }

        if ((l>1) && (word[l-1]=='^')) {
          word[l-1]='\0';
          hyflag=1;
        }

        if ((l==1) && (word[0]=='_')) {
          if (lastx<0) lastx=sym[i-1].x+sym[i-1].wr;
          PUT3("%.1f %.1f %.1f wln ", lastx+3, sym[i].x+1, yword)
        }
        else if ((l==1) && (word[0]=='^')) {
          PUT2("%.1f %.1f whf ", x0, yword)
          lastx=x0+vfsize*swfac*w;
        }
        else {
          tex_str (word,t,&w);
          if (isdigit(word[0]))
            x0=x0-3*vfsize*swfac*cwid('1');
          else
            x0=x0-VOCPRE*vfsize*swfac*w;
          if (strcmp(t," ")) PUT3("(%s) %.1f %.1f wd ", t, x0, yword)
          lastx=x0+vfsize*swfac*w;
        }
      }
    }
    if (hyflag) PUT2("%.1f %.1f whf ",lastx+5,yword)
    yword=yword-lskip;
  }
  *botpos=yword + lskip - bspace;
}

/* ----- draw_symbols: draw symbols at proper positions on staff ----- */
void draw_symbols (FILE *fp, float bspace, float *bpos, int is_top)
{
  int i,inbeam,j,nwl;
  float x,y,top,xl,d,w,gchy,botnote,botpos,spc,swfac;
  struct BEAM bm;
  char t[81];

  inbeam=do_words=0;
  botnote=0;
  nwl=0;
  for (i=0;i<nsym;i++) {                      /* draw the symbols */

    for (j=0;j<NWLINE;j++)
      if (sym[i].wordp[j]) {
        if (j+1>nwl) nwl=j+1;
      }
    if (nbuf+100>BUFFSZ)
      rx ("PS output exceeds reserved space per staff",
          " -- increase BUFFSZ1");
    x=sym[i].x;

    switch (sym[i].type) {

    case NOTE:
      xl=0; w=sym[i].wl;
      if (i>0) { xl=sym[i-1].x; w=w+sym[i-1].wr; }
      d=x-xl;

      if (sym[i].word_st && !sym[i].word_end) {
        if (calculate_beam (i,&bm)) inbeam=1;
      }
      if (inbeam) {
        top=draw_note(x,w,d,&sym[i],0,&gchy);
        if (i==bm.i2) {
          inbeam=0;
          draw_beams (&bm);
        }
      }
      else {
        top=draw_note(x,w,d,&sym[i],1,&gchy);
      }
      sym[i].gchy=gchy;
      sym[i].dc.top=top;
      if (sym[i].ymn-5<botnote) botnote=sym[i].ymn-5;
      break;

    case REST:
      y=sym[i].y;
      if (inbeam) y=rest_under_beam (sym[i].x,sym[i].head,&bm);
      draw_rest(x,y,sym[i],&gchy);
      sym[i].gchy=gchy;
      break;

    case BAR:
      if (sym[i].v) set_ending(i);
      draw_bar (x,sym[i]);
      break;

    case CLEF:
      if (sym[i].u==TREBLE) {
        if (sym[i].v) PUT1("%.1f stclef\n", x)
        else          PUT1("%.1f tclef\n", x)
      }
      else if (sym[i].u==TREBLE8) {
        if (sym[i].v) PUT1("%.1f st8clef\n", x)
        else          PUT1("%.1f t8clef\n", x)
      }
      else if (sym[i].u==BASS) {
        if (sym[i].v) PUT1("%.1f sbclef\n", x)
        else          PUT1("%.1f bclef\n", x)
      }
      else if (sym[i].u==ALTO) {
        if (sym[i].v) PUT1("%.1f scclef\n", x)
        else          PUT1("%.1f cclef\n", x)
      }
      else
        bug("unknown clef type", 0);
      voice[ivc].key.ktype=sym[i].u;
      break;

    case TIMESIG:
      draw_timesig (x,sym[i]);
      break;

    case KEYSIG:
      voice[ivc].key.sf=draw_keysig (x,sym[i]);
      break;

    case INVISIBLE:
      break;

    default:
      printf (">>> cannot draw symbol type %d\n", sym[i].type);
    }
  }

  draw_nplet_brackets ();

  if (ivc==ivc0) draw_barnums (fp);


  /* draw guitar chords */
  if (voice[ivc].do_gch) {
    set_font(fp,cfmt.gchordfont,0);
    swfac=1.0;
    if (strstr(cfmt.gchordfont.name,"Times-Roman"))    swfac=1.00;
    if (strstr(cfmt.gchordfont.name,"Times-Bold"))     swfac=1.05;
    if (strstr(cfmt.gchordfont.name,"Helvetica"))      swfac=1.10;
    if (strstr(cfmt.gchordfont.name,"Helvetica-Bold")) swfac=1.15;

    for (i=0;i<nsym;i++) {
      if ((sym[i].type==NOTE)||(sym[i].type==REST)) {
        if (strlen(sym[i].text)>0) {
          tex_str (sym[i].text,t,&w);
          w=cfmt.gchordfont.size*w;
          spc=w*GCHPRE;
          if (spc>8.0) spc=8.0;
          PUT3("%.1f %.1f (%s) gc ", sym[i].x-spc, sym[i].gchy, t)
        }
      }
    }
  }


  draw_all_slurs ();
  draw_all_phrasings ();

/*|   if (is_top) draw_endings (); |*/
  if (ivc==ivc0) draw_endings ();
  num_ending=0;

  botpos=-bspace;
  if (botnote<botpos) botpos=botnote;

  if (nwl>0) draw_vocals (fp,nwl,botnote,bspace,&botpos);

  *bpos=botpos;

}


/* ----- draw_sysbars: draw bars extending over staves----- */
void draw_sysbars (FILE *fp, int ip1, int ip2, float wid0, float h1, float dh)
{
  int i,v,ok,u,uu,p;
  float x;

  PUT2 ("gsave 0 %.2f T 1.0 %.4f scale ",h1,dh/24.0)
  i=ip1;
  for (;;) {
    if (xp[i].type==BAR) {
      p=xp[i].p[ivc0];
      u=symv[ivc0][p].u;
      if (u==B_LREP) u=B_FAT1;
      if (u==B_RREP) u=B_FAT2;
      ok=1;
      for (v=0;v<nvoice;v++) {
        if (v!=ivc0 && voice[v].draw) {
          p=xp[i].p[v];
          if (p<0) ok=0;
          else {
            uu=symv[v][p].u;
            if (uu==B_LREP) uu=B_FAT1;
            if (uu==B_RREP) uu=B_FAT2;
            if (uu!=u) ok=0;
          }
        }
      }

      if (ok) {
        x=xp[i].x+wid0;
        if (u==B_SNGL)
          PUT1("%.1f bar ", x)
        else if (u==B_DBL)
          PUT1("%.1f dbar ", x)
        else if (u==B_FAT1)
          PUT1("%.1f fbar1 ", x)
        else if (u==B_FAT2)
          PUT1("%.1f fbar2 ", x)
        else if (u==B_DREP) {
          PUT1("%.1f fbar1 ", x-1)
          PUT1("%.1f fbar2 ", x+1)
        }
      }
    }

    i=xp[i].next;
    if (i==ip2) break;
  }

  PUT0 (" 0.0 bar grestore\n")

}


/* ----- count_symbols: count number of "real" symbols ---- */
int count_symbols ()
{
  int i,c;

  c=0;
  for (i=0;i<nsym;i++) {
    switch (sym[i].type) {
    case NOTE:
    case REST:
    case BAR:
      c++;
    }
  }
  return c;

}

/* ----- select_piece: choose limits for piece to put on one staff ---- */
int select_piece (int ip1)
{
  int i,num,ltype,count_this;

  /* find next symbol marked as eol */
  i=ip1;
  for (;;) {
    if (xp[i].eoln) break;
    if (xp[i].next==XP_END) break;
    i=xp[i].next;
  }
  i=xp[i].next;
  if (cfmt.barsperstaff==0) return i;

  /* find first note or rest */
  i=ip1;
  for (;;) {
    if (xp[i].type==NOTE || xp[i].type==REST)
    if (xp[i].type==NOTE || xp[i].type==REST) break;
    i=xp[i].next;
    if (i==XP_END) return i;
  }
  i=xp[i].next;
  if (i==XP_END) return i;

  /* count bars until number is reached */
  num=0;
  ltype=0;
  for (;;) {
    count_this=0;
    if (xp[i].type==BAR) {
      count_this=1;
      if (ltype==BAR) count_this=0;
    }
    num=num+count_this;
    ltype=xp[i].type;
    i=xp[i].next;
    if (i==XP_END) return i;
    if (num==cfmt.barsperstaff) return i;
  }
  i=xp[i].next;

  printf ("Got here! i=%d\n", i);
  return i;

}

/* ----- is_topvc: check for top voice of set staved together --- */
int is_topvc (int jv)
{
  int iv,kv,ok;

  ok=0;
  for (iv=0;iv<jv;iv++) {
    if (voice[iv].staves >= jv-iv+1) {
      for (kv=iv;kv<jv;kv++)
        if (voice[kv].draw) ok=1;
    }
  }

  return 1-ok;
}


/* ----- vc_select: set flags for voice selection from -V option --- */
int vc_select ()
{
  char *s;
  int i,a,b,n;

  if (strlen(vcselstr)==0) return nvoice;

  for (i=0;i<nvoice;i++) voice[i].select=0;
  s=vcselstr;
  for (;;) {
    if (*s==0) break;
    if (!sscanf(s,"%d%n",&a,&n)) break;
    s+=n;
    b=a;
    if (*s=='-') {
      s++;
      if (*s==0) break;
      if (!sscanf(s,"%d%n",&b,&n)) break;
      s+=n;
    }
    for (i=a-1;i<b;i++)
      if (i>=0 && i<nvoice) voice[i].select=1;


    if (*s==0) {
      n=0;
      for (i=0;i<nvoice;i++) if (voice[i].select) n++;
      if (verbose>=4) {
        printf ("Selection <%s>   selected voices:",vcselstr);
        for (i=0;i<nvoice;i++) if (voice[i].select) printf(" %d",i+1);
        printf ("\n");
      }
      return n;
    }
    if (*s==',') s++;
    else break;
  }

  rx ("Bad voice selection: -V ", vcselstr);
  return 0;
}

/* ----- voice_label: label voice, or just return length if job==0 -- */
float voice_label (FILE *fp, char *label, float h, float xc, float dx0, int job)
{
  float w,wid,dy,xcc;
  char lab[81],t[81];
  int n,i;
  char *p,*q,*l[10];

  if (strlen(label)==0) return 0.0;

  strcpy(lab,label);
  n=0;
  p=lab;
  l[0]=p;
  while ((q=strstr(p,"\\\\"))) { *q='\0'; p=q+2; n++; l[n]=p; }
  n++;

  wid=0;
  for (i=0;i<n;i++) {
    tex_str(l[i],t,&w);
    w=cfmt.voicefont.size*w;
    if (w>wid) wid=w;
    if (job!=0) {
      xcc=xc;
      if (xcc+0.5*w>dx0) xcc=dx0-0.5*w;
      dy = 8.0 + 0.5*cfmt.voicefont.size*(n-1-2*i) + h;
      PUT3 (" %.2f %.2f M (%s) cshow\n",xcc,dy,t)
    }
  }
  return wid;
}



/* ----- mstave_deco: draw decorations over multiple staves ----- */
void mstave_deco (FILE *fp, int ip1, int ip2, float wid0, float hsys,
                  float htab[])
{
  int   iv,jv,nv;
  float hbot,htop,ht,x0,y0,wid,wc,wcb,w,wmin;
  float wmin1=15.0, wmin2=10.0;  /* space to staff */

  wmin=wmin2;
  if (do_indent) wmin=wmin1;

  /* draw bar lines */
  if (mvoice>1) {
    for (iv=0;iv<nvoice;iv++) {
      hbot=10000; htop=-hbot; nv=1;
      for (jv=iv;jv<iv+voice[iv].staves;jv++) {
        if (voice[jv].draw) nv++;
        if (voice[jv].draw && (htab[jv]<hbot)) hbot=htab[jv];
        if (voice[jv].draw && (htab[jv]>htop)) htop=htab[jv];
      }
      if ((hbot<htop-0.001) && (nv>1))
        draw_sysbars (fp,ip1,ip2,wid0,hsys-htop,htop-hbot+24.0);
    }
    PUT1 ("gsave 1.0 %.4f scale 0.0 bar grestore\n", (hsys+24.0)/24.0)
  }

  /* next part draws voice names (except on braces) */
  set_font (fp, cfmt.voicefont, 0);
  nv=0;
  for (iv=0;iv<nvoice;iv++) if (voice[iv].draw) nv++;

  /* get max label width, then center labels above each other */
  wid=w=0;
  for (iv=0;iv<nvoice;iv++) {
    if (voice[iv].draw) {
      if (do_indent)
        w=voice_label (fp, voice[iv].name, 0.0,0.0,0.0, 0);
      else if (nv>1)
        w=voice_label (fp, voice[iv].sname, 0.0,0.0,0.0, 0);
      if (w>wid) wid=w;
    }
  }
  wc=wcb=0.5*wid+wmin;
  if (wcb<18.0) wcb=18.0;     /* label on brace needs a bit more room */

  for (iv=0;iv<nvoice;iv++) {
    if (voice[iv].draw && (voice[iv].brace==0) ) {
      y0=hsys-htab[iv];
      if (do_indent)
        voice_label (fp, voice[iv].name,y0,-wc,-wmin,1);
      else if (nv>1)
        voice_label (fp,voice[iv].sname,y0,-wc,-wmin,1);
    }
  }

  /* braces and brackets */
  for (iv=0;iv<nvoice;iv++) {
    hbot=10000; htop=-hbot; nv=0;
    for (jv=iv;jv<iv+voice[iv].brace;jv++) {
      if (voice[jv].draw) nv++;
      if (voice[jv].draw && (htab[jv]<hbot)) hbot=htab[jv];
      if (voice[jv].draw && (htab[jv]>htop)) htop=htab[jv];
    }
    if (hbot<htop+0.001) {
      ht=htop-hbot+24.0;
      y0=hsys-htop+0.5*ht;
      x0=-8;
      ht=ht-2.0;
      if (voice[iv].brace==1) ht=ht+15.0;
      if (voice[iv].brace>2)  ht=ht-8.0;
      if ((nv>1)||(voice[iv].brace==1))
        PUT3 (" %.4f %.1f %.1f brace\n", ht/120.0, x0, y0)
      if (do_indent)
        voice_label (fp, voice[iv].name, y0-12.0,-wcb,-wmin,1);
      else if (nv>1)
        voice_label (fp, voice[iv].sname,y0-12.0,-wcb,-wmin,1);
    }

    hbot=10000; htop=-hbot; nv=0;
    for (jv=iv;jv<iv+voice[iv].bracket;jv++) {
      if (voice[jv].draw) nv++;
      if (voice[jv].draw && (htab[jv]<hbot)) hbot=htab[jv];
      if (voice[jv].draw && (htab[jv]>htop)) htop=htab[jv];
    }
    if ((hbot<htop+0.001) && ((nv>1)||(voice[iv].bracket==1)))
      PUT2 ("\n %.1f -3 %.1f bracket\n", htop-hbot+24.0+6.0, hsys-htop-3.0)

  }

}



/* ----- output_music: output for parsed symbol list ----- */
void output_music (FILE *fp)
{
  int ip1,ip2,mv,is_top,nsel,b,bnum;
  float realwidth,staffwidth,wid0,widv,lscale,lwidth,bpos;
  float spa1,spa2,hsys,htab[40],extra,indent,spax;

  /* save current meter and key, to continue after P: or T: field */
  for (ivc=0;ivc<nvoice;ivc++) {
    voice[ivc].meter1 = voice[ivc].meter;
    voice[ivc].key1   = voice[ivc].key;
  }

  if (!file_initialized && !epsf) {
    init_ps (fout,infostr,0,0.0,0.0,0.0,0.0);
    init_page (fout);
  }

  if (nvoice==0) { init_parse_params(); return; }
  if (verbose>=10) print_vsyms ();

  alfa_last=0.1; beta_last=0.0;
  lwidth=cfmt.staffwidth;
  lscale=cfmt.scale;
  check_margin (cfmt.leftmargin);

  /* dump buffer if not enough space for a staff line */
  check_buffer (fp, BUFFSZ1);

  /* initialize meter and key for voices */
  for (ivc=0;ivc<nvoice;ivc++) {
    voice[ivc].meter = voice[ivc].meter0;
    voice[ivc].key   = voice[ivc].key0;
    if (!do_meter) voice[ivc].meter.insert=0;
  }

  /* setup for horizontal positioning, decide which voices to draw */
  nsel=vc_select ();
  for (ivc=0;ivc<nvoice;ivc++)
    if (!voice[ivc].select) voice[ivc].nsym=0;

  mvoice=0;
  ivc0=-1;
  for (ivc=0;ivc<nvoice;ivc++) {
    voice[ivc].draw=0;
    if (voice[ivc].nsym>0) {
      mvoice++;
      if (ivc0<0) ivc0=ivc;
      voice[ivc].draw=1;
    }
  }
  if (mvoice==0) { init_parse_params(); return; }

  for (ivc=0;ivc<nvoice;ivc++) {
    if (voice[ivc].draw) {
      set_sym_chars  (0,voice[ivc].nsym,symv[ivc]);
      set_beams      (0,voice[ivc].nsym,symv[ivc]);
      set_stems      (0,voice[ivc].nsym,symv[ivc]);
      b=set_sym_times(0,voice[ivc].nsym,symv[ivc],voice[ivc].meter0);
      set_sym_widths (0,voice[ivc].nsym,symv[ivc],ivc);
      if (ivc==ivc0) bnum=b;
    }
  }
  barinit=bnum;

  if (mvoice==1)
    set_style_pars (cfmt.strict1);
  else
    set_style_pars (cfmt.strict2);

  set_poslist ();
  set_xpwid ();
  set_spaces ();

  /* loop over pieces to output */
  ip1=xp[XP_START].next;
  for (;;) {
    mline++;
    ip1=contract_keysigs (ip1);
    wid0=0;
    for (ivc=0;ivc<nvoice;ivc++) {
      nsym_st[ivc]=set_initsyms (ivc, &widv);
      if (widv>wid0) wid0=widv;
    }
    indent = (do_indent==1) ? cfmt.indent : 0.0;

    ip2=select_piece (ip1);
    ip2=check_overflow (ip1,ip2,lwidth/lscale-wid0-indent);
    if (verbose>5) printf ("output posits %d to %d\n",ip1,ip2-1);
    realwidth=set_glue (ip1,ip2,lwidth/lscale-wid0-indent);
    check_bars1 (ip1,ip2);
    check_bars2 (ip1,ip2);

    spa1=spa2=cfmt.staffsep;
    if (mvoice>1) {
      spa1=cfmt.systemsep;
      spa2=cfmt.sysstaffsep;
    }

    hsys=0;
    mv=0;
    for (ivc=0;ivc<nvoice;ivc++) {
      if (voice[ivc].draw) {
        mv++;
        nsym=copy_vsyms (ivc,ip1,ip2,wid0);
        PUT2("\n%% --- ln %d vc %d \n", mline,ivc+1)
        if (mv==1) bskip(lscale*(0.5*spa1+24.0));
        else       bskip(lscale*(0.5*spa2+24.0));
        PUT3("gsave %.3f %.3f scale %.2f setlinewidth\n",
             lscale, lscale, BASEWIDTH)
        if (do_indent) PUT1(" %.2f 0 T ",indent)
          staffwidth=realwidth+wid0;
        PUT1("%.2f staff\n", staffwidth)
        htab[ivc]=hsys;
        spax=spa2+voice[ivc].sep;
        if (voice[ivc].sep>1000.0) spax=voice[ivc].sep-2000.0;

        is_top=is_topvc(ivc);
        draw_symbols (fp,0.5*spa2+spax-spa2,&bpos,is_top);

        if (mv==mvoice) mstave_deco (fp,ip1,ip2,wid0,hsys,htab);

        PUT0("grestore\n")
        bskip(-lscale*bpos);
        hsys=hsys+0.5*spa2+24.0-bpos;
      }
    }

    extra=-bpos-0.5*spa2;
    if (mvoice>1) bskip(lscale*(spa1+bpos-0.5*spa1+extra));
    buffer_eob (fp);

    do_meter=do_indent=0;
    ip1=ip2;
    if (ip1==XP_END) break;
  }

  /* set things to continue parsing */
  for (ivc=0;ivc<nvoice;ivc++) {
    voice[ivc].nsym=0;
    voice[ivc].meter0 = voice[ivc].meter = voice[ivc].meter1;
    voice[ivc].key0   = voice[ivc].key   = voice[ivc].key1;
  }
  init_parse_params ();

}

/* ----- process_textblock ----- */
void process_textblock(fpin,fp,job)
FILE *fp,*fpin;
int  job;
{
  char w1[81],ln[BSIZE],ln1[BSIZE];
  float lwidth,baseskip,parskip;
  int i,ll,add_final_nl;

  baseskip = cfmt.textfont.size * cfmt.lineskipfac;
  parskip  = cfmt.textfont.size * cfmt.parskipfac;
  add_final_nl=0;
  if (job==OBEYLINES) add_final_nl=1;
  lwidth=cfmt.staffwidth;
  output_music (fp);
  buffer_eob (fp);
  set_font (fp, cfmt.textfont, 0);
  ntxt=0;
  for (i=0;i<100;i++) {
    if (feof(fpin)) rx("EOF reached scanning text block","");
    strcpy (ln, "");
    get_a_line(ln, BSIZE, fpin);
    ll=strlen(ln);
    linenum++;
    if ((verbose>=5) || (vb>=10) ) printf ("%3d  %s \n", linenum, ln);
    if ((ln[0]=='%') && (ln[1]=='%')) {
      strcpy(ln1,ln+2);
      strcpy(ln,ln1);
    }

    strcpy(w1,"");
    sscanf(ln,"%s",w1);
    if (!strcmp(w1,"endtext")) break;

    if (job!=SKIP) {
      if (isblankstr(ln)) {
        write_text_block (fp,job);
        ntxt=0;
      }
      else {
        add_to_text_block (ln,add_final_nl);
      }
    }
  }
  if (job!=SKIP) write_text_block (fp,job);
}


/* ----- process_pscomment  ----- */
void process_pscomment (fpin,fp,line)
FILE *fp,*fpin;
char line[];
{
  char w[81],fstr[81],unum1[41],unum2[41],unum3[41];
  float h1,h2,len,lwidth;
  int i,nch,job;

  lwidth=cfmt.staffwidth;
  line[0]=' ';
  line[1]=' ';
  for (i=0;i<strlen(line);i++) if (line[i]=='%') line[i]='\0';
  strcpy(w," ");
  sscanf(line,"%s%n", w, &nch);

  if (!strcmp(w,"begintext")) {
    if (epsf && !within_block) return;
    strcpy(fstr,"");
    sscanf(line, "%*s %s", fstr);
    if (isblankstr(fstr)) strcpy(fstr,"obeylines");
    if      (!strcmp(fstr,"obeylines")) job=OBEYLINES;
    else if (!strcmp(fstr,"align"))     job=ALIGN;
    else if (!strcmp(fstr,"skip"))      job=SKIP;
    else if (!strcmp(fstr,"ragged"))    job=RAGGED;
    else rx("bad argument for begintext: ",fstr);
    if (within_block && !do_this_tune) job=SKIP;
    process_textblock (fpin,fp,job);
    return;
  }

  if (!strcmp(w,"text") || !strcmp(w,"center")) {
    if (epsf && !within_block) return;
    if (within_block && !do_this_tune) return;
    output_music (fp);
    set_font (fp, cfmt.textfont, 0);
    ntxt=0;
    add_to_text_block (line+nch+1,1);
    if (!strcmp(w,"text"))
      write_text_block (fp,OBEYLINES);
    else
      write_text_block (fp,OBEYCENTER);
    buffer_eob (fp);
  }

  else if (!strcmp(w,"sep")) {
    if (within_block && !do_this_tune) return;
    output_music (fp);
    strcpy(unum1,"");
    strcpy(unum2,"");
    strcpy(unum3,"");
    sscanf(line,"%*s %s %s %s", unum1,unum2,unum3);
    g_unum(unum1,unum1,&h1);
    g_unum(unum2,unum1,&h2);
    g_unum(unum3,unum1,&len);
    if (h1*h1<0.00001) h1=0.5*CM;
    if (h2*h2<0.00001) h2=h1;
    if (len*len<0.0001) len=3.0*CM;
    bskip (h1);
    PUT2("%.1f %.1f sep0\n", lwidth/2-len/2, lwidth/2+len/2);
    bskip (h2);
    buffer_eob (fp);
  }

  else if (!strcmp(w,"vskip")) {
    if (within_block && !do_this_tune) return;
    output_music (fp);
    strcpy(unum1,"");
    sscanf(line,"%*s %s", unum1);
    g_unum(unum1,unum1,&h1);
    if (h1*h1<0.00001) h1=0.5*CM;
    bskip (h1);
    buffer_eob (fp);
  }

  else if (!strcmp(w,"newpage")) {
    if (within_block && !do_this_tune) return;
    output_music (fp);
    write_buffer (fp);
    use_buffer=0;
    write_pagebreak (fp);
  }

  else  {
    if (within_block) {
      interpret_format_line (line,&cfmt);
      ops_into_fmt (&cfmt);

    }
    else {
      interpret_format_line (line,&dfmt);
      ops_into_fmt (&dfmt);
      cfmt=dfmt;
    }
  }


}

/* ----- check_selected ----- */
void check_selected(fp,xref_str,npat,pat,sel_all,search_field)
FILE *fp;
int npat,sel_all,search_field;
char xref_str[],pat[][STRL1];
{

  if (!do_this_tune) {
    if (is_selected(xref_str,npat,pat,sel_all,search_field)) {
      do_this_tune=1;
      verbose=vb;
      clear_buffer ();
/*    set to 0 to permit staff breaks in a tune */
      use_buffer=1;
      write_tunetop(fp);
    }
  }
}

/* ----- process_line ----- */
void process_line (fp,type,xref_str,npat,pat,sel_all,search_field)
FILE *fp;
int type,npat,sel_all,search_field;
char xref_str[],pat[][STRL1];
{
  char fnm[81],finf[MAXINF];
  FILE *feps;

  if ((vb>15) || ((verbose>10)&&within_block)) {
    printf ("process_line, type %d ", type); print_linetype(type); }


  switch (type) {

  case XREF:                       /* start of new block */
    if (!epsf) write_buffer (fp);    /* flush stuff left from %% lines */
    if (within_block) printf ("\n+++ Last tune not closed properly\n");
    get_default_info ();
    within_block  = 1;
    within_tune   = 0;
    do_this_tune  = 0;
    numtitle=0;
    ntext=0;
    init_pdims();
    cfmt=dfmt;
    break;

  case TITLE:
    if (!within_block) break;
    if (within_tune) {             /* title within tune */
      if (do_this_tune ) {
        output_music (fp);
        write_inside_title (fp);
        do_meter=do_indent=1;
        barinit=1;
      }
    }
    else
      check_selected(fp,xref_str,npat,pat,sel_all,search_field);
    break;

  case TEMPO:
    if (!within_block) break;
    if (within_tune) {             /* tempo within tune */
      if (do_this_tune ) {
        output_music (fp);
        write_inside_tempo(fp);
      }
    }
    else
      check_selected(fp,xref_str,npat,pat,sel_all,search_field);
    break;

  case KEY:
    if (!within_block) break;
    if (within_tune) {
      if (do_this_tune) handle_inside_field(type);
    }
    else {                         /* end of header.. start now */
      check_selected(fp,xref_str,npat,pat,sel_all,search_field);
      if (do_this_tune) {
        tunenum++;
        if (verbose>=3)
          printf ("---- start %d (%s) ----\n", xrefnum, info.title);
        fflush (stdout);
        set_keysig (info.key,  &default_key, 1);
        halftones=get_halftones (default_key, transpose);
        set_transtab (halftones,&default_key);
        set_meter  (info.meter,&default_meter);
        set_dlen   (info.len,  &default_meter);
        check_margin (cfmt.leftmargin);
        write_heading (fp);
        nvoice=0;
        init_parse_params ();
        default_meter.insert=1;
        mline=0;
        do_indent=do_meter=1;
        barinit=1;
        writenum=0;
      }
    }
    within_tune=1;
    break;

  case METER:
    if (!within_block) break;
    if (do_this_tune && within_tune) handle_inside_field(type);
    break;

  case DLEN:
    if (!within_block) break;
    if (do_this_tune && within_tune) handle_inside_field(type);
    break;

  case PARTS:
    if (!within_block) break;
    if (do_this_tune && within_tune) {
      output_music (fp);
      write_parts(fp);
    }
    break;

  case VOICE:
    if (do_this_tune && within_block) {
      if (!within_tune)
        printf ("+++ Voice field not allowed in header: V:%s\n", lvoiceid);
      else
        ivc=switch_voice (lvoiceid);
    }
    break;

  case BLANK:                      /* end of block or file */
  case E_O_F:
    if (do_this_tune) {
      output_music (fp);
      put_words (fp);
      if (cfmt.writehistory) put_history (fp);
      if (epsf) {
        close_output_file ();
        if (choose_outname) {
          epsf_title (info.title, fnm);
          strcat (fnm,".eps");
        }
        else {
          nepsf++;
          sprintf (fnm, "%s%03d.eps", outf, nepsf);
        }
        sprintf (finf, "%s (%d)", in_file[0], xrefnum);
        if ((feps = fopen (fnm,"w")) == NULL)
            rx ("Cannot open output file ", fnm);
        init_ps (feps, finf, 1,
                 cfmt.leftmargin-5, posy+bposy-5,
                 cfmt.leftmargin+cfmt.staffwidth+5,
                 cfmt.pageheight-cfmt.topmargin);
        init_epsf (feps);
        write_buffer (feps);
        printf ("\n[%s] %s", fnm, info.title);
        close_epsf (feps);
        fclose (feps);
        in_page=0;
        init_pdims ();
      }
      else {
        buffer_eob (fp);
        write_buffer (fp);
        if ((verbose==0) && (tunenum%10==0)) printf (".");
        if (verbose==2) printf ("%s - ", info.title);
      }
      verbose=0;
    }
    info=default_info;
    if (within_block && !within_tune)
      printf ("\n+++ Header not closed in tune %d\n", xrefnum);
    within_tune=within_block=do_this_tune=0;
    break;

  }
}

/* ----- process_file ----- */
void process_file (fpin,fpout,xref_str,npat,pat,sel_all,search_field)
FILE *fpin,*fpout;
int npat,sel_all,search_field;
char xref_str[],pat[][STRL1];
{
  char line[BSIZE];
  int type;

  within_tune=within_block=do_this_tune=0;
  linenum=0;
  numtitle=0;
  reset_info (&default_info);
  info=default_info;
  verbose=0;
  if (vb>=20) db=3;
  if (vb>=25) db=5;

  type=read_line (fpin,0,line);
  for (;;) {

    if (type==PSCOMMENT) {
      process_pscomment(fpin,fpout,line);
    }
    else {
      process_line (fpout,type,xref_str,npat,pat,sel_all,search_field);
    }

    if (type==E_O_F) break;
    type=read_line (fpin,do_this_tune,line);
  }
  if (!epsf) {
    buffer_eob (fpout);
    write_buffer (fpout);
  }

}





