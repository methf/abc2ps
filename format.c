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

#include "abc2ps.h"
#include "util.h"

#include "format.h"

/*  subroutines connected with page layout  */


/* ----- fontspec ----- */
void fontspec (struct FONTSPEC *f, char name[], float size, int box)
{
  strcpy (f->name, name);
  f->size = size;
  f->box  = box;
}

/* ----- add_font ----- */
/* checks font list, adds font if new */
int add_font (f)
struct FONTSPEC *f;
{
  int i,i0,fnum;

  i0=-1;
  for (i=0;i<nfontnames;i++) {
    if (!strcmp(f->name,fontnames[i])) i0=i;
  }

  if (i0>=0) {
    fnum=i0;
    if (vb>=10) printf("Already at %d: font %s\n",fnum,f->name);
  }
  else {
    fnum=nfontnames;
    strcpy(fontnames[fnum],f->name);
    if (vb>=10) printf("Add new at %d: font %s\n",fnum,f->name);
    nfontnames++;
  }
  return fnum;
}


/* ----- make_font_list ----- */
void make_font_list (f)
struct FORMAT *f;
{
  if (vb>=10) printf ("Adding fonts from format..\n");
  add_font (&f->titlefont);
  add_font (&f->subtitlefont);
  add_font (&f->composerfont);
  add_font (&f->partsfont);
  add_font (&f->vocalfont);
  add_font (&f->textfont);
  add_font (&f->wordsfont);
  add_font (&f->gchordfont);
  add_font (&f->barnumfont);
  add_font (&f->barlabelfont);
  add_font (&f->voicefont);
}

/* ----- set_standard_format ----- */
void set_standard_format (f)
struct FORMAT *f;
{
  strcpy (f->name,          "standard");
  strcpy (f->auquality,     "1");
  f->playtempo              =   PLAY_TEMPO;
  f->pageheight             =   PAGEHEIGHT * CM;
  f->staffwidth             =   STAFFWIDTH * CM;
  f->leftmargin             =   LEFTMARGIN * CM;
  f->topmargin              =   1.0  * CM;
  f->botmargin              =   1.0  * CM;
  f->topspace               =   0.8  * CM;
  f->titlespace             =   0.2  * CM;
  f->subtitlespace          =   0.1  * CM;
  f->composerspace          =   0.2  * CM;
  f->musicspace             =   0.2  * CM;
  f->partsspace             =   0.3  * CM;
  f->staffsep               =  46.0  * PT;
  f->sysstaffsep            =  40.0  * PT;
  f->systemsep              =  55.0  * PT;
  f->vocalspace             =  23.0  * PT;
  f->textspace              =   0.5  * CM;
  f->wordsspace             =   0.0  * CM;
  f->scale                  =   0.70;
  f->maxshrink              =   0.65;
  f->landscape              =   0;
  f->titleleft              =   0;
  f->stretchstaff           =   1;
  f->stretchlast            =   0;
  f->continueall            =   0;
  f->breakall               =   0;
  f->writehistory           =   0;
  f->withxrefs              =   0;
  f->oneperpage             =   0;
  f->titlecaps              =   0;
  f->barsperstaff           =   0;
  f->barnums                =   -1;
  f->lineskipfac            =   1.1;
  f->parskipfac             =   0.4;
  f->strict1                =   0.5;
  f->strict2                =   0.8;
  f->indent                 =   0.0;
  fontspec (&f->titlefont,     "Times-Roman",    15.0, 0);
  fontspec (&f->subtitlefont,  "Times-Roman",    12.0, 0);
  fontspec (&f->composerfont,  "Times-Italic",   11.0, 0);
  fontspec (&f->partsfont,     "Times-Roman",    11.0, 0);
  fontspec (&f->tempofont,     "Times-Bold",     10.0, 0);
  fontspec (&f->vocalfont,     "Times-Bold",     13.0, 0);
  fontspec (&f->textfont,      "Times-Roman",    12.0, 0);
  fontspec (&f->wordsfont,     "Times-Roman",    12.0, 0);
  fontspec (&f->gchordfont,    "Helvetica",      12.0, 0);
  fontspec (&f->voicefont,     "Times-Roman",    12.0, 0);
  fontspec (&f->barnumfont,    "Times-Italic",   12.0, 0);
  fontspec (&f->barlabelfont,  "Times-Bold",     18.0, 0);
  fontspec (&f->indexfont,     "Times-Roman",    11.0, 0);
  if (vb>=10) printf ("Loading format \"%s\"\n",f->name);
}

