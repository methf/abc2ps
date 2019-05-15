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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "abc2ps.h"
#include "util.h"
#include "format.h"
#include "subs.h"

#include "parse.h"


/*  subroutines connected with parsing the input file  */

/* ----- sytax: print message for syntax errror -------- */
void syntax (msg, q)
char msg[];
char *q;
{
  int i,n,len,m1,m2,pp,qq,maxcol=65;

  if (verbose<=2) printf ("\n");
  qq=q-p0+1;
  if (qq<0) qq=0;
  printf ("+++ %s in line %d.%d \n", msg, linenum, qq);
  m1=0;
  m2=len=strlen(p0);
  n=q-p0;
  if (m2>maxcol) {
    if (n<maxcol)
      m2=maxcol;
    else {
      m1=n-10;
      m2=m1+maxcol;
      if (m2>len) m2=len;
    }
  }

  printf ("%4d ", linenum);
  pp=5;
  if (m1>0) { printf ("..."); pp+=3; }
  for (i=m1;i<m2;i++) printf ("%c", p0[i]);
  if (m2<len) printf ("...");
  printf ("\n");

  if (n>=0 && n<200) {
    for (i=0;i<n+pp-m1;i++) printf(" ");
    printf ("^\n");
  }
}

/* ----- isnote: checks char for valid note symbol ----- */
int isnote (char c)
{
  if (c == '\0') return 0;
  if (strchr("CDEFGABcdefgab^=_",c)) return 1;
  return 0;
}

/* ----- zero_sym: init global zero SYMBOL struct ----- */
void zero_sym ()
{
  int j;
  zsym.type      = 0;
  zsym.npitch    = 0;
  zsym.word_st   = 0;
  zsym.word_end  = 0;
  zsym.slur_st   = 0;
  zsym.slur_end  = 0;
  zsym.len       = 0;
  zsym.fullmes   = 0;
  zsym.yadd      = 0;
  zsym.xs        = 0;
  zsym.ys        = 0;
  zsym.stem      = 0;
  zsym.eoln      = 0;
  zsym.ylo       = 12;
  zsym.yhi       = 12;
  zsym.u         = 0;
  zsym.v         = 0;
  zsym.w         = 0;
  zsym.t         = 0;
  zsym.q         = 0;
  zsym.invis     = 0;
  zsym.wl        = 0;
  zsym.wr        = 0;
  zsym.pr        = 0;
  zsym.pl        = 0;
  zsym.p_plet    = 0;
  zsym.q_plet    = 0;
  zsym.r_plet    = 0;
  zsym.gchy      = 0;
  zsym.gr.n      = 0;
  zsym.dc.n      = 0;
  zsym.dc.top    = 0;
  zsym.p         = -1;
  strcpy (zsym.text, "");
  for (j=0;j<MAXHD;j++) zsym.sl1[j]=zsym.sl2[j]=0;

  for (j=0;j<NWLINE;j++) zsym.wordp[j] = 0;
}

/* ----- add_sym: returns index for new symbol at end of list ---- */
int add_sym (type)
int type;
{
  int k;

  k=voice[ivc].nsym;
  if (k>=maxSyms) rxi("Too many symbols; increase maxSyms, now ",maxSyms);
  voice[ivc].nsym++;
  symv[ivc][k]=zsym;
  symv[ivc][k].type = type;
  return k;

}

/* ----- insert_sym: returns index for new symbol inserted at k --- */
int insert_sym (type,k)
int type,k;
{
  int i,n;

  n=voice[ivc].nsym;
  if (n>=maxSyms) rxi("insert_sym: maxSyms exceeded: ",maxSyms);
  for (i=n;i>k;i--) symv[ivc][i]=symv[ivc][i-1];
  n++;
  symv[ivc][k]=zsym;
  symv[ivc][k].type = type;
  voice[ivc].nsym=n;
  return k;
}

/* ----- get_xref: get xref from string ----- */
int get_xref (str)
char str[];
{

  int a,ok;
  char *q;

  if (strlen(str)==0) {
    wng ("xref string is empty", "");
    return 0;
  }
  q=str;
  ok=1;
  while (*q != '\0') {
    if (!isdigit(*q)) ok=0;
    q++;
  }
  if (!ok) {
    wng ("xref string has invalid symbols: ", str);
    return 0;
  }

  sscanf (str, "%d", &a);

  return a;
}

/* ----- set_meter: interpret meter string, store in struct ---- */
void set_meter (str,meter)
char str[];
struct METERSTR *meter;
{

  int m1,m2,m1a,m1b,m1c,d,l;
  char *q;
  int meter1,meter2,dlen,mflag,lflag;
  char meter_top[31];

  l=strlen(str);
  if (l==0) { wng("Empty meter string", ""); return; }
  if (str[0]=='C') {
    if (str[1]=='|') {
      meter1=4;
      meter2=4;
      dlen=EIGHTH;
      mflag=2;
      strcpy(meter_top,"C");
    }
    else {
      meter1=4;
      meter2=4;
      dlen=EIGHTH;
      mflag=1;
      strcpy(meter_top,"C");
    }
  }
  else {
    m1=m2=m1a=m1b=m1c=0;
    strcpy (meter_top, str);
    q=strchr(meter_top,'/');
    if (!q) {
      wng("Cannot identify meter, missing /: ", str);
      m1=m2=1;
      return;
    }
    *q='\0';
    if (strchr(meter_top,'+')) {
      sscanf(str,"%d+%d+%d/", &m1a, &m1b, &m1c);
      m1=m1a+m1b+m1c;
    }
    else {
      sscanf(str,"%d %d %d/", &m1a, &m1b, &m1c);
      m1=m1a; if (m1b>m1) m1=m1b; if (m1c>m1) m1=m1c;
      if (m1>30) {            /* handle things like 78/8 */
        m1a=m1/100;
        m1c=m1-100*m1a;
        m1b=m1c/10;
        m1c=m1c-10*m1b;
        m1=m1a; if (m1b>m1) m1=m1b; if (m1c>m1) m1=m1c;
      }
    }

    q++;
    sscanf (q, "%d", &m2);
    if (m1*m2 == 0) wng("Cannot identify meter: ", str);
    d=BASE/m2;
    if (d*m2 != BASE) wng("Meter not recognized: ", str);
    meter1=m1;
    meter2=m2;
    dlen=EIGHTH;
    if (4*meter1 < 3*meter2) dlen=SIXTEENTH;
    mflag=0;
  }

  if (verbose>=4)
    printf ("Meter <%s> is %d over %d with default length 1/%d\n",
            str, meter1, meter2, BASE/dlen);

  /* handle old-style change of default length */
  lflag=0;
  if (str[l-1]=='s') { dlen=dlen*2; lflag=1; }
  if (str[l-1]=='l') { dlen=dlen/2; lflag=-1; }

  /* store parsed data in struct */
  meter->meter1 = meter1;
  meter->meter2 = meter2;
  meter->mflag  = mflag;
  meter->dlen   = dlen;
  meter->lflag  = lflag;
  strcpy(meter->top, meter_top);
}


/* ----- set_dlen: set default length for parsed notes ---- */
void set_dlen (str,meter)
char str[];
struct METERSTR *meter;
{
  int l1,l2,d,dlen;

  l1=0;
  l2=1;
  sscanf(str,"%d/%d ", &l1, &l2);
  if (l1 == 0) return;       /* empty string.. don't change default length */
  else {
    d=BASE/l2;
    if (d*l2 != BASE) {
      wng("Length incompatible with BASE, using 1/8: ",str);
      dlen=BASE/8;
    }
    else
      dlen = d*l1;
  }
  if (verbose>=4)
    printf ("Dlen  <%s> sets default note length to %d/%d = 1/%d\n",
            str, dlen, BASE, BASE/dlen);

  meter->dlen=dlen;

}

/* ----- set_keysig: interpret keysig string, store in struct ---- */
/* This part was adapted from abc2mtex by Chris Walshaw */
/* updated 03 Oct 1997 Wil Macaulay - support all modes */
/* Returns 1 if key was actually specified, because then we want
   to transpose. Returns zero if this is just a clef change. */
