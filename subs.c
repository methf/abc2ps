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
#include "style.h"
#include "buffer.h"
#include "util.h"
#include "format.h"
#include "pssubs.h"
#include "parse.h"

#include "subs.h"


/*  miscellaneous subroutines  */

/* ----- write_help ----- */
void write_help ()
{

  printf ("abc2ps v%s.%s (%s, %s) compiled %s\n",
          VERSION, REVISION, VDATE, STYLE, __DATE__);

  printf ("Usage: abc2ps files..  [-e nums-or-pats] [other flags]\n"
          "  - show index of abc files or typeset tunes in Postscript.\n"
          "where: files   input files in abc format\n"
          "       nums    tune xref numbers, i.e. 1 3 6-7,20-\n"
          "       pats    patterns for title search\n"
          "Tunes are selected if they match a number or a pattern.\n"
          "Flags: -o      write output for selected tunes\n"
          "       -E      produce EPSF output, one tune per file\n"
          "       -O aa   set outfile name to aaa\n"
          "       -O =    make outfile name from infile/title\n"
          "       -I      write index to Ind.ps\n"
          "       -i      run in interactive mode\n"
          "       -v nn   set verbosity level to nn\n"
          "       -h      show this command summary\n"
          "  Selection:\n"
          "       -e      following arguments are selectors\n"
          "       -f      following arguments are file names\n"
          "       -T      search  Title   field (default)\n"
          "       -C      search Composer field instead of title\n"
          "       -R      search  Rhythm  field instead of title\n"
          "       -S      search  Source  field instead of title\n"
          "       -V str  select voices, eg. -V 1,4-5\n"
          "  Formatting:\n"
          "       -H      show the format parameters\n"
          "       -p      pretty output (looks better, needs more space)\n"
          "       -P      select second predefined pretty output style\n"
          "       -s xx   set scale factor for symbol size to xx\n"
          "       -w xx   set staff width (cm/in/pt)\n"
          "       -m xx   set left margin (cm/in/pt)\n"
          "       -d xx   set staff separation (cm/in/pt)\n"
          "       -x      include xref numbers in output\n"
          "       -k nn   number every nn bars; 0 for first in staff\n"
          "       -n      include notes and history in output\n"
          "       -N      write page numbers\n"
          "       -1      write one tune per page\n"
          "       -l      landscape mode\n"
          "       -g xx   set glue mode to shrink|space|stretch|fill\n"
          "       -F foo  read format from \"foo.fmt\"\n"
          "       -D bar  look for format files in directory \"bar\"\n"
          "       -X x    set strictness for note spacing, 0<x<1 \n"
          "  Transposing:\n"
          "       -t n    transpose by n halftones (_n for negative number)\n"
          "       -t XX   transpose root up/down to note XX (eg. C A# ^Bb _Bb)\n"
          "  Line breaks:\n"
          "       -b      break at all line ends (ignore continuations)\n"
          "       -c      continue all line ends (append '\\')\n"
          "       -B bb   put line break every bb bars\n"
          "  Alloc options:\n"
          "     -maxs n   set maximal number of symbols (default %d)\n"
          "     -maxv n   set maximal number of voices (default %d)\n"
          , AUDIOFILE, maxSyms, maxVc);
}


/* ----- write_version ----- */
void write_version ()
{

/*  printf ("abc2ps v%s (%s, %s) compiled %s\n",
          VERSION, VDATE, STYLE, __DATE__); */

  printf ("abc2ps v%s.%s (%s, %s) compiled %s\n",
          VERSION, REVISION, VDATE, STYLE, __DATE__);

  if (strlen(DEFAULT_FDIR)>0)
    printf ("Default format directory %s\n", DEFAULT_FDIR);

}

/* ----- is_xrefstr: check if string ok for xref selection ---- */
int is_xrefstr (str)
char str[];
{

  char *c;
  c=str;
  while (*c != '\0') {
    if ((!isdigit(*c)) && (*c!='-') && (*c!=',') && (*c!=' ')) return 0;
    c++;
  }
  return 1;
}


/* ----- make_arglist: splits one string into list or arguments ---- */
int make_arglist (str, av)
char str[];
char *av[];
{
  char *q;
  int n;

  q=str;
  n=1;                     /* first real arg is 1, as in argv */
  for (;;) {
    while (*q==' ') q++;
    if (*q=='\0') break;
    av[n]=q;
    n++;
    while ((*q!=' ') && (*q!='\0')) q++;
    if (*q=='\0') break;
    *q='\0';
    q++;
  }
  return n;
}


/* ----- init_ops ----- */
void init_ops (job)
int job;
{

  one_per_page         = -1;
  landscape            = -1;
  scalefac             = -1.0;
  lmargin              = -1.0;
  swidth               = -1.0;
  write_history        = -1;
  staffsep             = -1;
  dstaffsep            =  0;
  break_continues      = -1;
  continue_lines       = -1;
  include_xrefs        = -1;
  alfa_c               = -1.0;
  strict1              = -1.0;
  strict2              = -1.0;
  barnums              = -1;
  make_index           =  0;
  make_ps              =  0;


  select_all           = 0;
  do_mode              = DO_INDEX;
  pagenumbers          = 0;
  strcpy (styf, "");
  strcpy (transpose, "");
  strcpy (vcselstr, "");
  strcpy (au_qstr, "");

  if (job) {
    strcpy (styd, DEFAULT_FDIR);
    strcpy (outf, OUTPUTFILE);
    interactive          = 0;
    pretty               = 0;
    epsf                 = 0;
    choose_outname       = 0;
    gmode                = G_FILL;
    vb                   = VERBOSE0;
    search_field0        = S_TITLE;
  }
}