/* ----- set_pretty_format ----- */
void set_pretty_format (f)
struct FORMAT *f;
{
  set_standard_format (f);
  strcpy (f->name,    "pretty");
  f->titlespace             =   0.4  * CM;
  f->composerspace          =   0.25 * CM;
  f->musicspace             =   0.25 * CM;
  f->partsspace             =   0.3  * CM;
  f->staffsep               =  50.0  * PT;
  f->sysstaffsep            =  45.0  * PT;
  f->systemsep              =  55.0  * PT;
  f->scale                  =   0.75;
  f->maxshrink              =   0.55;
  f->parskipfac             =   0.1;
  fontspec (&f->titlefont,     "Times-Roman",    18.0, 0);
  fontspec (&f->subtitlefont,  "Times-Roman",    15.0, 0);
  fontspec (&f->composerfont,  "Times-Italic",   12.0, 0);
  fontspec (&f->partsfont,     "Times-Roman",    12.0, 0);
  fontspec (&f->tempofont,     "Times-Bold",     10.0, 0);
  fontspec (&f->vocalfont,     "Times-Bold",     14.0, 0);
  fontspec (&f->textfont,      "Times-Roman",    10.0, 0);
  fontspec (&f->wordsfont,     "Times-Roman",    10.0, 0);
  fontspec (&f->gchordfont,    "Helvetica",      12.0, 0);
  fontspec (&f->voicefont,     "Times-Roman",    12.0, 0);
}

/* ----- set_pretty2_format ----- */
void set_pretty2_format (f)
struct FORMAT *f;
{
  set_standard_format (f);
  strcpy (f->name,    "pretty2");
  f->titlespace             =   0.4  * CM;
  f->composerspace          =   0.3  * CM;
  f->musicspace             =   0.25 * CM;
  f->partsspace             =   0.2  * CM;
  f->staffsep               =  55.0  * PT;
  f->sysstaffsep            =  45.0  * PT;
  f->systemsep              =  55.0  * PT;
  f->textspace              =   0.2  * CM;
  f->scale                  =   0.70;
  f->maxshrink              =   0.55;
  f->titleleft              =   1;
  f->parskipfac             =   0.1;
  fontspec (&f->titlefont,     "Helvetica-Bold", 16.0, 0);
  fontspec (&f->subtitlefont,  "Helvetica-Bold", 13.0, 0);
  fontspec (&f->composerfont,  "Helvetica",      10.0, 0);
  fontspec (&f->partsfont,     "Times-Roman",    12.0, 0);
  fontspec (&f->tempofont,     "Times-Bold",     10.0, 0);
  fontspec (&f->vocalfont,     "Times-Bold",     13.0, 0);
  fontspec (&f->textfont,      "Times-Roman",    10.0, 0);
  fontspec (&f->wordsfont,     "Times-Roman",    10.0, 0);
  fontspec (&f->gchordfont,    "Helvetica",      12.0, 0);
  fontspec (&f->voicefont,     "Times-Roman",    12.0, 0);
  fontspec (&f->barnumfont,    "Times-Roman",    11.0, 1);
  fontspec (&f->barlabelfont,  "Times-Bold",     18.0, 1);
}


/* ----- print_font ----- */
void print_font (str,fs)
char *str;
struct FONTSPEC fs;
{
  printf ("  %-14s %s %.1f", str, fs.name, fs.size);
  if (fs.box) printf (" box");
  printf ("\n");
}


/* ----- print_format ----- */
void print_format (f)
struct FORMAT f;
{
  char yn[2][5]={"no","yes"};