int set_keysig(s,ks,init)
char s[];
struct KEYSTR *ks;
int init;
{
  int c,sf,j,ok;
  char w[81];
  int ktype,add_pitch,root,root_acc;

  /* maybe initialize with key C (used for K field in header) */
  if (init) {
    ks->sf         = 0;
    ks->ktype      = TREBLE;
    ks->add_pitch  = 0;
    ks->root       = 2;
    ks->root_acc   = A_NT;
  }

  /* check for "treble" "bass" "alto" with no other information */
  ktype=ks->ktype;
  add_pitch=0;
  if (!strcmp(s,"bass"))    {ks->ktype=BASS;    return 0;}
  if (!strcmp(s,"treble"))  {ks->ktype=TREBLE;  return 0;}
  if (!strcmp(s,"treble8")) {ks->ktype=TREBLE8; return 0;}
  if (!strcmp(s,"alto"))    {ks->ktype=ALTO;    return 0;}

  if (!strcmp(s,"bass+8"))    {ks->ktype=BASS;    ks->add_pitch=8; return 0;}
  if (!strcmp(s,"treble+8"))  {ks->ktype=TREBLE;  ks->add_pitch=8; return 0;}
  if (!strcmp(s,"treble8+8")) {ks->ktype=TREBLE8; ks->add_pitch=8; return 0;}
  if (!strcmp(s,"alto+8"))    {ks->ktype=ALTO;    ks->add_pitch=8; return 0;}

  if (!strcmp(s,"bass-8"))    {ks->ktype=BASS;    ks->add_pitch=-8; return 0;}
  if (!strcmp(s,"treble-8"))  {ks->ktype=TREBLE;  ks->add_pitch=-8; return 0;}
  if (!strcmp(s,"treble8-8")) {ks->ktype=TREBLE8; ks->add_pitch=-8; return 0;}
  if (!strcmp(s,"alto-8"))    {ks->ktype=ALTO;    ks->add_pitch=-8; return 0;}

  c=0;
  bagpipe=0;
  switch (s[c]) {
  case 'F':
    sf = -1; root=5;
    break;
  case 'C':
    sf = 0; root=2;
    break;
  case 'G':
    sf = 1; root=6;
    break;
  case 'D':
    sf = 2; root=3;
    break;
  case 'A':
    sf = 3; root=0;
    break;
  case 'E':
    sf = 4; root=4;
    break;
  case 'B':
    sf = 5; root=1;
    break;
  case 'H':
    bagpipe=1;
    c++;
    if      (s[c] == 'P') { sf=0; root=2; }
    else if (s[c] == 'p') { sf=2; root=3; }
    else wng("unknown bagpipe-like key: ",s);
    break;
  default:
    wng ("Using C because key not recognised: ", s);
    sf = 0; root=2;
  }
  c++;

  root_acc=A_NT;
  if (s[c] == '#') {
    sf += 7;
    c += 1;
    root_acc=A_SH;
  } else if (s[c] == 'b') {
    sf -= 7;
    c += 1;
    root_acc=A_FT;
  }

  /* loop over blank-delimited words: get the next token in lower case */
  for (;;) {
    while (s[c] == ' ') c++;
    if (s[c]=='\0') break;

    j=0;
    while ((s[c]!=' ') && (s[c]!='\0')) { w[j]=tolower(s[c]); c++; j++; }
    w[j]='\0';

    /* now identify this word */

    /* first check for mode specifier */
    if ((strncmp(w,"mix",3)) == 0) {
      sf -= 1;
      ok = 1;
      /* dorian mode on the second note (D in C scale) */
    } else if ((strncmp(w,"dor",3)) == 0) {
      sf -= 2;
      ok = 1;
      /* phrygian mode on the third note (E in C scale) */
    } else if ((strncmp(w,"phr",3)) == 0) {
      sf -= 4;
      ok = 1;
      /* lydian mode on the fourth note (F in C scale) */
    } else if ((strncmp(w,"lyd",3)) == 0) {
      sf += 1;
      ok = 1;
      /* locrian mode on the seventh note (B in C scale) */
    } else if ((strncmp(w,"loc",3)) == 0) {
      sf -= 5;
      ok = 1;
      /* major and ionian are the same ks */
    } else if ((strncmp(w,"maj",3)) == 0) {
      ok = 1;
    } else if ((strncmp(w,"ion",3)) == 0) {
      ok = 1;
      /* aeolian, m, minor are the same ks - sixth note (A in C scale) */
    } else if ((strncmp(w,"aeo",3)) == 0) {
      sf -= 3;
      ok = 1;
    } else if ((strncmp(w,"min",3)) == 0) {
      sf -= 3;
      ok = 1;
    } else if ((strcmp(w,"m")) == 0) {
      sf -= 3;
      ok = 1;
    }

    /* check for trailing "bass" "treble" "alto" */
    else if (!strcmp(w,"bass")) {
      ktype=BASS;
    }
    else if (!strcmp(w,"treble")) {
      ktype=TREBLE;
    }
    else if (!strcmp(w,"treble8")) {
      ktype=TREBLE8;
    }
    else if (!strcmp(w,"alto")) {
      ktype=ALTO;
    }

    /* check for "+8" or "-8" */
    else if (!strcmp(w,"+8")) {
      add_pitch=7;
    }
    else if (!strcmp(w,"-8")) {
      add_pitch=-7;
    }
    else wng("Unknown token in key specifier: ",w);

  }  /* end of loop over blank-delimted words */

  if (verbose>=4) printf ("Key   <%s> gives sharpsflats %d, type %d\n",
                          s, sf, ktype);

  /* copy to struct */
  ks->sf         = sf;
  ks->ktype      = ktype;
  ks->add_pitch  = add_pitch;
  ks->root       = root;
  ks->root_acc   = root_acc;

  return 1;

}

/* ----- get_halftones: figure out how by many halftones to transpose --- */
/*  In the transposing routines: pitches A..G are coded as with 0..7 */
int get_halftones (key, transpose)
struct KEYSTR key;
char transpose[];
{
  int pit_old,pit_new,direction,stype,root_new,racc_new,nht;
  int root_old, racc_old;
  char *q;
  /* pit_tab associates true pitches 0-11 with letters A-G */
  int pit_tab[] = {0,2,3,5,7,8,10};

  if (strlen(transpose)==0) return 0;
  root_old=key.root;
  racc_old=key.root_acc;

  /* parse specification for target key */
  q=transpose;
  direction=0;

  if (*q=='^') {
    direction=1; q++;
  } else if (*q=='_') {
    direction=-1; q++;
  }
  stype=1;
  if (strchr("ABCDEFG",*q)) {
    root_new=*q-'A'; q++; stype=2;
  } else if (strchr("abcdefg",*q)) {
    root_new=*q-'a'; q++;  stype=2;
  }

  /* first case: offset was given directly as numeric argument */
  if (stype==1) {
    sscanf(q,"%d", &nht);
    if (direction<0) nht=-nht;
    if (nht==0) {
      if (direction<0) nht=-12;
      if (direction>0) nht=+12;
    }
    return nht;
  }

  /* second case: root of target key was specified explicitly */
  racc_new=0;
  if (*q=='b') {
    racc_new=A_FT; q++;
  } else if (*q=='#') {
    racc_new=A_SH; q++;
  } else if (*q!='\0')
    wng ("expecting accidental in transpose spec: ", transpose);

  /* get pitch as number from 0-11 for root of old key */
  pit_old=pit_tab[root_old];
  if (racc_old==A_FT) pit_old--;
  if (racc_old==A_SH) pit_old++;
  if (pit_old<0)  pit_old+=12;
  if (pit_old>11) pit_old-=12;

  /* get pitch as number from 0-11 for root of new key */
  pit_new=pit_tab[root_new];
  if (racc_new==A_FT) pit_new--;
  if (racc_new==A_SH) pit_new++;
  if (pit_new<0)  pit_new+=12;
  if (pit_new>11) pit_new-=12;

  /* number of halftones is difference */
  nht=pit_new-pit_old;
  if (direction==0) {
    if (nht>6)  nht-=12;
    if (nht<-5) nht+=12;
  }
  if (direction>0 && nht<=0) nht+=12;
  if (direction<0 && nht>=0) nht-=12;

  return nht;

}



/* ----- shift_key: make new key by shifting nht halftones --- */
void shift_key (sf_old,nht,sfnew,addt)
int sf_old,nht,*sfnew,*addt;
{
  int sf_new,r_old,r_new,add_t,  dh,dr;
  int skey_tab[] = {2,6,3,0,4,1,5,2};
  int fkey_tab[] = {2,5,1,4,0,3,6,2};
  char root_tab[]={'A','B','C','D','E','F','G'};

  /* get sf_new by adding 7 for each halftone, then reduce mod 12 */
  sf_new=sf_old+nht*7;
  sf_new=(sf_new+240)%12;
  if (sf_new>=6) sf_new=sf_new-12;

  /* get old and new root in ionian mode, shift is difference */
  r_old=2;
  if (sf_old>0) r_old=skey_tab[sf_old];
  if (sf_old<0) r_old=fkey_tab[-sf_old];
  r_new=2;
  if (sf_new>0) r_new=skey_tab[sf_new];
  if (sf_new<0) r_new=fkey_tab[-sf_new];
  add_t=r_new-r_old;

  /* fix up add_t to get same "decade" as nht */
  dh=(nht+120)/12; dh=dh-10;
  dr=(add_t+70)/7; dr=dr-10;
  add_t=add_t+7*(dh-dr);

  if (verbose>=8)
    printf ("shift_key: sf_old=%d new %d   root: old %c new %c  shift by %d\n",
            sf_old, sf_new, root_tab[r_old], root_tab[r_new], add_t);

  *sfnew=sf_new;
  *addt=add_t;

}


/* ----- set_transtab: setup for transposition by nht halftones --- */
void set_transtab (nht,key)
int nht;
struct KEYSTR *key;
{
  int a,b,sf_old,sf_new,add_t,i,j,acc_old,acc_new,root_old,root_acc;
  /* for each note A..G, these tables tell how many sharps (resp. flats)
     the keysig must have to get the accidental on this note. Phew. */
  int sh_tab[] = {5,7,2,4,6,1,3};
  int fl_tab[] = {3,1,6,4,2,7,5};
  /* tables for pretty printout only */
  char root_tab[]={'A','B','C','D','E','F','G'};
  char acc_tab[][3] ={"bb","b ","  ","# ","x "};
  char c1[6],c2[6],c3[6];

  /* nop if no transposition is wanted */
  if (nht==0) {
    key->add_transp=0;
    for (i=0;i<7;i++) key->add_acc[i]=0;
    return;
  }