/* ----- ops_into_fmt ----- */
void ops_into_fmt (fmt)
struct FORMAT *fmt;
{
  if (landscape >= 0)        fmt->landscape=landscape;
  if (scalefac >= 0)         fmt->scale=scalefac;
  if (lmargin >= 0)          fmt->leftmargin=lmargin;
  if (swidth >= 0)           fmt->staffwidth=swidth;
  if (continue_lines >= 0)   fmt->continueall=continue_lines;
  if (break_continues >= 0)  fmt->breakall=break_continues;
  if (write_history >= 0)    fmt->writehistory=write_history;
  if (bars_per_line > 0)     fmt->barsperstaff=bars_per_line;
  if (include_xrefs >= 0)    fmt->withxrefs=include_xrefs;
  if (one_per_page >= 0)     fmt->oneperpage=one_per_page;
  if (alfa_c >= 0)           fmt->maxshrink=alfa_c;
  if (staffsep >= 0)         fmt->staffsep=staffsep;
  if (strict1 >= 0)          fmt->strict1=strict1;
  if (strict2 >= 0)          fmt->strict2=strict2;
  if (barnums >= 0)          fmt->barnums=barnums;
  if (strlen(au_qstr)>0)     strcpy(fmt->auquality, au_qstr);
  fmt->staffsep    = fmt->staffsep    + dstaffsep;
  fmt->sysstaffsep = fmt->sysstaffsep + dstaffsep;

}