  printf ("\nFormat \"%s\":\n", f.name);
  printf ("  pageheight     %.2fcm\n",    f.pageheight/CM);
  printf ("  staffwidth     %.2fcm\n",    f.staffwidth/CM);
  printf ("  topmargin      %.2fcm\n",    f.topmargin/CM);
  printf ("  botmargin      %.2fcm\n",    f.botmargin/CM);
  printf ("  leftmargin     %.2fcm\n",    f.leftmargin/CM);
  printf ("  topspace       %.2fcm\n",    f.topspace/CM);
  printf ("  titlespace     %.2fcm\n",    f.titlespace/CM);
  printf ("  subtitlespace  %.2fcm\n",    f.subtitlespace/CM);
  printf ("  composerspace  %.2fcm\n",    f.composerspace/CM);
  printf ("  musicspace     %.2fcm\n",    f.musicspace/CM);
  printf ("  partsspace     %.2fcm\n",    f.partsspace/CM);
  printf ("  wordsspace     %.2fcm\n",    f.wordsspace/CM);
  printf ("  textspace      %.2fcm\n",    f.textspace/CM);
  printf ("  vocalspace     %.1fpt\n",    f.vocalspace);
  printf ("  staffsep       %.1fpt\n",    f.staffsep);
  printf ("  sysstaffsep    %.1fpt\n",    f.sysstaffsep);
  printf ("  systemsep      %.1fpt\n",    f.systemsep);
  printf ("  scale          %.2f\n",      f.scale);
  printf ("  maxshrink      %.2f\n",      f.maxshrink);
  printf ("  strictness1    %.2f\n",      f.strict1);
  printf ("  strictness2    %.2f\n",      f.strict2);
  printf ("  indent         %.1fpt\n",    f.indent);

  print_font("titlefont",    f.titlefont);
  print_font("subtitlefont", f.subtitlefont);
  print_font("composerfont", f.composerfont);
  print_font("partsfont",    f.partsfont);
  print_font("tempofont",    f.tempofont);
  print_font("vocalfont",    f.vocalfont);
  print_font("gchordfont",   f.gchordfont);
  print_font("textfont",     f.textfont);
  print_font("wordsfont",    f.wordsfont);
  print_font("voicefont",    f.voicefont);
  print_font("barnumberfont",f.barnumfont);
  print_font("barlabelfont", f.barlabelfont);
  print_font("indexfont",    f.indexfont);

  printf ("  lineskipfac    %.1f\n",    f.lineskipfac);
  printf ("  parskipfac     %.1f\n",    f.parskipfac);
  printf ("  barsperstaff   %d\n",      f.barsperstaff);
  printf ("  barnumbers     %d\n",      f.barnums);
  printf ("  landscape      %s\n", yn[f.landscape]);
  printf ("  titleleft      %s\n", yn[f.titleleft]);
  printf ("  titlecaps      %s\n", yn[f.titlecaps]);
  printf ("  stretchstaff   %s\n", yn[f.stretchstaff]);
  printf ("  stretchlast    %s\n", yn[f.stretchlast]);
  printf ("  writehistory   %s\n", yn[f.writehistory]);
  printf ("  continueall    %s\n", yn[f.continueall]);
  printf ("  breakall       %s\n", yn[f.breakall]);
  printf ("  oneperpage     %s\n", yn[f.oneperpage]);
  printf ("  withxrefs      %s\n", yn[f.withxrefs]);

  printf ("  playtempo      %.0f\n", f.playtempo);
  printf ("  auquality      %s\n",   f.auquality);

}


/* ----- g_unum: read a number with a unit ----- */
void g_unum (l,s,num)
char *l,*s;
float *num;
{
  float a,b;
  char unit[81];

  strcpy(unit,"pt");
  sscanf(s,"%f%s", &a, unit);

  if      (!strcmp(unit,"cm")) b=a*CM;
  else if (!strcmp(unit,"mm")) b=a*CM*0.1;
  else if (!strcmp(unit,"in")) b=a*IN;
  else if (!strcmp(unit,"pt")) b=a*PT;
  else {
    printf ("+++ Unknown unit \"%s\" in line: %s\n",unit,l);
    exit (3);
  }
  *num = b;
}

/* ----- g_logv: read a logical variable ----- */
void g_logv (l,s,v)
char *l,*s;
int  *v;
{
  int k;
  char t[31];

  strcpy(t,"1");
  sscanf (s,"%s", t);
  if (!strcmp(t,"1") || !strcmp(t,"yes") || !strcmp(t,"true"))
    k=1;
  else if (!strcmp(t,"0") || !strcmp(t,"no") || !strcmp(t,"false"))
    k=0;
  else {
    printf ("\n+++ Unknown logical value \"%s\" near \"%s\"\n",t,l);
    exit (3);
  }
  *v = k;
}


/* ----- g_fltv: read a float variable, no units ----- */
void g_fltv (l,nch,v)
char   *l;
int    nch;
float  *v;
{
  float k;

  sscanf (l+nch,"%f", &k);
  *v = k;
}