  /* get new sharps_flats and shift of numeric pitch; copy to key */
  sf_old   = key->sf;
  root_old = key->root;
  root_acc = key->root_acc;
  shift_key (sf_old, nht, &sf_new, &add_t);
  key->sf = sf_new;
  key->add_transp = add_t;

  /* set up table for conversion of accidentals */
  for (i=0;i<7;i++) {
    j=i+add_t;
    j=(j+70)%7;
    acc_old=0;
    if ( sf_old >= sh_tab[i]) acc_old=1;
    if (-sf_old >= fl_tab[i]) acc_old=-1;
    acc_new=0;
    if ( sf_new >= sh_tab[j]) acc_new=1;
    if (-sf_new >= fl_tab[j]) acc_new=-1;
    key->add_acc[i]=acc_new-acc_old;
  }

  /* printout keysig change */
  if (verbose>=3) {
    i=root_old;
    j=i+add_t;
    j=(j+70)%7;
    acc_old=0;
    if ( sf_old >= sh_tab[i]) acc_old=1;
    if (-sf_old >= fl_tab[i]) acc_old=-1;
    acc_new=0;
    if ( sf_new >= sh_tab[j]) acc_new=1;
    if (-sf_new >= fl_tab[j]) acc_new=-1;
    strcpy(c3,"s"); if (nht==1 || nht==-1) strcpy(c3,"");
    strcpy(c1,""); strcpy(c2,"");
    if (acc_old==-1) strcpy(c1,"b"); if (acc_old==1) strcpy(c1,"#");
    if (acc_new==-1) strcpy(c2,"b"); if (acc_new==1) strcpy(c2,"#");
    printf ("Transpose root from %c%s to %c%s (shift by %d halftone%s)\n",
            root_tab[i],c1,root_tab[j],c2,nht,c3);
  }

  /* printout full table of transformations */
  if (verbose>=4) {
    printf ("old & new keysig    conversions\n");
    for (i=0;i<7;i++) {
      j=i+add_t;
      j=(j+70)%7;
      acc_old=0;
      if ( sf_old >= sh_tab[i]) acc_old=1;
      if (-sf_old >= fl_tab[i]) acc_old=-1;
      acc_new=0;
      if ( sf_new >= sh_tab[j]) acc_new=1;
      if (-sf_new >= fl_tab[j]) acc_new=-1;
      printf("%c%s-> %c%s           ", root_tab[i],acc_tab[acc_old+2],
             root_tab[j],acc_tab[acc_new+2]);
      for (a=-1;a<=1;a++) {
        b=a+key->add_acc[i];
        printf ("%c%s-> %c%s  ", root_tab[i],acc_tab[a+2],
                root_tab[j],acc_tab[b+2]);
      }
      printf ("\n");
    }
  }

}

/* ----- do_transpose: transpose numeric pitch and accidental --- */
void do_transpose (key,pitch,acc)
struct KEYSTR key;
int *pitch, *acc;
{
  int pitch_old,pitch_new,sf_old,sf_new,acc_old,acc_new,i,j;

  pitch_old = *pitch;
  acc_old   = *acc;
  pitch_new=pitch_old+key.add_transp;
  i=(pitch_old+70)%7;
  j=(pitch_new+70)%7;

  if (acc_old) {
    if (acc_old==A_DF) sf_old=-2;
    if (acc_old==A_FT) sf_old=-1;
    if (acc_old==A_NT) sf_old=0 ;
    if (acc_old==A_SH) sf_old=1;
    if (acc_old==A_DS) sf_old=2;
    sf_new=sf_old+key.add_acc[i];
    if (sf_new==-2) acc_new=A_DF;
    if (sf_new==-1) acc_new=A_FT;
    if (sf_new== 0) acc_new=A_NT;
    if (sf_new== 1) acc_new=A_SH;
    if (sf_new== 2) acc_new=A_DS;
  }
  else {
    acc_new=0;
  }
  *pitch = pitch_new;
  *acc   = acc_new;
}


/* ----- gch_transpose: transpose guitar chord string in gch --- */
void gch_transpose (key)
struct KEYSTR key;
{
  char *q,*r;
  char str[201];
  int root_old,root_new,sf_old,sf_new,ok;
  char root_tab[]={'A','B','C','D','E','F','G'};
  char root_tub[]={'a','b','c','d','e','f','g'};

  if (halftones==0) return;

  /* try to avoid some common abuses of gchord string */
  if (strstr(gch,"capo")) return;
  if (strstr(gch,"Capo")) return;
  if (strstr(gch,"Fine")) return;
  if (strstr(gch,"fine")) return;

  q=gch;
  r=str;

  for (;;) {
    while (*q==' ' || *q=='(') { *r=*q; q++; r++; }
    if (*q=='\0') break;
    ok=0;
    if (strchr("ABCDEFG",*q)) {
      root_old=*q-'A'; q++; ok=1;
    } else if (strchr("abcdefg",*q)) {
      root_old=*q-'a'; q++; ok=2;
    }

    if (ok) {
      sf_old=0;
      if (*q=='b') { sf_old=-1; q++; }
      if (*q=='#') { sf_old= 1; q++; }
      root_new=root_old+key.add_transp;
      root_new=(root_new+28)%7;
      sf_new=sf_old+key.add_acc[root_old];
      if (ok==1) { *r=root_tab[root_new]; r++; }
      if (ok==2) { *r=root_tub[root_new]; r++; }
      if (sf_new==-1) { *r='b'; r++; }
      if (sf_new== 1) { *r='#'; r++; }
    }

    while (*q!=' ' && *q!='/' && *q!='\0') {*r=*q; q++; r++; }
    if (*q=='/') {*r=*q; q++; r++; }

  }

  *r='\0';
/*|   printf("tr_ch: <%s>  <%s>\n", gch, str);   |*/

  strcpy (gch,str);

}


/* ----- init_parse_params: initialize variables for parsing --- */
void init_parse_params ()
{
  int i;

  slur=0;
  voice[0].end_slur=0;
  nwpool=nwline=0;
  ntinext=0;

  /* for continuation after output: reset nsym, switch to first voice */
  for (i=0;i<nvoice;i++) {
    voice[i].nsym=0;
    voice[i].insert_btype=0;
    voice[i].end_slur=0;
  }

  ivc=0;

  word=0;
  carryover=0;
  last_note=last_real_note=-1;
  pplet=qplet=rplet=0;
  num_ending=0;
  mes1=mes2=0;
  strcpy (gch, "");

}

/* ----- add_text ---- */
void add_text (str,type)
char str[];
int type;
{
  if (! make_ps) return;
  if (ntext>=NTEXT) {
    wng ("No more room for text line <%s>", str);
    return;
  }
  strcpy (text[ntext], str);
  text_type[ntext]=type;
  ntext++;
}

/* ----- reset_info ---- */
void reset_info (inf)
struct ISTRUCT *inf;
{

  /* reset all info fields except info.xref */

  strcpy(inf->parts,  "");
  strcpy(inf->area,   "");
  strcpy(inf->book,   "");
  inf->ncomp=0;
  strcpy(inf->disc,   "");
  strcpy(inf->group,  "");
  strcpy(inf->hist,   "");
  strcpy(inf->info,   "");
  strcpy(inf->key,    "C");
  strcpy(inf->meter,  "4/4");
  strcpy(inf->notes,  "");
  strcpy(inf->orig,   "");
  strcpy(inf->rhyth,  "");
  strcpy(inf->src,    "");
/*  strcpy(inf->title,  "(untitled)");  */
  strcpy(inf->title,  "");
  strcpy(inf->title2, "");
  strcpy(inf->title3, "");
  strcpy(inf->trans,  "");
  strcpy(inf->tempo,  "");
}

/* ----- get_default_info: set info to default, except xref field --- */
void get_default_info ()
{
  char savestr[STRL];

  strcpy (savestr, info.xref);
  info=default_info;
  strcpy (info.xref, savestr);

}

/* ----- is_info_field: identify any type of info field ---- */
int is_info_field (str)
char str[];
{
  if (strlen(str)<2) return 0;
  if (str[1]!=':')   return 0;
  if (str[0]=='|')   return 0;   /* |: at start of music line */
  return 1;
}

/* ----- is_end_line: identify eof ----- */
int is_end_line (str)
char str[];
{
  if (strlen(str)<3) return 0;
  if (str[0]=='E' && str[1]=='N' && str[2]=='D') return 1;
  return 0;
}

/* ----- is_pseudocomment ----- */
int is_pseudocomment (str)
char str[];
{
  if (strlen(str)<2) return 0;
  if ((str[0]=='%')&&(str[1]=='%'))  return 1;
  return 0;
}

/* ----- is_comment ----- */
int is_comment (str)
char str[];
{
  if (strlen(str)<1) return 0;
  if (str[0]=='%')  return 1;
  if (str[0]=='\\') return 1;
  return 0;
}


/* ----- trim_title: move trailing "The" to front ------------ */
void trim_title (s,s0)
char s[],s0[];
{
  char *q;
  char rest[81],str[301];
  int done;

  strcpy (str, s0);
  done=0;

  if ((q=strchr(str,','))) {
    if (*q != '\0') {
      strip (rest,q+1);
      if (!strcmp(rest,"The")) {
        strcpy (s, rest);
        *q = '\0';
        strcat (s, " ");
        strcat (s, str);
        done=1;
      }
    }
  }

  if (!done) strcpy (s,s0);

}