/* ----- parse_args: parse list of arguments, interpret flags ----- */
int parse_args (ac, av)
int ac;
char *av[];
{
  int i,m,k,nsel,sel_arg,j,ok,f_pos,got_value;
  char c,aaa[201],ext[41];

  help_me=0;
  ninf=0;
  nsel=0;
  sel_arg=0;
  f_pos=-1;
  strcpy (sel_str[0], "");
  s_field[0]=search_field0;

  i=1;
  while (i<ac) {

    if (av[i][0]=='+') {        /* switch off flags with '+' */
      m=1;
      k=strlen(av[i]);
      while (m<k) {
        c=av[i][m];
        if      (c=='b')  break_continues=0;
        else if (c=='c')  continue_lines=0;
        else if (c=='x')  include_xrefs=0;
        else if (c=='1')  one_per_page=0;
        else if (c=='B')  bars_per_line=0;
        else if (c=='i')  interactive=0;
        else if (c=='n')  write_history=0;
        else if (c=='l')  landscape=0;
        else if (c=='p')  pretty=0;
        else if (c=='E')  epsf=0;
        else if (c=='F')  strcpy (styf, "");
        else if (c=='N')  pagenumbers=0;
        else if (c=='O')  { choose_outname=0; strcpy (outf, OUTPUTFILE); }
        else {
          printf ("+++ Cannot switch off flag: +%c\n", c);
          return 1;
        }
        m++;
      }
    }

    else if (av[i][0]=='-') {         /* interpret a flag with '-'*/

                                      /* identify fullword options first */
      if (!strcmp(av[i],"-maxv")) {
        if ((i==ac-1)||(av[i+1][0]=='-')) rx("missing parameter for ",av[i]);
        sscanf(av[++i],"%d",&maxVc);
        if (maxVc > 100) maxVc=100;
      }
      else if (!strcmp(av[i],"-maxs")) {
        if ((i==ac-1)||(av[i+1][0]=='-')) rx("missing parameter for ",av[i]);
        sscanf(av[++i],"%d",&maxSyms);
      }

      else {
        m=1;
        k=strlen(av[i]);
        while (m<k) {
          c=av[i][m];
          if      (c=='h')  help_me=1;                 /* simple flags */
          else if (c=='H')  help_me=2;
          else if (c=='e')  sel_arg=1;
          else if (c=='f')  {
            nsel++;
            strcpy (sel_str[nsel], "");
            if (ninf==0) {                /* selector before first file */
              strcpy(sel_str[nsel],sel_str[nsel-1]);
              s_field[nsel]=s_field[nsel-1];
            }
            s_field[nsel]=search_field0;
            sel_arg=0;
            f_pos=i;
          }
          else if (c=='b')  {break_continues=1; continue_lines=0;}
          else if (c=='c')  {continue_lines=1; break_continues=0;}
          else if (c=='x')  include_xrefs=1;
          else if (c=='1')  one_per_page=1;
          else if (c=='i')  interactive=1;
          else if (c=='n')  write_history=1;
          else if (c=='l')  landscape=1;
          else if (c=='p')  pretty=1;
          else if (c=='P')  pretty=2;
          else if (c=='E')  epsf=1;
          else if (c=='o')  { do_mode=DO_OUTPUT; make_ps=1; }
          else if (c=='N')  pagenumbers=1;
          else if (c=='I')  make_index=1;
          else if (strchr("TCRS",c)) {
            if (c=='T') s_field[nsel]=S_TITLE;
            if (c=='C') s_field[nsel]=S_COMPOSER;
            if (c=='R') s_field[nsel]=S_RHYTHM;
            if (c=='S') s_field[nsel]=S_SOURCE;
            sel_arg=1;
          }

          else if (strchr("vsdwmgtkqQDFYBVXO",c)) {  /* flags with parameter.. */
            strcpy (aaa, &av[i][m+1]);
            if ((strlen(aaa)>0) && strchr("glO",c)) {     /* no sticky arg */
              printf ("+++ Incorrect usage of flag -%c\n", c);
              return 1;
            }

            got_value=1;                              /* check for value */
            if (strlen(aaa)==0) {
              got_value=0;
              i++;
              if ((i>=ac) || (av[i][0]=='-'))
                i--;
              else {
                strcpy (aaa,av[i]);
                got_value=1;
              }
            }

            if (got_value && strchr("vskYB",c)) {      /* check num args */
              ok=1;
              for (j=0;j<strlen(aaa);j++)
                if (!strchr("0123456789.",aaa[j])) ok=0;
              if (!ok) {
                printf ("+++ Invalid parameter <%s> for flag -%c\n",aaa,c);
                return 1;
              }
            }

            /* --- next ops require a value --- */

            if (!got_value) {                   /* check value was given */
              printf ("+++ Missing parameter after flag -%c\n", c);
              return 1;
            }

            if (c=='k') {
              sscanf(aaa,"%d",&barnums);
              break;
            }

            if (c=='q') {
              strcpy (au_qstr, aaa);
              break;
            }

            if (c=='V') {                               /* -V flag */
              ok=1;
              strcpy (vcselstr, aaa);
            }

            if (c=='X') {                               /* -X flag */
              ok=1;
              if (aaa[0]==',') {
                sscanf(aaa,",%f",&strict2);
                if (strict2<-0.001 || strict2>1.001) ok=0;
              }
              else if (strchr(aaa,',')) {
                sscanf (aaa,"%f,%f",&strict1,&strict2);
                if (strict1<-0.001 || strict1>1.001) ok=0;
                if (strict2<-0.001 || strict2>1.001) ok=0;
              }
              else {
                sscanf(aaa,"%f",&strict1);
                if (strict1<-0.001 || strict1>1.001) ok=0;
                strict2=strict1;
              }
              if (!ok) {
                printf ("+++ Invalid parameter <%s> for flag -%c\n",aaa,c);
                return 1;
              }
            }

            if (c=='O') {                               /* -O flag  */
              if (!strcmp(aaa,"=")) {
                choose_outname=1;
              }
              else {
                getext (aaa,ext);
                if (strcmp(ext,"ps") && strcmp(ext,"eps") && strcmp(ext,"")) {
                  printf ("Wrong extension for output file: %s\n", aaa);
                  return 1;
                }
                strext (outf, aaa, "ps", 1);
                choose_outname=0;
              }
            }

            if (c=='B') {
              sscanf(aaa,"%d",&bars_per_line);
              continue_lines=0;
            }
            if (c=='t') strcpy(transpose,aaa);
            if (c=='v') sscanf(aaa,"%d",&vb);
            if (c=='s') sscanf(aaa,"%f",&scalefac);
            if (c=='d') {
              if (aaa[0]=='+' || aaa[0]=='-') dstaffsep = scan_u(aaa);
              else staffsep = scan_u(aaa);
            }
            if (c=='w') swidth   = scan_u(aaa);
            if (c=='m') lmargin  = scan_u(aaa);
            if (c=='F') strcpy(styf,aaa);
            if (c=='D') strcpy (styd,aaa);
            if (c=='g') {
              if      (abbrev(aaa,"shrink", 2)) gmode=G_SHRINK;
              else if (abbrev(aaa,"stretch",2)) gmode=G_STRETCH;
              else if (abbrev(aaa,"space",  2)) gmode=G_SPACE;
              else if (abbrev(aaa,"fill",   2)) gmode=G_FILL;
              else {
                printf ("+++ Bad parameter for flag -g: %s\n",aaa);
                return 1;
              }
            }
            break;
          }
          else {
            printf ("+++ Unknown flag: -%c\n", c);
            return 1;
          }
          m++;
        }
      }
    }

    else {
      if (strstr(av[i],".fmt")) {     /* implict -F */
        strcpy(styf, av[i]);
      }
      else {
        if (strstr(av[i],".abc") && sel_arg) {     /* file if .abc */
          nsel++;
          strcpy (sel_str[nsel], "");
          s_field[nsel]=S_TITLE;
          if (ninf==0) {                /* selector before first file */
            strcpy(sel_str[nsel],sel_str[nsel-1]);
            s_field[nsel]=s_field[nsel-1];
        }
          sel_arg=0;
        }
        if (is_xrefstr(av[i]) && (!sel_arg)) {    /* sel if xref numbers */
          if (i-1 != f_pos) sel_arg=1;
        }

        if (!sel_arg) {             /* this arg is a file name */
          if (ninf>=MAXINF) {
            printf ("+++ Too many input files, max is %d\n", MAXINF);
            return 1;
          }
          strcpy (in_file[ninf], av[i]);
          psel[ninf]=nsel;
          ninf++;
        }
        else {                             /* this arg is a selector */
          strcat(sel_str[nsel], av[i]);
          strcat(sel_str[nsel], " ");
        }
      }
    }
    i++;
  }

  return 0;
}