/* ----- g_intv: read an int variable, no units ----- */
void g_intv (l,nch,v)
char   *l;
int    nch;
int    *v;
{
  int k;

  sscanf (l+nch,"%d", &k);
  *v = k;
}



/* ----- g_fspc: read a font specifier ----- */
void g_fspc (l,nch,fn)
char *l;
int  nch;
struct FONTSPEC *fn;
{
  char  fname[101],ssiz[101],sbox[101];
  float fsize;

  fsize=fn->size;
  strcpy(sbox,"SnOt");
  strcpy(ssiz,"SnOt");

  sscanf (l+nch,"%s %s %s", fname, ssiz, sbox);
  if (strcmp(fname,"*")) strcpy (fn->name, fname);

  if (strcmp(ssiz,"*")) sscanf(ssiz,"%f",&fsize);
  fn->size = fsize;

  if (!strcmp(sbox,"box"))       fn->box=1;
  else if (!strcmp(sbox,"SnOt")) ;
  else wng ("incorrect font spec: ", l+nch);

  if (!file_initialized) add_font (fn);
}


/* ----- interpret_format_line ----- */
/* read a line with a format directive, set in format struct f */
int interpret_format_line (l,f)
char l[];
struct FORMAT *f;
{
  char w[81],fnm[81];
  int nch,i,fnum;
  char *s;
  struct FONTSPEC tempfont;

  strcpy(w,"");
  sscanf(l,"%s%n", w, &nch);
  if (!strcmp(w,"")) return 0;
  if (w[0]=='%') return 0;
  if (vb>=6) printf ("Interpret format line: %s\n", l);
  if (!strcmp(w,"end")) return 1;
  s=l+nch;

  if      (!strcmp(w,"pageheight"))    g_unum(l,s,&f->pageheight);
  else if (!strcmp(w,"staffwidth"))    g_unum(l,s,&f->staffwidth);
  else if (!strcmp(w,"topmargin"))     g_unum(l,s,&f->topmargin);
  else if (!strcmp(w,"botmargin"))     g_unum(l,s,&f->botmargin);
  else if (!strcmp(w,"leftmargin"))    g_unum(l,s,&f->leftmargin);
  else if (!strcmp(w,"topspace"))      g_unum(l,s,&f->topspace);
  else if (!strcmp(w,"wordsspace"))    g_unum(l,s,&f->wordsspace);
  else if (!strcmp(w,"titlespace"))    g_unum(l,s,&f->titlespace);
  else if (!strcmp(w,"subtitlespace")) g_unum(l,s,&f->subtitlespace);
  else if (!strcmp(w,"composerspace")) g_unum(l,s,&f->composerspace);
  else if (!strcmp(w,"musicspace"))    g_unum(l,s,&f->musicspace);
  else if (!strcmp(w,"partsspace"))    g_unum(l,s,&f->partsspace);
  else if (!strcmp(w,"staffsep"))      g_unum(l,s,&f->staffsep);
  else if (!strcmp(w,"sysstaffsep"))   g_unum(l,s,&f->sysstaffsep);
  else if (!strcmp(w,"systemsep"))     g_unum(l,s,&f->systemsep);
  else if (!strcmp(w,"vocalspace"))    g_unum(l,s,&f->vocalspace);
  else if (!strcmp(w,"textspace"))     g_unum(l,s,&f->textspace);

  else if (!strcmp(w,"scale"))         g_fltv(l,nch,&f->scale);
  else if (!strcmp(w,"maxshrink"))     g_fltv(l,nch,&f->maxshrink);
  else if (!strcmp(w,"lineskipfac"))   g_fltv(l,nch,&f->lineskipfac);
  else if (!strcmp(w,"parskipfac"))    g_fltv(l,nch,&f->parskipfac);
  else if (!strcmp(w,"barsperstaff"))  g_intv(l,nch,&f->barsperstaff);
  else if (!strcmp(w,"barnumbers"))    g_intv(l,nch,&f->barnums);
  else if (!strcmp(w,"strictness1"))   g_fltv(l,nch,&f->strict1);
  else if (!strcmp(w,"strictness2"))   g_fltv(l,nch,&f->strict2);
  else if (!strcmp(w,"strictness")) {
    g_fltv(l,nch,&f->strict1); f->strict2=f->strict1; }
  else if (!strcmp(w,"indent"))        g_unum(l,s,&f->indent);