/* ----- find_voice ----- */
int find_voice (vid,new)
char vid[];
int *new;
{
  int i;

  for (i=0;i<nvoice;i++)
    if (!strcmp(vid,voice[i].id)) {
      *new=0;
      return i;
    }

  i=nvoice;
  if (i>=maxVc)
    rxi("Too many voices; use -maxv to increase limit, now ",maxVc);

  strcpy(voice[i].id,    vid);
  strcpy(voice[i].name,  "");
  strcpy(voice[i].sname, "");
  voice[i].stems    = 0;
  voice[i].staves   = 0;
  voice[i].brace    = 0;
  voice[i].bracket  = 0;
  voice[i].do_gch   = 1;
  voice[i].select   = 1;
  voice[i].sep=0.0;
  voice[i].meter = default_meter;
  voice[i].key   = default_key;
  voice[i].nsym   = 0;
  nvoice++;
  if (verbose>5)
    printf ("Make new voice %d with id \"%s\"\n", i,voice[i].id);
  *new=1;
  return i;

}

/* ----- switch_voice: read spec for a voice, return voice number ----- */
int switch_voice (str)
char str[];
{
  int j,np,new,ap;
  char *r,*q;
  char t1[201],t2[201];

  if (!do_this_tune) return 0;

  j=-1;

  /* start loop over vioce options: parse t1=t2 */
  r=str;
  np=0;
  for (;;) {
    while (*r == ' ') r++;
    if (*r=='\0') break;
    strcpy(t1,"");
    strcpy(t2,"");
    q=t1;
    while (*r!=' ' && *r!='\0' && *r!='=') { *q=*r; r++; q++; }
    *q='\0';
    if (*r=='=') {
      r++;
      q=t2;
      if (*r=='"') {
        r++;
        while (*r!='"' && *r!='\0') { *q=*r; r++; q++; }
        if (*r=='"') r++;
      }
      else {
        while (*r!=' ' && *r!='\0') { *q=*r; r++; q++; }
      }
      *q='\0';
    }
    np++;

    /* interpret the parsed option. First case is identifier. */
    if (np==1) j=find_voice (t1,&new);

    else {                         /* interpret option */
      if (j<0) bug("j invalid in switch_voice",1);
      if      (!strcmp(t1,"name")    || !strcmp(t1,"nm"))
        strcpy(voice[j].name,  t2);

      else if (!strcmp(t1,"sname")   || !strcmp(t1,"snm"))
        strcpy(voice[j].sname, t2);

      else if (!strcmp(t1,"staves")  || !strcmp(t1,"stv"))
        voice[j].staves  = atoi(t2);

      else if (!strcmp(t1,"brace")   || !strcmp(t1,"brc"))
        voice[j].brace   = atoi(t2);

      else if (!strcmp(t1,"bracket") || !strcmp(t1,"brk"))
        voice[j].bracket = atoi(t2);

      else if (!strcmp(t1,"gchords") || !strcmp(t1,"gch"))
        g_logv (str,t2,&voice[j].do_gch);

      /* for sspace: add 2000 as flag if not incremental */
      else if (!strcmp(t1,"space")   || !strcmp(t1,"spc")) {
        g_unum (str,t2,&voice[j].sep);
        if (t2[0]!='+' && t2[0]!='-') voice[j].sep += 2000.0;
      }

      else if (!strcmp(t1,"clef")    || !strcmp(t1,"cl")) {
        ap=0;
        if      (!strcmp(t2,"treble"))    voice[j].key.ktype=TREBLE;
        else if (!strcmp(t2,"treble+8"))  {voice[j].key.ktype=TREBLE; ap=+7;}
        else if (!strcmp(t2,"treble-8"))  {voice[j].key.ktype=TREBLE; ap=-7;}
        else if (!strcmp(t2,"treble+16")) {voice[j].key.ktype=TREBLE; ap=+14;}
        else if (!strcmp(t2,"treble-16")) {voice[j].key.ktype=TREBLE; ap=-14;}
        else if (!strcmp(t2,"treble8"))   voice[j].key.ktype=TREBLE8;
        else if (!strcmp(t2,"treble8+8")) {voice[j].key.ktype=TREBLE8; ap=+7;}
        else if (!strcmp(t2,"treble8-8")) {voice[j].key.ktype=TREBLE8; ap=-7;}
        else if (!strcmp(t2,"treble8+16")){voice[j].key.ktype=TREBLE8; ap=+14;}
        else if (!strcmp(t2,"treble8-16")){voice[j].key.ktype=TREBLE8; ap=-14;}
        else if (!strcmp(t2,"bass"))      voice[j].key.ktype=BASS;
        else if (!strcmp(t2,"bass+8"))    {voice[j].key.ktype=BASS; ap=+7;}
        else if (!strcmp(t2,"bass-8"))    {voice[j].key.ktype=BASS; ap=-7;}
        else if (!strcmp(t2,"bass+16"))   {voice[j].key.ktype=BASS; ap=+14;}
        else if (!strcmp(t2,"bass-16"))   {voice[j].key.ktype=BASS; ap=-14;}
        else if (!strcmp(t2,"alto"))       voice[j].key.ktype=ALTO;
        else if (!strcmp(t2,"alto+8"))    {voice[j].key.ktype=ALTO; ap=+7;}
        else if (!strcmp(t2,"alto-8"))    {voice[j].key.ktype=ALTO; ap=-7;}
        else if (!strcmp(t2,"alto+16"))   {voice[j].key.ktype=ALTO; ap=+14;}
        else if (!strcmp(t2,"alto-16"))   {voice[j].key.ktype=ALTO; ap=-14;}
        else wng("Unknown clef in voice spec: ",t2);
        voice[j].key.add_pitch=ap;
      }
      else if (!strcmp(t1,"stems") || !strcmp(t1,"stm")) {
        if      (!strcmp(t2,"up"))    voice[j].stems=1;
        else if (!strcmp(t2,"down"))  voice[j].stems=-1;
        else if (!strcmp(t2,"free"))  voice[j].stems=0;
        else wng("Unknown stem setting in voice spec: ",t2);
      }
      else wng("Unknown option in voice spec: ",t1);
    }

  }

  /* if new voice was initialized, save settings im meter0, key0 */
  if (new) {
    voice[j].meter0 = voice[j].meter;
    voice[j].key0   = voice[j].key;
  }

  if (verbose>7)
    printf ("Switch to voice %d  <%s> <%s> <%s>  clef=%d\n",
            j,voice[j].id,voice[j].name,voice[j].sname,
            voice[j].key.ktype);

  nsym0=voice[j].nsym;  /* set nsym0 to decide about eoln later.. ugly */
  return j;

}


/* ----- info_field: identify info line, store in proper place  ---- */
/* switch within_block: either goes to default_info or info.
   Only xref ALWAYS goes to info. */
int info_field (str)
char str[];
{
  char t[STRL];
  struct ISTRUCT *inf;
  int i;

  for (i=0;i<strlen(str);i++) if (str[i]=='%') str[i]='\0';

  if (within_block) {
    inf=&info;
  }
  else {
    inf=&default_info;
  }

  if (strlen(str)<2) return 0;
  if (str[1]!=':')   return 0;
  if (str[0]=='|')   return 0;   /* |: at start of music line */

  if (str[0]=='X') {
    strip (info.xref,   &str[2]);
    xrefnum=get_xref(info.xref);
    return XREF;
  }

  else if (str[0]=='A') strip (inf->area,   &str[2]);
  else if (str[0]=='B') strip (inf->book,   &str[2]);
  else if (str[0]=='C') {
    if (inf->ncomp>=NCOMP)
      wng("Too many composer lines","");
    else {
      strip (inf->comp[inf->ncomp],&str[2]);
      inf->ncomp++;
    }
  }
  else if (str[0]=='D') {
    strip (inf->disc,   &str[2]);
    add_text (&str[2], TEXT_D);
  }

  else if (str[0]=='G') strip (inf->group,  &str[2]);
  else if (str[0]=='H') {
    strip (inf->hist,   &str[2]);
    add_text (&str[2], TEXT_H);
    return HISTORY;
  }
  else if (str[0]=='W') {
    add_text (&str[2], TEXT_W);
    return WORDS;
  }
  else if (str[0]=='I') strip (inf->info,   &str[2]);
  else if (str[0]=='K') {
    strip (inf->key,    &str[2]);
    return KEY;
  }
  else if (str[0]=='L') {
    strip (inf->len,    &str[2]);
    return DLEN;
  }
  else if (str[0]=='M') {
    strip (inf->meter,  &str[2]);
    return METER;
  }
  else if (str[0]=='N') {
    strip (inf->notes,  &str[2]);
    add_text (&str[2], TEXT_N);
  }
  else if (str[0]=='O') strip (inf->orig,   &str[2]);
  else if (str[0]=='R') strip (inf->rhyth,  &str[2]);
  else if (str[0]=='P') {
    strip (inf->parts,  &str[2]);
    return PARTS;
  }
  else if (str[0]=='S') strip (inf->src,    &str[2]);
  else if (str[0]=='T') {
    strip (t, &str[2]);
    numtitle++;
    if (numtitle>3) numtitle=3;
    if (numtitle==1)      trim_title (inf->title,  t);
    else if (numtitle==2) trim_title (inf->title2, t);
    else if (numtitle==3) trim_title (inf->title3, t);
    return TITLE;
  }
  else if (str[0]=='V') {
    strip (lvoiceid,  &str[2]);
    return VOICE;
  }
  else if (str[0]=='Z') {
    strip (inf->trans,  &str[2]);
    add_text (&str[2], TEXT_Z);
  }
  else if (str[0]=='Q') {
    strip (inf->tempo,  &str[2]);
    return TEMPO;
  }

  else if (str[0]=='E') ;

  else {
    return 0;
  }

  return INFO;
}