/* ----- alloc_structs ----- */
/* Thanks to Henrik Norbeck for this */
void alloc_structs ()
{
  int j;

  sym = (struct SYMBOL *)calloc(maxSyms, sizeof(struct SYMBOL));
  if (sym==NULL) rx("Out of memory","");

  symv = (struct SYMBOL **)calloc(maxVc, sizeof(struct SYMBOL *));
  if (symv==NULL) rx("Out of memory","");

  for (j=0;j<maxVc;j++) {
    symv[j] = (struct SYMBOL *)calloc(maxSyms, sizeof(struct SYMBOL));
    if (symv[j]==NULL) rx("Out of memory","");
  }

  xp = (struct XPOS *)calloc(maxSyms+1, sizeof(struct XPOS));
  if (xp==NULL) rx("Out of memory","");

  for (j=0;j<maxSyms+1;j++) {
    xp[j].p = (int *)calloc(maxVc, sizeof(int));
    if (xp[j].p==NULL) rx("Out of memory","");
  }

  voice=(struct VCESPEC *)calloc(maxVc, sizeof(struct VCESPEC));
  if (voice==NULL) rx("Out of memory","");

  sym_st = (struct SYMBOL **)calloc(maxVc, sizeof(struct SYMBOL *));
  if (sym_st==NULL) rx("Out of memory","");

  for (j=0;j<maxVc;j++) {
    sym_st[j] = (struct SYMBOL *)calloc(MAXSYMST, sizeof(struct SYMBOL));
    if (sym_st[j]==NULL) rx("Out of memory","");
  }

  nsym_st = (int *)calloc(maxVc, sizeof(int));
  if (nsym_st==NULL) rx("Out of memory","");

}


/* ----- set_page_format ----- */
int set_page_format ()
{
  int i,j;

  if (pretty==1)
    set_pretty_format (&cfmt);
  else if (pretty==2)
    set_pretty2_format (&cfmt);
  else
    set_standard_format (&cfmt);

  i=read_fmt_file ("fonts.fmt", styd, &cfmt);
  j=0;
  if (strlen(styf)>0) {
    strext(styf,styf,"fmt",1);
    j=read_fmt_file (styf, styd, &cfmt);
    if (j==0)  {
      printf ("\n+++ Cannot open file: %s\n", styf);
      return 0;
    }
    strcpy(cfmt.name, styf);
  }
  if (i || j) printf("\n");
  ops_into_fmt (&cfmt);

  make_font_list (&cfmt);
  sfmt=cfmt;
  dfmt=cfmt;
  return 1;
}


/* ----- tex_str: change string to take care of some tex-style codes --- */
/* Puts \ in front of ( and ) in case brackets are not balanced,
   interprets some TeX-type strings using ISOLatin1 encodings.
   Returns the length of the string as finally given out on paper.
   Also returns an estimate of the string width... */
int tex_str (str,s,wid)
char *str;
char s[];
float *wid;
{
  char *c;
  int base,add,n;
  char t[21];
  float w;

  c=str;
  strcpy(s,"");
  n=0;
  w=0;
  while (*c != '\0') {

    if ((*c=='(') || (*c==')'))           /* ( ) becomes \( \) */
      {sprintf(t, "\\%c", *c); strcat(s,t); w+=cwid('('); n++; }

    else if (*c=='\\') {                  /* backslash sequences */
      c++;
      if (*c=='\0') break;
      add=0;                              /* accented vowels */
      if (*c=='`')  add=1;
      if (*c=='\'') add=2;
      if (*c=='^')  add=3;
      if (*c=='"')  add=4;
      if (add) {
        c++;
        base=0;
        if (*c=='a') { base=340; if (add==4) add=5; }
        if (*c=='e')   base=350;
        if (*c=='i')   base=354;
        if (*c=='o') { base=362; if (add==4) add=5; }
        if (*c=='u')   base=371;
        if (*c=='A') { base=300; if (add==4) add=5; }
        if (*c=='E')   base=310;
        if (*c=='I')   base=314;
        if (*c=='O') { base=322; if (add==4) add=5; }
        if (*c=='U')   base=331;
        w+=cwid(*c);
        if (base)
          {sprintf(t,"\\%d",base+add-1); strcat(s,t); n+=1; }
        else
          {sprintf(t,"%c",*c); strcat(s,t); n+=1; }
      }

      else if (*c==' ')                               /* \-space */
        { strcat(s," "); w+=cwid(' '); n++; }

      else if (*c=='O')                               /* O-slash */
        { strcat(s,"\\330"); w+=cwid('O'); n++; }

      else if (*c=='o')                               /* o-slash */
        { strcat(s,"\\370"); w+=cwid('O'); n++; }

      else if((*c=='s')&&(*(c+1)=='s'))               /* sz */
        { c++; strcat(s,"\\337"); w+=cwid('s'); n++; }
      else if((*c=='a')&&(*(c+1)=='a'))               /* a-ring */
        { c++; strcat(s,"\\345"); w+=cwid('a'); n++; }
      else if((*c=='A')&&(*(c+1)=='A'))               /* A-ring */
        { c++; strcat(s,"\\305"); w+=cwid('A'); n++; }
      else if((*c=='a')&&(*(c+1)=='e'))               /* ae */
        { c++; strcat(s,"\\346"); w+=1.5*cwid('a'); n++; }
      else if((*c=='A')&&(*(c+1)=='E'))               /* AE */
        { c++; strcat(s,"\\306"); w+=1.5*cwid('A'); n++; }


      else if (*c=='c') {                           /* c-cedilla */
        c++;
        w+=cwid(*c);
        if      (*c=='C') { strcat(s,"\\307"); n++; }
        else if (*c=='c') { strcat(s,"\\347"); n++; }
        else              {sprintf(t,"%c",*c); strcat(s,t); n++; }
      }

      else if (*c=='~') {                           /* n-twiddle */
        c++;
        w+=cwid(*c);
        if      (*c=='N') { strcat(s,"\\321"); n++; }
        else if (*c=='n') { strcat(s,"\\361"); n++; }
		/*
		 *  cd: changed default behaviour in order to enable escaping of '~'
		 *  else              { sprintf(t,"%c",*c); strcat(s,t); n++; }
		 */
		else              { sprintf(t,"~%c",*c); strcat(s,t); n++; }
      }

      else                           /* \-something-else; pass through */
        {sprintf(t,"\\%c",*c); strcat(s,t); w+=cwid('A'); n++; }
    }

    else if (*c=='{')
      ;
    else if (*c=='}')
      ;

    else                             /* other characters: pass though */
      {
        sprintf(t,"%c",*c);
        strcat(s,t); n++;
        w+=cwid(*c);
      }

    c++;
  }
  *wid = w;

  return n;
}