  else if (!strcmp(w,"titleleft"))     g_logv(l,s,&f->titleleft);
  else if (!strcmp(w,"titlecaps"))     g_logv(l,s,&f->titlecaps);
  else if (!strcmp(w,"landscape"))     g_logv(l,s,&f->landscape);
  else if (!strcmp(w,"stretchstaff"))  g_logv(l,s,&f->stretchstaff);
  else if (!strcmp(w,"stretchlast"))   g_logv(l,s,&f->stretchlast);
  else if (!strcmp(w,"continueall"))   g_logv(l,s,&f->continueall);
  else if (!strcmp(w,"breakall"))      g_logv(l,s,&f->breakall);
  else if (!strcmp(w,"writehistory"))  g_logv(l,s,&f->writehistory);
  else if (!strcmp(w,"withxrefs") )    g_logv(l,s,&f->withxrefs);
  else if (!strcmp(w,"oneperpage") )   g_logv(l,s,&f->oneperpage);

  else if (!strcmp(w,"titlefont"))     g_fspc(l,nch,&f->titlefont);
  else if (!strcmp(w,"subtitlefont"))  g_fspc(l,nch,&f->subtitlefont);
  else if (!strcmp(w,"vocalfont"))     g_fspc(l,nch,&f->vocalfont);
  else if (!strcmp(w,"partsfont"))     g_fspc(l,nch,&f->partsfont);
  else if (!strcmp(w,"tempofont"))     g_fspc(l,nch,&f->tempofont);
  else if (!strcmp(w,"textfont"))      g_fspc(l,nch,&f->textfont);
  else if (!strcmp(w,"composerfont"))  g_fspc(l,nch,&f->composerfont);
  else if (!strcmp(w,"wordsfont"))     g_fspc(l,nch,&f->wordsfont);
  else if (!strcmp(w,"gchordfont"))    g_fspc(l,nch,&f->gchordfont);
  else if (!strcmp(w,"voicefont"))     g_fspc(l,nch,&f->voicefont);
  else if (!strcmp(w,"barnumberfont")) g_fspc(l,nch,&f->barnumfont);
  else if (!strcmp(w,"barlabelfont"))  g_fspc(l,nch,&f->barlabelfont);
  else if (!strcmp(w,"indexfont"))     g_fspc(l,nch,&f->indexfont);
  else if (!strcmp(w,"playtempo"))     g_fltv(l,nch,&f->playtempo);
  else if (!strcmp(w,"auquality"))     sscanf (l+nch,"%s", &f->auquality);

  else if (!strcmp(w,"font")) {
    sscanf(l,"%*s %s", fnm);
    fnum=-1;
    for (i=0;i<nfontnames;i++) {
      if (!strcmp(fnm,fontnames[i])) fnum=i;
    }
    if (fnum<0) {
      if (file_initialized) {
        printf ("+++ Cannot predefine when output file open: %s\n", l);
        exit (3);
      }
      tempfont.size=12.0;
      g_fspc(l,nch,&tempfont);
    }
  }

  else {
    if (vb>=5) printf ("Ignore format line: %s\n", l);
    return 2;
  }
  return 0;
}

/* ----- read_fmt_file ----- */
int read_fmt_file (filename,dirname,f)
char filename [],dirname[];
struct FORMAT *f;
{
  FILE *fp;
  char line[BSIZE],fname[201];
  int i,end;

  strcpy(fname,filename);
  if ((fp = fopen (fname,"r")) == NULL) {
    if (strlen(dirname)==0)
      return 0;
    else {
      strcpy(fname,dirname);
      strcat(fname,"/");
      strcat(fname,filename);
      if ((fp = fopen (fname,"r")) == NULL) return 0;
    }
  }

  if (vb>=4) printf ("Reading format file %s:\n", fname);
  printf ("%s .. ", fname);
  strcpy (line, "");
  get_a_line(line, BSIZE, fp);
  for (i=0;i<200;i++) {
    end=interpret_format_line (line,f);
    if (end==1) return 1;
    strcpy (line, "");
    if (feof(fp)) return 1;
    if (!get_a_line(line, BSIZE, fp)) return 1 ;
  }
  fclose (fp);
  return 1;
}