/* ----- append_meter: add meter to list of symbols -------- */
void append_meter (meter)
struct METERSTR meter;
{
  int kk;

  kk=add_sym(TIMESIG);
  symv[ivc][kk]=zsym;
  symv[ivc][kk].type = TIMESIG;
  symv[ivc][kk].u    = meter.meter1;
  symv[ivc][kk].v    = meter.meter2;
  symv[ivc][kk].w    = meter.mflag;
  strcpy(symv[ivc][kk].text,meter.top);

}

/* ----- append_key_change: append change of key to sym list ------ */
void append_key_change(oldkey,newkey)
struct KEYSTR oldkey,newkey;
{
  int n1,n2,t1,t2,kk;

  n1=oldkey.sf;
  t1=A_SH;
  if (n1<0) { n1=-n1; t1=A_FT; }
  n2=newkey.sf;
  t2=A_SH;

  if (newkey.ktype != oldkey.ktype) {      /* clef change */
    kk=add_sym(CLEF);
    symv[ivc][kk].u=newkey.ktype;
    symv[ivc][kk].v=1;
  }

  if (n2<0) { n2=-n2; t2=A_FT; }
  if (t1==t2) {              /* here if old and new have same type */
    if (n2>n1) {                 /* more new symbols ..*/
      kk=add_sym(KEYSIG);        /* draw all of them */
      symv[ivc][kk].u=1;
      symv[ivc][kk].v=n2;
      symv[ivc][kk].w=100;
      symv[ivc][kk].t=t1;
      symv[ivc][kk].q=newkey.sf;
    }
    else if (n2<n1) {            /* less new symbols .. */
      kk=add_sym(KEYSIG);          /* draw all new symbols and neutrals */
      symv[ivc][kk].u=1;
      symv[ivc][kk].v=n1;
      symv[ivc][kk].w=n2+1;
      symv[ivc][kk].t=t2;
      symv[ivc][kk].q=newkey.sf;
    }
    else return;
  }
  else {                     /* here for change s->f or f->s */
    kk=add_sym(KEYSIG);          /* neutralize all old symbols */
    symv[ivc][kk].u=1;
    symv[ivc][kk].v=n1;
    symv[ivc][kk].w=1;
    symv[ivc][kk].t=t1;
    kk=add_sym(KEYSIG);          /* add all new symbols */
    symv[ivc][kk].u=1;
    symv[ivc][kk].v=n2;
    symv[ivc][kk].w=100;
    symv[ivc][kk].t=t2;
    symv[ivc][kk].q=newkey.sf;
  }

}



/* ----- numeric_pitch ------ */
/* adapted from abc2mtex by Chris Walshaw */
int numeric_pitch(char note)
{

  if (note=='z')
    return 14;
  if (note >= 'C' && note <= 'G')
    return(note-'C'+16+voice[ivc].key.add_pitch);
  else if (note >= 'A' && note <= 'B')
    return(note-'A'+21+voice[ivc].key.add_pitch);
  else if (note >= 'c' && note <= 'g')
    return(note-'c'+23+voice[ivc].key.add_pitch);
  else if (note >= 'a' && note <= 'b')
    return(note-'a'+28+voice[ivc].key.add_pitch);
  printf ("numeric_pitch: cannot identify <%c>\n", note);
  return(0);
}

/* ----- symbolic_pitch: translate numeric pitch back to symbol ------ */
int symbolic_pitch(pit,str)
int pit;
char str[];
{
  int  p,r,s;
  char ltab1[7] = {'C','D','E','F','G','A','B'};
  char ltab2[7] = {'c','d','e','f','g','a','b'};

  p=pit-16;
  r=(p+700)%7;
  s=(p-r)/7;

  if (p<7) {
    sprintf (str,"%c,,,,,",ltab1[r]);
    str[1-s]='\0';
  }
  else {
    sprintf (str,"%c'''''",ltab2[r]);
    str[s]='\0';
  }
  return 0;
}

/* ----- handle_inside_field: act on info field inside body of tune --- */
void handle_inside_field(type)
int type;
{
  struct KEYSTR oldkey;
  int rc;

  if (type==METER) {
    if (nvoice==0) ivc=switch_voice (DEFVOICE);
    set_meter (info.meter,&voice[ivc].meter);
    append_meter (voice[ivc].meter);
  }

  else if (type==DLEN) {
    if (nvoice==0) ivc=switch_voice (DEFVOICE);
    set_dlen (info.len,  &voice[ivc].meter);
  }

  else if (type==KEY) {
    if (nvoice==0) ivc=switch_voice (DEFVOICE);
    oldkey=voice[ivc].key;
    rc=set_keysig(info.key,&voice[ivc].key,0);
    if (rc) set_transtab (halftones,&voice[ivc].key);
    append_key_change(oldkey,voice[ivc].key);
  }

  else if (type==VOICE) {
    ivc=switch_voice (lvoiceid);
  }

}



/* ----- parse_uint: parse for unsigned integer ----- */
int parse_uint ()
{
  int number,ndig;
  char num[21];

  if (!isdigit(*p)) return 0;
  ndig=0;
  while (isdigit(*p)) {
    num[ndig]=*p;
    ndig++;
    num[ndig]=0;
    p++;
  }
  sscanf (num, "%d", &number);
  if (db>3) printf ("  parsed unsigned int %d\n", number);
  return number;

}

/* ----- parse_bar: parse for some kind of bar ---- */
int parse_bar ()
{
  int k;

  parse_gchord ();

  /* special cases: [1 or [2 without a preceeding bar, [| */
  if (*p=='[') {
    if ((*(p+1)=='1') || (*(p+1)=='2')) {
      k=add_sym (BAR);
      symv[ivc][k].u=B_INVIS;
      symv[ivc][k].v=1;
      if (*(p+1)=='2') symv[ivc][k].v=2;
      p=p+2;
      return 1;
    }
  }

  /* identify valid standard bar types */
  if (*p == '|') {
    p++;
    if (*p == '|') {
      k=add_sym (BAR);
      symv[ivc][k].u=B_DBL;
      p++;
    }
    else if (*p == ':') {
      k=add_sym(BAR);
      symv[ivc][k].u=B_LREP;
      p++;
    }
    else if (*p==']') {                  /* code |] for fat end bar */
      k=add_sym(BAR);
      symv[ivc][k].u=B_FAT2;
      p=p+1;
    }
    else {
      k=add_sym(BAR);
      symv[ivc][k].u=B_SNGL;
    }
  }
  else if (*p == ':') {
    p++;
    if (*p == '|') {
      k=add_sym(BAR);
      symv[ivc][k].u=B_RREP;
      p++;
    }
    else if (*p == ':') {
      k=add_sym(BAR);
      symv[ivc][k].u=B_DREP;
      p++; }
    else {
      syntax ("Syntax error parsing bar", p-1);
      return 0;
    }
  }

  else if ((*p=='[') && (*(p+1)=='|') && (*(p+2)==']')) {  /* code [|] invis */
    k=add_sym(BAR);
    symv[ivc][k].u=B_INVIS;
    p=p+3;
  }

  else if ((*p=='[') && (*(p+1)=='|')) {    /* code [| for thick-thin bar */
    k=add_sym(BAR);
    symv[ivc][k].u=B_FAT1;
    p=p+2;
  }

  else return 0;

  strcpy(symv[ivc][k].text,"");
  if (strlen(gch)>0) {
    strcpy (symv[ivc][k].text, gch);
    strcpy (gch, "");
  }

  /* see if valid bar is followed by specifier for first or second ending */
  if (*p=='1')      { symv[ivc][k].v=1; p++; }
  else if (*p=='2') { symv[ivc][k].v=2; p++; }
  else if ((*p=='[') && (*(p+1)=='1')) { symv[ivc][k].v=1; p=p+2; }
  else if ((*p=='[') && (*(p+1)=='2')) { symv[ivc][k].v=2; p=p+2; }
  else if ((*p==' ') && (*(p+1)=='[') && (*(p+2)=='1'))
    { symv[ivc][k].v=1; p=p+3; }
  else if ((*p==' ') && (*(p+1)=='[') && (*(p+2)=='2'))
    { symv[ivc][k].v=2; p=p+3; }

  return 1;
}

/* ----- parse_space: parse for whitespace ---- */
int parse_space ()
{
  int rc;

  rc=0;
  while ((*p==' ')||(*p=='\t')) {
    rc=1;
    p++;
  }
  if (db>3) if (rc) printf ("  parsed whitespace\n");
  return rc;
}

/* ----- parse_esc: parse for escape sequence ----- */
int parse_esc ()
{

  int nseq;
  char *pp;

  if (*p == '\\') {                     /* try for \...\ sequence */
    p++;
    nseq=0;
    while ((*p!='\\') && (*p!=0)) {
      escseq[nseq]=*p;
      nseq++;
      p++;
    }
    if (*p == '\\') {
      p++;
      escseq[nseq]=0;
      if (db>3) printf ("  parsed esc sequence <%s>\n", escseq);
      return ESCSEQ;
    }
    else {
      if (cfmt.breakall) return DUMMY;
      if (db>3) printf ("  parsed esc to EOL.. continuation\n");
    }
    return CONTINUE;
  }

  /* next, try for [..] sequence */
  if ((*p=='[') && (*(p+1)>='A') && (*(p+1)<='Z') && (*(p+2)==':')) {
    pp=p;
    p++;
    nseq=0;
    while ((*p!=']') && (*p!=0)) {
      escseq[nseq]=*p;
      nseq++;
      p++;
    }
    if (*p == ']') {
      p++;
      escseq[nseq]=0;
      if (db>3) printf ("  parsed esc sequence <%s>\n", escseq);
      return ESCSEQ;
    }
    syntax ("Escape sequence [..] not closed", pp);
    return ESCSEQ;
  }
  return 0;
}