/* ----- put_str: output a string in postscript ----- */
void put_str (str)
char *str;
{
  char s[801];
  float w;

  tex_str (str,s,&w);
  PUT1 ("%s", s);
}

/* ----- set_font ----- */
void set_font (fp,font,add_bracket)
FILE *fp;
struct FONTSPEC font;
int add_bracket;
{
  int i,fnum;

  fnum=-1;
  for (i=0;i<nfontnames;i++) {
    if (!strcmp(font.name,fontnames[i])) fnum=i;
  }
  if (fnum<0) {
    printf ("\n+++ Font \"%s\" not predefined; using first in list\n",
            font.name);
    fnum=0;
  }
  PUT3("%.1f %d F%d ", font.size, font.box, fnum)
  if (add_bracket) PUT0("(")
}

/* ----- set_font_str ----- */
void set_font_str (str,font)
char str[];
struct FONTSPEC font;
{
  int i,fnum;

  fnum=-1;
  for (i=0;i<nfontnames;i++) {
    if (!strcmp(font.name,fontnames[i])) fnum=i;
  }
  sprintf (str,"%.1f %d F%d ", font.size, font.box, fnum);
}


/* ----- check_margin: do horizontal shift if needed ---- */
void check_margin (float new_posx)
{
  float dif;

  dif=new_posx-posx;
  if (dif*dif<0.001) return;

  PUT1("%.2f 0 T\n", dif)
  posx=new_posx;
}


/* ----- epsf_title ------ */
void epsf_title (title,fnm)
char title[],fnm[];
{
  char *p,*q;

  p=title; q=fnm;
  while (*p != '\0') {
    if (*p == ' ')
      *q = '_';
    else
      *q = *p;
    p++; q++;
  }
  *q = '\0';
}

/* ----- close_output_file ------ */
void close_output_file ()
{
  int m;

  if (!file_open) return;

  if (interactive) printf ("(close %s)\n", outfnam);

  close_page(fout);
  close_ps (fout);
  fclose (fout);
  if (tunenum==0)
    printf ("+++ Warning: no tunes written to output file\n");
  m=get_file_size (outfnam);
  printf ("Output written on %s (%d page%s, %d title%s, %d byte%s)\n",
          outfnam,
          pagenum, pagenum==1 ? "" : "s",
          tunenum, tunenum==1 ? "" : "s",
          m, m==1 ? "" : "s");
  file_open=0;
  file_initialized=0;

}


/* ----- open_output_file ------ */
void open_output_file (fnam,tstr)
char fnam[],tstr[];
{

  if (!strcmp(fnam,outfnam)) return;

  if (file_open) close_output_file ();

  if (interactive) printf ("(open %s)\n", fnam);
  strcpy (outfnam, fnam);
  if ((fout = fopen (outfnam,"w")) == NULL)
    rx ("Cannot open output file ", outf);
  pagenum=0;
  tunenum=tnum1=tnum2=0;
  file_open=1;
  file_initialized=0;

}



/* ----- open_index_file ------- */
void open_index_file (fnam)
char fnam[];
{
  if (vb>=8) printf("Open index file \"%s\"\n", fnam);
  if ((findex = fopen (fnam,"w")) == NULL)
    rx ("Cannot open index file: ", fnam);

  index_initialized=0;

}

/* ----- close_index_file ------- */
void close_index_file ()
{

  if (vb>=8) printf("Close index file\n");

  close_index_page (findex);

  fclose (findex);

}

/* ----- add_to_text_block ----- */
void add_to_text_block (ln,add_final_nl)
char ln[];
int add_final_nl;
{
  char *c,*a;
  char word[81];
  int nt,nl;

  nt=ntxt;
  c=ln;

  for (;;) {
    while (*c==' ') c++;
    if (*c=='\0') break;
    a=word;
    while ((*c!=' ')&&(*c!='\0')&&(*c!='\n')) {
      nl=0;
      if ((*c=='\\')&&(*(c+1)=='\\')) {
        nl=1;
        c+=2;
        break;
      }
      *a=*c;
      c++;
      a++;

    }
    *a='\0';
    if (strlen(word)>MAXWLEN) {
      word[MAXWLEN-1]='\0';
      printf ("+++ Insanely long word truncated to %d chars: %s\n",
              MAXWLEN-1,word);
    }
    if (nt>=MAXNTEXT) {
      printf ("\n+++ Line %d: %s\n", linenum,ln);
      rx("Text overflow; increase MAXNTEXT and recompile.","");
    }
    if (strlen(word)>0) {
      strcpy(txt[nt],word);
      nt++;
    }
    if (nl) {
      strcpy(txt[nt],"$$NL$$");
      nt++;
    }
  }
  if (add_final_nl) {
    strcpy(txt[nt],"$$NL$$");
    nt++;
  }

  ntxt=nt;

}


/* ----- write_text_block ----- */
void write_text_block (fp,job)
FILE *fp;
int  job;
{
  int i,i1,i2,ntline,nc,mc,nbreak;
  float textwidth,ftline,ftline0,swfac,baseskip,parskip;
  float wwidth,wtot,spw;
  char str[81];

  if (ntxt<=0) return;

  baseskip = cfmt.textfont.size * cfmt.lineskipfac;
  parskip      = cfmt.textfont.size * cfmt.parskipfac;
  set_font_str (page_init,cfmt.textfont);

  /* estimate text widths.. ok for T-R, wild guess for other fonts */
  swfac=1.0;
  if (strstr(cfmt.textfont.name,"Times-Roman"))    swfac=1.00;
  if (strstr(cfmt.textfont.name,"Times-Bold"))     swfac=1.05;
  if (strstr(cfmt.textfont.name,"Helvetica"))      swfac=1.10;
  if (strstr(cfmt.textfont.name,"Helvetica-Bold")) swfac=1.15;
  if (strstr(cfmt.textfont.name,"Palatino"))       swfac=1.10;
  swfac=1.0;
  spw=cwid(' ');
  PUT1("/LF {0 %.1f rmoveto} bind def\n",-baseskip)

  /* output by pieces, separate at newline token */
  i1=0;
  while (i1<ntxt) {
    i2=-1;
    for (i=i1;i<ntxt;i++) if(!strcmp(txt[i],"$$NL$$")) {i2=i; break; }
    if (i2==-1) i2=ntxt;
    bskip(baseskip);

    if (job==OBEYLINES) {
      PUT0("0 0 M (")
      for (i=i1;i<i2;i++) {
        tex_str(txt[i],str,&wwidth);
        PUT1("%s ",str);
      }
      PUT0(") rshow\n")
    }

    else if (job==OBEYCENTER) {
      PUT1("%.1f 0 M (",cfmt.staffwidth/2)
      for (i=i1;i<i2;i++) {
        tex_str(txt[i],str,&wwidth);
        PUT1("%s",str)
        if (i<i2-1) PUT0(" ")
      }
      PUT0(") cshow\n")
    }

    else {
      PUT0("0 0 M mark\n")
      nc=0;
      mc=-1;
      wtot=-spw;
      for (i=i2-1;i>=i1;i--) {
        mc+=tex_str(txt[i],str,&wwidth)+1;
        wtot+=wwidth+spw;
        nc+=strlen(str)+2;
        if (nc>=72) {nc=0; PUT0("\n") }
        PUT1 ("(%s)",str);
      }
      if (job==RAGGED)
        PUT1(" %.1f P1\n",cfmt.staffwidth)
      else
        PUT1(" %.1f P2\n",cfmt.staffwidth)
          /* first estimate: (total textwidth)/(available width) */
          textwidth=wtot*swfac*cfmt.textfont.size;
      if (strstr(cfmt.textfont.name,"Courier"))
        textwidth=0.60*mc*cfmt.textfont.size;
      ftline0=textwidth/cfmt.staffwidth;
      /* revised estimate: assume some chars lost at each line end */
      nbreak=ftline0;
      textwidth=textwidth+5*nbreak*cwid('a')*swfac*cfmt.textfont.size;
      ftline=textwidth/cfmt.staffwidth;
      ntline=ftline+1.0;
      if (vb>=10)
        printf("first estimate %.2f, revised %.2f\n", ftline0,ftline);
      if (vb>=10)
        printf("Output %d word%s, about %.2f lines (fac %.2f)\n",
               i2-i1, i2-i1==1?"":"s", ftline,swfac);
      bskip((ntline-1)*baseskip);
    }

    buffer_eob (fp);
    /* next line to allow pagebreak after each text "line" */
    /* if (!epsf && !within_tune) write_buffer(fp); */
    i1=i2+1;
  }
  bskip(parskip);
  buffer_eob (fp);
  /* next line to allow pagebreak after each paragraph */
  if (!epsf && !within_tune) write_buffer(fp);
  strcpy (page_init,"");
  return;
}



/* ----- put_words ------- */
void put_words (fp)
FILE *fp;
{
  int i,nw,n;
  char str[81];
  char *p,*q;

  set_font (fp,cfmt.wordsfont, 0);
  set_font_str (page_init,cfmt.wordsfont);

  n=0;
  for (i=0;i<ntext;i++) if (text_type[i]==TEXT_W) n++;
  if (n==0) return;

  bskip(cfmt.wordsspace);
  for (i=0;i<ntext;i++) {
    if (text_type[i]==TEXT_W) {
      bskip(cfmt.lineskipfac*cfmt.wordsfont.size);
      p=&text[i][0];
      q=&str[0];
      if (isdigit(text[i][0])) {
        while (*p != '\0') {
          *q=*p;
          q++;
          p++;
          if (*p==' ') break;
          if (*(p-1)==':') break;
          if (*(p-1)=='.') break;
        }
        if (*p==' ') p++;
      }
      *q='\0';

      /* permit page break at empty lines or stanza start */
      nw=nwords(text[i]);
      if ((nw==0) || (strlen(str)>0)) buffer_eob(fp);

      if (nw>0) {
        if (strlen(str)>0) {
          PUT0("45 0 M (")
          put_str (str);
          PUT0(") lshow\n")
        }
        if (strlen(p)>0) {
          PUT0("50 0 M (")
          put_str (p);
          PUT0(") rshow\n")
        }
      }
    }
  }

  buffer_eob (fp);
  strcpy (page_init,"");

}

/* ----- put_text ------- */
void put_text (fp, type, str)
FILE *fp;
int type;
char str[];
{
  int i,n;
  float baseskip,parskip;

  n=0;
  for (i=0;i<ntext;i++) if (text_type[i]==type) n++;
  if (n==0) return;

  baseskip = cfmt.textfont.size * cfmt.lineskipfac;
  parskip  = cfmt.textfont.size * cfmt.parskipfac;
  PUT0("0 0 M\n")
  ntxt=0;
  add_to_text_block(str,0);
  for (i=0;i<ntext;i++) {
    if (text_type[i]==type) add_to_text_block(text[i],1);
  }
  write_text_block (fp,RAGGED);
  buffer_eob (fp);

}