/* ----- parse_nl: parse for newline ----- */
int parse_nl ()
{

  if ((*p == '\\')&&(*(p+1)=='\\')) {
    p+=2;
    return 1;
  }
  else
    return 0;
}

/* ----- parse_gchord: parse guitar chord, save in buffer ----- */
int parse_gchord ()
{
  char *q;
  int n;

  if (*p != '"') return 0;

  q=p;
  p++;
  n=strlen(gch);
  if (n > 0) syntax ("Overwrite unused guitar chord", q);

  while ((*p != '"') && (*p != 0)) {
    gch[n]=*p;
    n++;
    if (n >= 200) {
      syntax ("String for guitar chord too long", q);
      return 1;
    }
    p++;
  }
  if (*p == 0) {
    syntax ("EOL reached while parsing guitar chord", q);
    return 1;
  }
  p++;
  gch[n]=0;
  if (db>3) printf("  parse guitar chord <%s>\n", gch);

/*|   gch_transpose (voice[ivc].key); |*/

  return 1;
}


/* ----- parse_deco: parse for decoration on note ----- */
int parse_deco (dtype)
int dtype[10];
{
  int deco,n;

  n=0;

  for (;;) {
    deco=0;
    if (*p == '~')  {
      if (DECO_IS_ROLL) deco=D_ROLL;
      else              deco=D_GRACE;
    }
    if (*p == '.')  deco=D_STACC;
    if (*p == 'J')  deco=D_SLIDE;
    if (*p == 'M')  deco=D_EMBAR;
    if (*p == 'H')  deco=D_HOLD;
    if (*p == 'R')  deco=D_ROLL;
    if (*p == 'T')  deco=D_TRILL;
    if (*p == 'u')  deco=D_UPBOW;
    if (*p == 'v')  deco=D_DOWNBOW;
    if (*p == 'K')  deco=D_HAT;
    if (*p == 'k')  deco=D_ATT;

    if (deco) {
      p++;
      dtype[n]=deco;
      n++;
    }
    else
      break;
  }

  return n;
}


/* ----- parse_length: parse length specifer for note or rest --- */
int parse_length ()
{
  int len,fac;

  len=voice[ivc].meter.dlen;          /* start with default length */

  if (len<=0) printf ("!!! parse_len:  got len=%d\n", len);


  if (isdigit(*p)) {                 /* multiply note length */
    fac=parse_uint ();
    if (fac==0) fac=1;
    len *= fac;
  }

  if (*p=='/') {                   /* divide note length */
    while (*p=='/') {
      p++;
      if (isdigit(*p))
        fac=parse_uint();
      else
        fac=2;
      if (len%fac) {
        syntax ("Bad length divisor", p-1);
        return len;
      }
      len=len/fac;
    }
  }

  return len;

}

/* ----- parse_grace_sequence --------- */
int parse_grace_sequence (pgr,agr)
int pgr[],agr[];
{

  char *p0;
  int n,len;

  p0=p;
  if (*p != '{') return 0;
  p++;

  n=0;
  while (*p != '}') {
    if (*p == '\0') {
      syntax ("Unbalanced grace note sequence", p0);
      return 0;
    }
    if (!isnote(*p)) {
      syntax ("Unexpected symbol in grace note sequence", p);
      p++;
    }
    agr[n]=0;
    if (*p == '=') agr[n]=A_NT;
    if (*p == '^') {
      if (*(p+1)=='^') { agr[n]=A_DS; p++; }
      else agr[n]=A_SH;
    }
    if (*p == '_') {
      if (*(p+1)=='_') { agr[n]=A_DF; p++; }
      else agr[n]=A_FT;
    }
    if (agr[n]) p++;

    pgr[n] = numeric_pitch(*p);
    p++;
    while (*p == '\'') { pgr[n] += 7; p++; }
    while (*p == ',') {  pgr[n] -= 7; p++; }

    do_transpose (voice[ivc].key, &pgr[n], &agr[n]);

    len=parse_length ();      /* ignore any length specifier */
    n++;
  }

  p++;
  return n;
}

/* ----- identify_note: set head type, dots, flags for note --- */
void identify_note (s,q)
struct SYMBOL *s;
char *q;
{
  int head,base,len,flags,dots;

  if (s->len==0) s->len=s->lens[0];
  len=s->len;

  /* set flag if duration equals length of one measure */
  if (nvoice>0) {
    if (len==(WHOLE*voice[ivc].meter.meter1)/voice[ivc].meter.meter2)
      s->fullmes=1;
  }

  base=WHOLE;
  if (len>=WHOLE)              base=WHOLE;
  else if (len>=HALF)          base=HALF;
  else if (len>=QUARTER)       base=QUARTER;
  else if (len>=EIGHTH)        base=EIGHTH;
  else if (len>=SIXTEENTH)     base=SIXTEENTH;
  else if (len>=THIRTYSECOND)  base=THIRTYSECOND;
  else if (len>=SIXTYFOURTH)   base=SIXTYFOURTH;
  else syntax("Cannot identify head for note",q);

  if (base==WHOLE)     head=H_OVAL;
  else if (base==HALF) head=H_EMPTY;
  else                 head=H_FULL;

  if (base==SIXTYFOURTH)        flags=4;
  else if (base==THIRTYSECOND)  flags=3;
  else if (base==SIXTEENTH)     flags=2;
  else if (base==EIGHTH)        flags=1;
  else                          flags=0;

  dots=0;
  if (len==base)            dots=0;
  else if (2*len==3*base)   dots=1;
  else if (4*len==7*base)   dots=2;
  else if (8*len==15*base)  dots=3;
  else syntax("Cannot handle note length for note",q);

/*  printf ("identify_note: length %d gives head %d, dots %d, flags %d\n",
          len,head,dots,flags);  */

  s->head=head;
  s->dots=dots;
  s->flags=flags;
}


/* ----- double_note: change note length for > or < char --- */
/* Note: if symv[ivc][i] is a chord, the length shifted to the following
   note is taken from the first note head. Problem: the crazy syntax
   permits different lengths within a chord. */
void double_note (i,num,sign,q)
int i,num,sign;
char *q;
{
  int m,shift,j,len;

  if ((symv[ivc][i].type!=NOTE) && (symv[ivc][i].type!=REST))
    bug("sym is not NOTE or REST in double_note", 1);

  shift=0;
  len=symv[ivc][i].lens[0];
  for (j=0;j<num;j++) {
    len=len/2;
    shift -= sign*len;
    symv[ivc][i].len += sign*len;
    for (m=0;m<symv[ivc][i].npitch;m++) symv[ivc][i].lens[m] += sign*len;
  }
  identify_note (&symv[ivc][i],q);
  carryover += shift;
}

/* ----- parse_basic_note: parse note or rest with pitch and length --*/
int parse_basic_note (pitch,length,accidental)
int *pitch,*length,*accidental;
{
  int pit,len,acc;

  acc=pit=0;                       /* look for accidental sign */
  if (*p == '=') acc=A_NT;
  if (*p == '^') {
    if (*(p+1)=='^') { acc=A_DS; p++; }
    else acc=A_SH;
  }
  if (*p == '_') {
    if (*(p+1)=='_') { acc=A_DF; p++; }
    else acc=A_FT;
  }

  if (acc) {
    p++;
    if (!strchr("CDEFGABcdefgab",*p)) {
      syntax("Missing note after accidental", p-1);
      return 0;
    }
  }
  if (!isnote(*p)) {
    syntax ("Expecting note", p);
    p++;
    return 0;
  }

  pit= numeric_pitch(*p);             /* basic pitch */
  p++;

  while (*p == '\'') {                /* eat up following ' chars */
    pit += 7;
    p++;
  }

  while (*p == ',') {                 /* eat up following , chars */
    pit -= 7;
    p++;
  }

  len=parse_length();

  do_transpose (voice[ivc].key, &pit, &acc);

  *pitch=pit;
  *length=len;
  *accidental=acc;

  if (db>3) printf ("  parsed basic note,"
                    "length %d/%d = 1/%d, pitch %d\n",
                    len,BASE,BASE/len,pit);

  return 1;

}