/* ----- put_history ------- */
void put_history (fp)
FILE *fp;
{
  int i,ok;
  float baseskip,parskip;

  set_font (fp, cfmt.textfont,0);
  set_font_str (page_init,cfmt.textfont);
  baseskip = cfmt.textfont.size * cfmt.lineskipfac;
  parskip  = cfmt.textfont.size * cfmt.parskipfac;

  bskip(cfmt.textspace);

  if (strlen(info.rhyth)>0) {
    bskip(baseskip);
    PUT0("0 0 M (Rhythm: ")
    put_str (info.rhyth);
    PUT0(") show\n")
    bskip(parskip);
  }

  if (strlen(info.book)>0) {
    bskip(0.5*CM);
    PUT0("0 0 M (Book: ")
    put_str (info.book);
    PUT0(") show\n")
    bskip(parskip);
  }

  if (strlen(info.src)>0) {
    bskip(0.5*CM);
    PUT0("0 0 M (Source: ")
    put_str (info.src);
    PUT0(") show\n")
    bskip(parskip);
  }

  put_text (fp, TEXT_D, "Discography: ");
  put_text (fp, TEXT_N, "Notes: ");
  put_text (fp, TEXT_Z, "Transcription: ");

  ok=0;
  for (i=0;i<ntext;i++) {
    if (text_type[i]==TEXT_H) {
      bskip(0.5*CM);
      PUT0("0 0 M (")
      put_str (text[i]);
      PUT0(") show\n")
      ok=1;
    }
  }
  if (ok) bskip(parskip);
  buffer_eob (fp);
  strcpy (page_init,"");

}


/* ----- write_inside_title  ----- */
void write_inside_title (fp)
FILE *fp;
{
  char t[201];

  if      (numtitle==1) strcpy (t,info.title);
  else if (numtitle==2) strcpy (t,info.title2);
  else if (numtitle==3) strcpy (t,info.title3);
  if (vb>15) printf ("write inside title <%s>\n", t);
  if (strlen(t)==0) return;

  bskip (cfmt.subtitlefont.size+0.2*CM);
  set_font (fp, cfmt.subtitlefont, 0);

  if (cfmt.titlecaps) cap_str(t);
  PUT0(" (")
  put_str (t);
  if (cfmt.titleleft) PUT0(") 0 0 M rshow\n")
  else PUT1(") %.1f 0 M cshow\n", cfmt.staffwidth/2)
  bskip (cfmt.musicspace+0.2*CM);

}


/* ----- write_tunetop ----- */
void write_tunetop(fp)
FILE *fp;
{

  PUT2("\n\n%% --- tune %d %s\n", tunenum+1, info.title)
  if (!epsf) bskip (cfmt.topspace);
}


/* ----- write_tempo ----- */
void write_tempo(fp,tempo,meter)
FILE *fp;
char tempo[];
struct METERSTR meter;
{
  char *r, *q;
  char text[STRL];
  int top,bot,value,len,i,err,fac,dig,div;
  struct SYMBOL s;
  float stem,dotx,doty,sc,dx;

  if (vb>15) printf ("write tempo <%s>\n", info.tempo);
  r=tempo;
  set_font (fp,cfmt.tempofont,0);
  PUT0(" 18 0 M\n")

  for (;;) {

    while (*r==' ') r++;                    /* skip blanks */
    if (*r=='\0') break;

    if (*r=='"') {                          /* write string */
      r++;
      q=text;
      while (*r!='"' && *r!='\0') { *q=*r; r++; q++; }
      if (*r=='"') r++;
      *q='\0';
      if (strlen(text)>0) {
        PUT0("6 0 rmoveto (")
        put_str (text);
        PUT0(") rshow 12 0 \n")
      }
    }

    else {                                  /* write tempo denotation */
      q=text;
      while (*r!=' ' && *r!='\0') { *q=*r; r++; q++; }
      *q='\0';

      q=text;
      len=QUARTER;
      value=0;
      err=0;
      if (strchr(q,'=')) {
        if (*q=='C' || *q=='c') {
          q++;
          len=meter.dlen;
          div=0;
          if (*q=='/') { div=1; q++; }
          fac=0;
          while (isdigit(*q)) { dig=*q-'0'; fac=10*fac+dig; q++; }

          if (div) {
            if (fac==0) fac=2;
            if (len%fac) printf ("Bad length divisor in tempo: %s", text);
            len=len/fac;
          }
          else
            if (fac>0) len=len*fac;
          if (*q!='=') err=1;
          q++;
          if (!isdigit(*q)) err=1;
          sscanf(q,"%d", &value);
        }
        else if (isdigit(*q)) {
          sscanf(q,"%d/%d=%d", &top,&bot,&value);
          len=(BASE*top)/bot;
        }
        else err=1;
      }
      else {
        if (isdigit(*q))
          sscanf(q,"%d", &value);
        else err=1;
      }

      if (err)                              /* draw note=value */
        printf("\n+++ invalid tempo specifier: %s\n", text);
      else {
        s.len=len;
        identify_note (&s,r);
        sc=0.55*cfmt.tempofont.size/10.0;
        PUT2("gsave %.2f %.2f scale 15 3 rmoveto currentpoint\n", sc,sc)
        if (s.head==H_OVAL)  PUT0("HD")
        if (s.head==H_EMPTY) PUT0("Hd")
        if (s.head==H_FULL)  PUT0("hd")
        dx=4.0;
        if (s.dots) {
          dotx=8; doty=0;
          if (s.flags>0) dotx=dotx+4;
          if (s.head==H_EMPTY) dotx=dotx+1;
          if (s.head==H_OVAL)  dotx=dotx+2;
          for (i=0;i<s.dots;i++) {
            PUT2(" %.1f %.1f dt", dotx, doty)
            dx=dotx;
            dotx=dotx+3.5;
          }
        }
        stem=16.0;
        if (s.flags>1) stem=stem+3*(s.flags-1);
        if (s.len<WHOLE) PUT1(" %.1f su",stem)
        if (s.flags>0) PUT2(" %.1f f%du",stem,s.flags)
        if ((s.flags>0) && (dx<6.0)) dx=6.0;
        dx=(dx+18)*sc;
        PUT2(" grestore %.2f 0 rmoveto ( = %d) rshow\n", dx,value)
      }
    }
  }
}