/* ----- parse_note: parse for one note or rest with all trimmings --- */
int parse_note ()
{
  int k,deco,i,chord,m,type,rc,sl1,sl2,j;
  int pitch,length,accidental,invis;
  int ngr,pgr[30],agr[30],dtype[30];
  char *q,*q0;

  ngr=parse_grace_sequence(pgr,agr);   /* grace notes */

  parse_gchord();                      /* permit chord after graces */

  deco=parse_deco(dtype);              /* decorations */

  parse_gchord();                      /* permit chord after deco */

  chord=0;                             /* determine if chord */
  q=p;
  if ((*p=='+') || (*p=='[')) { chord=1; p++; }

  type=invis=0;
  if (isnote(*p)) type=NOTE;
  if (chord && (*p=='(')) type=NOTE;
  if (chord && (*p==')')) type=NOTE;   /* this just for better error msg */
  if ((*p=='z')||(*p=='Z')) type=REST;
  if ((*p=='x')||(*p=='X')) {type=REST; invis=1; }
  if (!type) return 0;

  k=add_sym(type);                     /* add new symbol to list */


  symv[ivc][k].dc.n=deco;              /* copy over pre-parsed stuff */
  for (i=0;i<deco;i++)
    symv[ivc][k].dc.t[i]=dtype[i];
  symv[ivc][k].gr.n=ngr;
  for (i=0;i<ngr;i++) {
    symv[ivc][k].gr.p[i]=pgr[i];
    symv[ivc][k].gr.a[i]=agr[i];
  }
  if (strlen(gch)>0) {
    gch_transpose (voice[ivc].key);
    strcpy (symv[ivc][k].text, gch);
    strcpy (gch, "");
  }

  q0=p;
  if (type==REST) {
    p++;
    symv[ivc][k].lens[0] = parse_length();
    symv[ivc][k].npitch=1;
    symv[ivc][k].invis=invis;
    if (db>3) printf ("  parsed rest, length %d/%d = 1/%d\n",
                      symv[ivc][k].lens[0],BASE,BASE/symv[ivc][k].lens[0]);
  }
  else {
    m=0;                                 /* get pitch and length */
    sl1=sl2=0;
    for (;;) {
      if (chord && (*p=='(')) {
        sl1++;
        symv[ivc][k].sl1[m]=sl1;
        p++;
      }
      deco=parse_deco(dtype);     /* for extra decorations within chord */
      for (i=0;i<deco;i++) symv[ivc][k].dc.t[i+symv[ivc][k].dc.n]=dtype[i];
      symv[ivc][k].dc.n += deco;

      rc=parse_basic_note (&pitch,&length,&accidental);
      if (rc==0) { voice[ivc].nsym--; return 0; }
      symv[ivc][k].pits[m] = pitch;
      symv[ivc][k].lens[m] = length;
      symv[ivc][k].accs[m] = accidental;
      symv[ivc][k].ti1[m]  = symv[ivc][k].ti2[m] = 0;
      for (j=0;j<ntinext;j++)
        if (tinext[j]==symv[ivc][k].pits[m]) symv[ivc][k].ti2[m]=1;

      if (chord && (*p=='-')) {symv[ivc][k].ti1[m]=1; p++;}

      if (chord && (*p==')')) {
        sl2++;
        symv[ivc][k].sl2[m]=sl2;
        p++;
      }

      if (chord && (*p=='-')) {symv[ivc][k].ti1[m]=1; p++;}

      m++;

      if (!chord) break;
      if ((*p=='+')||(*p==']')) {
        p++;
        break;
      }
      if (*p=='\0') {
        if (chord) syntax ("Chord not closed", q);
        return type;
      }
    }
    ntinext=0;
    for (j=0;j<m;j++)
      if (symv[ivc][k].ti1[j]) {
        tinext[ntinext]=symv[ivc][k].pits[j];
        ntinext++;
      }
    symv[ivc][k].npitch=m;
  }

  for (m=0;m<symv[ivc][k].npitch;m++) {   /* add carryover from > or < */
    if (symv[ivc][k].lens[m]+carryover<=0) {
      syntax("> leads to zero or negative note length",q0);
    }
    else
      symv[ivc][k].lens[m] += carryover;
  }
  carryover=0;

  if (db>3) printf ("  parsed note, decos %d, text <%s>\n",
                    symv[ivc][k].dc.n, symv[ivc][k].text);


  symv[ivc][k].yadd=0;
  if (voice[ivc].key.ktype==BASS) symv[ivc][k].yadd=-6;
  if (voice[ivc].key.ktype==ALTO) symv[ivc][k].yadd=-3;
  identify_note (&symv[ivc][k],q0);
  return type;
}


/* ----- parse_sym: parse a symbol and return its type -------- */
int parse_sym ()
{
  int i;

  if (parse_gchord())   return GCHORD;
  if (parse_bar())      return BAR;
  if (parse_space())    return SPACE;
  if (parse_nl())       return NEWLINE;
  if ((i=parse_esc()))  return i;
  if ((i=parse_note())) return i;
  if (parse_nl())       return NEWLINE;
  return 0;
}

/* ----- add_wd ----- */
char *add_wd(str)
char str[];
{
  char *rp;
  int l;

  l=strlen(str);
  if (l==0) return 0;
  if (nwpool+l+1>NWPOOL)
    rx ("Overflow while parsing vocals; increase NWPOOL and recompile.","");

  strcpy(wpool+nwpool, str);
  rp=wpool+nwpool;
  nwpool=nwpool+l+1;
  return rp;
}

/* ----- parse_vocals: parse words below a line of music ----- */
/* Use '^' to mark a '-' between syllables - hope nobody needs '^' ! */
int parse_vocals (line)
char line[];
{
  int isym;
  char *c,*c1,*w;
  char word[81];

  if ((line[0]!='w') || (line[1]!=':')) return 0;
  p0=line;

  isym=nsym0-1;
  c=line+2;
  for (;;) {
    while(*c==' ') c++;
    if (*c=='\0') break;
    c1=c;
    if ((*c=='_') || (*c=='*') || (*c=='|') || (*c=='-')) {
      word[0]=*c;
      if (*c=='-') word[0]='^';
      word[1]='\0';
      c++;
    }
    else {
      w=word;
      *w='\0';
      while ((*c!=' ') && (*c!='\0')) {
        if ((*c=='_') || (*c=='*') || (*c=='|')) break;
        if (*c=='-') {
          if (*(c-1) != '\\') break;
          w--;
          *w='-';
        }
        *w=*c; w++; c++;
      }
      if (*c=='-') { *w='^' ; w++; c++; }
      *w='\0';
    }

    /* now word contains a word, possibly with trailing '^',
       or one of the special characters * | _ -               */

    if (!strcmp(word,"|")) {               /* skip forward to next bar */
      isym++;
      while ((symv[ivc][isym].type!=BAR) && (isym<voice[ivc].nsym)) isym++;
      if (isym>=voice[ivc].nsym)
        { syntax("Not enough bar lines for |",c1); break; }
    }

    else {                                 /* store word in next note */
      w=word;
      while (*w!='\0') {                   /* replace * and ~ by space */
		/* cd: escaping with backslash possible */
        if ( ((*w=='*') || (*w=='~')) && (*(w-1) != '\\') ) *w=' ';
        w++;
      }
      isym++;
      while ((symv[ivc][isym].type!=NOTE) && (isym<voice[ivc].nsym)) isym++;
      if (isym>=voice[ivc].nsym)
        { syntax ("Not enough notes for words",c1); break; }
      symv[ivc][isym].wordp[nwline]=add_wd(word);
    }

    if (*c=='\0') break;
  }

  nwline++;
  return 1;
}


/* ----- parse_music_line: parse a music line into symbols ----- */
int parse_music_line (line)
char line[];
{
  int type,num,nbr,n,itype,i;
  char msg[81];
  char *p1,*pmx;

  if (ivc>=nvoice) bug ("Trying to parse undefined voice",1);

  nwline=0;
  nsym0=voice[ivc].nsym;

  nbr=0;
  p=p0=line;
  pmx=p+strlen(p);

  while (*p != 0) {
    if (p>pmx) break;                /* emergency exit */
    type=parse_sym();
    n=voice[ivc].nsym;
    i=n-1;
    if ((db>4) && type)
      printf ("   sym[%d] code (%d,%d)\n",
              n-1,symv[ivc][n-1].type,symv[ivc][n-1].u);

    if (type==NEWLINE) {
      if ((n>0) && !cfmt.continueall && !cfmt.barsperstaff) {
        symv[ivc][i].eoln=1;
        if (word) {
          if (last_note>=0) symv[ivc][last_note].word_end=1;
          word=0;
        }
      }
    }

    if (type==ESCSEQ) {
      if (db>3)
        printf ("Handle escape sequence <%s>\n", escseq);
      itype=info_field (escseq);
      handle_inside_field (itype);
    }

    if (type==REST) {
      if (pplet) {                   /* n-plet can start on rest */
        symv[ivc][i].p_plet=pplet;
        symv[ivc][i].q_plet=qplet;
        symv[ivc][i].r_plet=rplet;
        pplet=0;
      }
      last_note=i;
      p1=p;
    }

    if (type==NOTE) {
      if (!word) {
        symv[ivc][i].word_st=1;
        word=1;
      }
      symv[ivc][i].slur_st+=nbr;
      nbr=0;
      if (voice[ivc].end_slur) symv[ivc][i].slur_end++;
      voice[ivc].end_slur=0;

      if (pplet) {                   /* start of n-plet */
        symv[ivc][i].p_plet=pplet;
        symv[ivc][i].q_plet=qplet;
        symv[ivc][i].r_plet=rplet;
        pplet=0;
      }
      last_note=last_real_note=i;
      p1=p;
    }

    /* for slurs into ending boxes */
    if (type==BAR) {
      if (symv[ivc][i].v == 1) voice[ivc].rem_slur=voice[ivc].end_slur;
      if ((symv[ivc][i].v==2)||(symv[ivc][i].u==B_RREP)||
          (symv[ivc][i].u==B_DREP)) {
        voice[ivc].end_slur=voice[ivc].rem_slur;
        last_note=-1;
      }
    }

    if (word && ((type==BAR)||(type==SPACE))) {
      if (last_real_note>=0) symv[ivc][last_real_note].word_end=1;
      word=0;
    }

    if (!type) {

      if (*p == '-') {                  /* a-b tie */
        if (last_note>=0) symv[ivc][last_note].slur_st++;
        voice[ivc].end_slur=1;
        p++;
      }

      else if (*p == '(') {
        p++;
        if (isdigit(*p)) {
          pplet=*p-'0'; qplet=0; rplet=pplet;
          p++;
          if (*p == ':') {
            p++;
            if (isdigit(*p)) { qplet=*p-'0';  p++; }
            if (*p == ':') {
              p++;
              if (isdigit(*p)) { rplet=*p-'0';  p++; }
            }
          }
        }
        else {
          nbr++;
        }
      }
      else if (*p == ')') {
        if (last_note>0)
          symv[ivc][last_note].slur_end++;
        else
          syntax ("Unexpected symbol",p);
        p++;
      }
      else if (*p == '>') {
        num=1;
        p++;
        while (*p == '>') { num++; p++; }
        if (last_note<0)
          syntax ("No note before > sign", p);
        else
          double_note (last_note, num, 1, p1);
      }
      else if (*p == '<') {
        num=1;
        p++;
        while (*p == '<') { num++; p++; }
        if (last_note<0)
          syntax ("No note before < sign", p);
        else
          double_note (last_note, num, -1, p1);
      }
      else if (*p == '*')     /* ignore stars for now  */
        p++;
      else if (*p == '!')     /* ditto for '!' */
        p++;
      else {
        if (*p != '\0')
          sprintf (msg, "Unexpected symbol \'%c\'", *p);
        else
          sprintf (msg, "Unexpected end of line");
        syntax (msg, p);
        p++;
      }
    }
  }

  /* maybe set end-of-line marker, if symbols were added */
  n=voice[ivc].nsym;

  if (n>nsym0) {
    symv[ivc][n-1].eoln=1;
    if (type==CONTINUE)     symv[ivc][n-1].eoln=0;
    if (cfmt.barsperstaff)  symv[ivc][n-1].eoln=0;
    if (cfmt.continueall)   symv[ivc][n-1].eoln=0;
  }



  /* break words at end of line */
  if (word && (symv[ivc][n-1].eoln==1)) {
    symv[ivc][last_note].word_end=1;
    word=0;
  }

  return TO_BE_CONTINUED;

}