/* ----- write_inside_tempo  ----- */
void write_inside_tempo (fp)
FILE *fp;
{

  bskip (cfmt.partsfont.size);
  write_tempo(fp,info.tempo,voice[ivc].meter);
  bskip (0.1*CM);
}

/* ----- write_heading  ----- */
void write_heading (fp)
FILE *fp;
{
  float lwidth,down1,down2;
  int i,ncl;
  char t[201];

  lwidth=cfmt.staffwidth;

  /* write the main title */
  bskip (cfmt.titlefont.size+cfmt.titlespace);
  set_font (fp,cfmt.titlefont,1);
  if (cfmt.withxrefs) PUT1("%d. ", xrefnum)
  strcpy (t,info.title);
  if (cfmt.titlecaps) cap_str(t);
  put_str (t);
  if (cfmt.titleleft) PUT0(") 0 0 M rshow\n")
  else PUT1(") %.1f 0 M cshow\n", lwidth/2)

  /* write second title */
  if (numtitle>=2) {
    bskip (cfmt.subtitlespace+cfmt.subtitlefont.size);
    set_font (fp,cfmt.subtitlefont,1);
    strcpy (t,info.title2);
    if (cfmt.titlecaps) cap_str(t);
    put_str (t);
    if (cfmt.titleleft) PUT0(") 0 0 M rshow\n")
    else PUT1(") %.1f 0 M cshow\n", lwidth/2)
  }

  /* write third title */
  if (numtitle>=3) {
    bskip (cfmt.subtitlespace+cfmt.subtitlefont.size);
    set_font (fp,cfmt.subtitlefont,1);
    strcpy (t,info.title3);
    if (cfmt.titlecaps) cap_str(t);
    put_str (t);
    if (cfmt.titleleft) PUT0(") 0 0 M rshow\n")
    else PUT1(") %.1f 0 M cshow\n", lwidth/2)
  }

  /* write composer, origin */
  if ((info.ncomp>0) || (strlen(info.orig)>0)) {
    set_font (fp,cfmt.composerfont,0);
    bskip(cfmt.composerspace);
    ncl=info.ncomp;
    if ((strlen(info.orig)>0) && (ncl<1)) ncl=1;
    for (i=0;i<ncl;i++) {
      bskip(cfmt.composerfont.size);
      PUT1("%.1f 0 M (", lwidth)
      put_str (info.comp[i]);
      if ((i==ncl-1)&&(strlen(info.orig)>0)) {
        put_str (" (");
        put_str (info.orig);
        put_str (")");
      }
      PUT0 (") lshow\n");
    }
    down1=cfmt.composerspace+cfmt.musicspace+ncl*cfmt.composerfont.size;
  }
  else {
    bskip(cfmt.composerfont.size+cfmt.composerspace);
    down1=cfmt.composerspace+cfmt.musicspace+cfmt.composerfont.size;
  }
  bskip (cfmt.musicspace);

  /* decide whether we need extra shift for parts and tempo */
  down2=cfmt.composerspace+cfmt.musicspace;
  if (strlen(info.parts)>0) down2=down2+cfmt.partsspace+cfmt.partsfont.size;
  if (strlen(info.tempo)>0) down2=down2+cfmt.partsspace+cfmt.partsfont.size;
  if (down2>down1) bskip (down2-down1);

  /* write tempo and parts */
  if (strlen(info.parts)>0 || strlen(info.tempo)>0) {

    if (strlen(info.tempo)>0) {
      bskip (-0.2*CM);
      PUT1 (" %.2f 0 T ", cfmt.indent*cfmt.scale)
      write_tempo(fp, info.tempo, default_meter);
      PUT1 (" %.2f 0 T ", -cfmt.indent*cfmt.scale)
      bskip (-cfmt.tempofont.size);
    }

    if (strlen(info.parts)>0) {
      bskip (-cfmt.partsspace);
      set_font (fp,cfmt.partsfont,0);
      PUT0("0 0 M (")
      put_str (info.parts);
      PUT0(") rshow\n")
      bskip (cfmt.partsspace);
    }

    if (strlen(info.tempo)>0) bskip (cfmt.tempofont.size+0.3*CM);

  }


}

/* ----- write_parts  ----- */
void write_parts (fp)
FILE *fp;
{
  if (strlen(info.parts)>0) {
    bskip (cfmt.partsfont.size);
    set_font (fp, cfmt.partsfont,0);
    PUT0("0 0 M (")
    put_str (info.parts);
    PUT0(") rshow\n")
    bskip(cfmt.partsspace);
  }
}