/* ----- is_selected: check selection for current info fields ---- */
int is_selected (xref_str,npat,pat,select_all,search_field)
int npat,select_all,search_field;
char xref_str[],pat[][STRL1];
{
  int i,j,a,b,m;

  /* true if select_all or if no selectors given */
  if (select_all) return 1;
  if (isblankstr(xref_str) && (npat==0)) return 1;

  for (i=0;i<npat;i++) {             /*patterns */
    if (search_field==S_COMPOSER) {
      for (j=0;j<info.ncomp;j++) {
        if (!m) m=match(info.comp[j],pat[i]);
      }
    }
    else if (search_field==S_SOURCE)
      m=match(info.src,pat[i]);
    else if (search_field==S_RHYTHM)
      m=match(info.rhyth,pat[i]);
    else {
      m=match(info.title,pat[i]);
      if ((!m) && (numtitle>=2)) m=match(info.title2,pat[i]);
      if ((!m) && (numtitle>=3)) m=match(info.title3,pat[i]);
    }
    if (m) return 1;
  }

  /* check xref against string of numbers */
  p=xref_str;
  while (*p != 0) {
    parse_space();
    a=parse_uint();
    if (!a) return 0;          /* can happen if invalid chars in string */
    parse_space();
    if (*p == '-') {
      p++;
      parse_space();
      b=parse_uint();
      if (!b) {
        if (xrefnum>=a) return 1;
      }
      else
        for (i=a;i<=b;i++) if (xrefnum==i) return 1;
    }
    else {
      if (xrefnum==a) return 1;
    }
    if (*p == ',') p++;
  }

  return 0;

}

/* ----- rehash_selectors: split selectors into patterns and xrefs -- */
int rehash_selectors (sel_str, xref_str, pat)
char sel_str[], xref_str[];
char pat[][STRL1];
{
  char *q;
  char arg[501];
  int i,npat;

  npat=0;
  strcpy (xref_str, "");
  q=sel_str;

  i=0;
  while (1) {
    if ((*q==' ') || (*q=='\0')) {
      arg[i]='\0';
      i=0;
      if (!isblankstr(arg)) {
        if (arg[0]=='-')               /* skip any flags */
          ;
        else if (is_xrefstr(arg)) {
          strcat(xref_str, arg);
          strcat(xref_str, " ");
        }
        else {                         /* pattern with * or + */
          if ((strchr(arg,'*')) || (strchr(arg,'+'))) {
            strcpy(pat[npat],arg);
          }
          else {                       /* simple pattern */
            strcpy(pat[npat],"*");
            strcat(pat[npat],arg);
            strcat(pat[npat],"*");
          }
          npat++;
        }
      }
    }
    else {
      arg[i]=*q;
      i++;
    }
    if (*q=='\0') break;
    q++;
  }
  return npat;
}


/* ----- decomment_line: cut off after % ----- */
void decomment_line (ln)
char ln[];
{
  int i;

  for (i=0;i<strlen(ln);i++) if (ln[i]=='%') ln[i]='\0';

}


/* ----- get_line: read line, do first operations on it ----- */
int get_line (fp,ln)
FILE *fp;
char ln[];
{
  int l;

  strcpy (ln, "");
  if (feof(fp)) return 0;

  get_a_line(ln, BSIZE, fp);
/*|   fgets(ln, BSIZE, fp); |*/
  linenum++;
  l=strlen(ln);
  if (l>STRL) {
    if (verbose<=2) printf ("\n");
    printf ("+++ Line %d too long, truncate from %d to %d chars\n",
            linenum, l, STRL);
    l=STRL-1;
    ln[l]='\0';
  }
  if (is_end_line(ln))  return 0;
  if (ln[l-1]=='\n') ln[l-1]='\0';

  if ((verbose>=7) || (vb>=10) ) printf ("%3d  %s \n", linenum, ln);

  return 1;

}


/* ----- read_line: returns type of line scanned --- */
int read_line (fp,do_music,line)
FILE *fp;
int do_music;
char line[BSIZE];
{
  int type,nsym0;

  if (!get_line(fp,line)) return E_O_F;

  if (isblankstr(line))           return BLANK;
  if (is_pseudocomment(line))  return PSCOMMENT;
  if (is_comment(line))        return COMMENT;
  decomment_line (line);

  if ((type=info_field(line))) {
    /* skip after history field. Nightmarish syntax, that. */
    if (type != HISTORY)
      return type;
    else {
      for (;;) {
        if (! get_line(fp,line)) return E_O_F;
        if (isblankstr(line)) return BLANK;
        if (is_info_field(line)) break;
        add_text (line, TEXT_H);
      }
      type=info_field (line);
      return type;
    }
  }

  if (!do_music) return COMMENT;
  if (parse_vocals(line)) return MWORDS;

  /* now parse a real line of music */
  if (nvoice==0) ivc=switch_voice (DEFVOICE);

  nsym0=voice[ivc].nsym;
  type=parse_music_line (line);

  if (db>1)
    printf ("  parsed music symbols %d to %d for voice %d\n",
            nsym0,voice[ivc].nsym-1,ivc);

  return type;

}

/* ----- do_index: print index of abc file ------ */
void do_index(fp,xref_str,npat,pat,select_all,search_field)
FILE *fp;
int npat,select_all,search_field;
char xref_str[],pat[][STRL1];
{
  int type,within_tune;
  char line[BSIZE];

  linenum=0;
  verbose=vb;
  numtitle=0;
  write_history=0;
  within_tune=within_block=do_this_tune=0;
  reset_info (&default_info);
  info=default_info;

  for (;;) {
    if (!get_line(fp,line)) break;
    if (is_comment(line)) continue;
    decomment_line (line);
    type=info_field (line);

    switch (type) {

    case XREF:
      if (within_block)
        printf ("+++ Tune %d not closed properly \n", xrefnum);
      numtitle=0;
      within_tune=0;
      within_block=1;
      ntext=0;
      break;

    case KEY:
      if (!within_block) break;
      if (!within_tune) {
        tnum2++;
        if (is_selected (xref_str,npat,pat,select_all,search_field)) {
          printf ("  %-4d %-5s %-4s", xrefnum, info.key, info.meter);
          if      (search_field==S_SOURCE)   printf ("  %-15s", info.src);
          else if (search_field==S_RHYTHM)   printf ("  %-8s",  info.rhyth);
          else if (search_field==S_COMPOSER) printf ("  %-15s", info.comp[0]);
          if (numtitle==3)
            printf ("  %s - %s - %s", info.title,info.title2,info.title3);
          if (numtitle==2) printf ("  %s - %s", info.title, info.title2);
          if (numtitle==1) printf ("  %s", info.title);

          printf ("\n");
          tnum1++;
        }
        within_tune=1;
      }
      break;

    }

    if (isblankstr(line)) {
      if (within_block && !within_tune)
        printf ("+++ Header not closed in tune %d\n", xrefnum);
      within_tune=0;
      within_block=0;
      info=default_info;
    }
  }
  if (within_block && !within_tune)
    printf ("+++ Header not closed in for tune %d\n", xrefnum);

}





