/*
 *  abc2ps: a program to typeset tunes written in abc format using PostScript
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
 *
 *  The author can be contacted as follows:
 *
 *  Michael Methfessel
 *  methfessel@ihp-ffo.de
 *  Institute for Semiconductor Physics, PO Box 409,
 *  D-15204 Frankfurt (Oder), Germany
 */

/* Main program abc2ps.c */

#include <string.h>

#include "abc2ps.h"
#include "subs.h"
#include "format.h"
#include "util.h"
#include "parse.h"
#include "music.h"


/* ----- definitions of global variables ----- */

int db=DEBUG_LV;                  /* debug control */
int maxSyms,maxVc;                /* for malloc */
struct ISTRUCT info,default_info; /* information fields */
char lvoiceid[233];               /* string from last V: line */
int  nvoice,mvoice;               /* number of voices defd, nonempty */
int  ivc;                         /* current voice */
int  ivc0;                        /* top nonempty voice */
int ixpfree;                      /* first free element in xp array */
struct METERSTR default_meter;    /* data to specify the meter */
struct KEYSTR default_key;        /* data to specify the key */
                          /* things to alloc: */
struct SYMBOL  *sym;              /* symbol list */
struct SYMBOL  **symv;            /* symbols for voices */
struct XPOS    *xp;               /* shared horizontal positions */
struct VCESPEC *voice;            /* characteristics of a voice */
struct SYMBOL  **sym_st;          /* symbols a staff start */
int            *nsym_st;

int halftones;                    /* number of halftones to transpose by */

float f0p,f5p,f1p,f0x,f5x,f1x;            /*   mapping fct */
float lnnp,bnnp,fnnp,lnnx,bnnx,fnnx;      /*   note-note spacing */
float lbnp,bbnp,rbnp,lbnx,bbnx,rbnx;      /*   bar-note spacing */
float lnbp,bnbp,rnbp,lnbx,bnbx,rnbx;      /*   note-bar spacing */


char wpool[NWPOOL];            /* pool for vocal strings */
int nwpool,nwline;             /* globals to handle wpool */

struct SYMBOL zsym;            /* symbol containing zeros */

struct FORMAT sfmt;                    /* format after initialization */
struct FORMAT dfmt;                    /* format at start of tune */
struct FORMAT cfmt;                    /* current format for output */

char fontnames[50][STRLFMT];           /* list of needed fonts */
int  nfontnames;

char txt[MAXNTEXT][MAXWLEN];           /* for output of text */
int  ntxt;

char vcselstr[101];            /* string for voice selection */
char mbf[501];                 /* mini-buffer for one line */
char buf[BUFFSZ];              /* output buffer.. should hold one tune */
int nbuf;                      /* number of bytes buffered */
float bposy;                   /* current position in buffered data */
int   ln_num;                  /* number of lines in buffer */
float ln_pos[BUFFLN];          /* vertical positions of buffered lines */
int   ln_buf[BUFFLN];          /* buffer location of buffered lines */
int   use_buffer;              /* 1 if lines are being accumulated */

char text [NTEXT][STRL];       /* pool for history, words, etc. lines */
int text_type[NTEXT];          /* type of each text line */
int ntext;                     /* number of text lines */
char page_init[201];           /* initialization string after page break */
int do_mode;                   /* control whether to do index or output */
char escseq[81];               /* escape sequence string */
int linenum;                   /* current line number in input file */
int tunenum;                   /* number of current tune */
int tnum1,tnum2;
int numtitle;                  /* how many titles were read */
int mline;                     /* number music lines in current tune */
int nsym;                      /* number of symbols in line */
int nsym0;                     /* nsym at start of parsing a line */
int pagenum;                   /* current page in output file */
int writenum;                  /* calls to write_buffer for each one tune */
int xrefnum;                   /* xref number of current tune */
int do_meter, do_indent;       /* how to start next block */

int index_pagenum;             /* for index file */
float index_posx, index_posy;
int index_initialized;

char gch[201];                   /* guitar chord string */
int bagpipe;                     /* switch for HP mode */
int within_tune, within_block;   /* where we are in the file */
int do_this_tune;                /* are we typesetting the current one ? */
float posx,posy;                 /* overall scale, position on page */
int barinit;                     /* carryover bar number between parts */

char *p, *p0;                    /* global pointers for parsing music line */

int word,slur;                   /* variables used for parsing... */
int last_note,last_real_note;
int pplet,qplet,rplet;
int carryover;                   /* for interpreting > and < chars */
int ntinext,tinext[MAXHD];       /* for chord ties */

struct ENDINGS ending[20];       /* where to draw endings */
int num_ending;                  /* number of endings to draw */
int mes1,mes2;                   /* to count measures in an ending */

int slur1[20],slur2[20];         /* needed for drawing slurs */
int overfull;                    /* flag if staff overfull */
int do_words;                    /* flag if staff has words under it */

int vb, verbose;                 /* verbosity, global and within tune */
int in_page=0;

                                 /* switches modified by flags: */
int gmode;                         /* switch for glue treatment */
int include_xrefs;                 /* to include xref numbers in title */
int one_per_page;                  /* new page for each tune ? */
int pagenumbers;                   /* write page numbers ? */
int write_history;                 /* write history and notes ? */
int interactive;                   /* interactive mode ? */
int help_me;                       /* need help ? */
int select_all;                    /* select all tunes ? */
int epsf;                          /* for EPSF postscript output */
int choose_outname;                /* 1 names outfile w. title/fnam */
int break_continues;               /* ignore continuations ? */
int search_field0;                 /* default search field */
int pretty;                        /* for pretty but sprawling layout */
int bars_per_line;                 /* bars for auto linebreaking */
int continue_lines;                /* flag to continue all lines */
int landscape;                     /* flag for landscape output */
int barnums;                       /* interval for bar numbers */
int make_index;                    /* write index file ? */
int make_ps;                       /* write postscript file ? */
float alfa_c;                      /* max compression allowed */
float scalefac;                    /* scale factor for symbol size */
float lmargin;                     /* left margin */
float swidth;                      /* staff width */
float staffsep,dstaffsep;          /* staff separation */
float strict1,strict2;             /* 1stave, mstave strictness */
char transpose[21];                /* target key for transposition */
char au_qstr[21];                  /* code for au quality selection */


float alfa_last,beta_last;         /* for last short short line.. */

char in_file[MAXINF][STRL1];     /* list of input file names */
int  ninf;                       /* number of input file names */
FILE *fin;                       /* for input file */

char outf[STRL1];                /* output file name */
char outfnam[STRL1];             /* internal file name for open/close */
char styf[STRL1];                /* layout style file name */
char styd[STRL1];                /* layout style directory */
char infostr[STRL1];             /* title string in PS file */

int  file_open;                  /* for output file */
int  file_initialized;           /* for output file */
FILE *fout,*findex;              /* for output file */
int nepsf;                       /* counter for epsf output files */

char sel_str[MAXINF][STRL1];     /* list of selector strings */
int  s_field[MAXINF];            /* type of selection for each file */
int  psel[MAXINF];               /* pointers from files to selectors */

int temp_switch;



/* ----- start of main ------ */
int main(argc, argv)
int argc;
char *argv[];
{

  char aaa[501],bbb[501],ccc[501],ext[41];
  char xref_str[STRL1], pat[30][STRL1];
  char *urgv[100];
  int isel,j,npat,search_field,urgc,retcode,rc1;

  /* ----- set default options and parse arguments ----- */

  maxSyms = 800;
  maxVc   = 4;

  init_ops (1);
  retcode=parse_args (argc, argv);
  if (retcode) exit (1);
  if (interactive || (do_mode==DO_OUTPUT))
    printf ("This is abc2ps, version %s.%s (%s)\n", VERSION, REVISION, VDATE);

  alloc_structs ();

  /* ----- set the page format ----- */
  nfontnames=0;
  if (!set_page_format()) exit (3);
  if (help_me==2) {
    print_format(cfmt);
    exit (0);
  }

  /* ----- help printout  ----- */
  if (argc<=1) help_me=1;
  if (help_me==1) {
    write_help ();
    exit (0);
  }

  if ((ninf==0) && (!interactive)) rx ("No input file specified", "");

  isel=psel[ninf-1];
  search_field0=s_field[isel];   /* default for interactive mode */
  if (epsf) cutext(outf);

  /* ----- initialize ----- */
  zero_sym();
  pagenum=0;
  tunenum=tnum1=tnum2=0;
  verbose=0;
  file_open=file_initialized=0;
  nepsf=0;
  bposy=0;
  posx=cfmt.leftmargin;
  posy=cfmt.pageheight-cfmt.topmargin;

  strcpy(page_init, "");
  strcpy (bbb,"");
  for (j=0;j<ninf;j++) {strcat(bbb,in_file[j]); strcat(bbb," ");}

  if ((do_mode == DO_OUTPUT) && make_index) open_index_file (INDEXFILE);


  /* ----- start infinite loop for interactive mode ----- */
  for (;;) {

    if (interactive) {
      printf ("\nSelect tunes: ");
/*|   gets (aaa); |*/
/*|   fgets (aaa, sizeof(aaa), stdin); |*/
      getline(aaa,500,stdin);
      if (isblankstr(aaa)) break;
      sscanf(aaa,"%s",ccc);
      if (ccc[0]=='?') {
        printf ("%s\n", bbb);
        continue;
      }
      if (ccc[0]=='*') {
        strcat (bbb,strchr(aaa,'*')+1);
        strcpy (aaa,bbb);
        printf ("%s\n", aaa);
      }
      if (ccc[0]=='!') {
        strcpy (bbb,"");
        for (j=0;j<ninf;j++) {
          strcat (bbb,in_file[j]);
          strcat (bbb," ");
        }
        strcat (bbb,strchr(aaa,'!')+1);
        strcpy (aaa,bbb);
        printf ("%s\n", aaa);
      }
      strcpy(bbb,aaa);
      urgc=make_arglist (aaa, urgv);
      if (!strcmp(urgv[1],"q"))    break;
      if (!strcmp(urgv[1],"quit")) break;
      init_ops(0);
      retcode=parse_args (urgc, urgv);
      if (retcode) continue;
      ops_into_fmt (&cfmt);
      if (make_ps) {
        rc1=set_page_format();
        if (rc1==0) continue;
      }
      if (epsf) cutext(outf);
      if (help_me==1) {
        write_help();
        continue;
      }
      if (help_me==2) {
        print_format(cfmt);
        continue;
      }

    }

    /* ----- loop over files in list ---- */
    if (ninf==0) printf ("++++ No input files\n");
    for (j=0;j<ninf;j++) {
      getext (in_file[j],ext);
      /*  skip .ps and .eps files */
      if ((!strcmp(ext,"ps"))||(!strcmp(ext,"eps"))) continue;

      if ((fin = fopen (in_file[j],"r")) == NULL) {
        if (!strcmp(ext,"")) strext (in_file[j],in_file[j],"abc",1);
        if ((fin = fopen (in_file[j],"r")) == NULL) {
          printf ("++++ Cannot open input file: %s\n", in_file[j]);
          continue;
        }
      }
      isel=psel[j];
      search_field=s_field[isel];
      npat=rehash_selectors (sel_str[isel], xref_str, pat);
      dfmt=sfmt;
      strcpy(infostr, in_file[j]);

      if (do_mode==DO_INDEX) {
        printf ("%s:\n", in_file[j]);
        do_index (fin,xref_str,npat,pat,select_all,search_field);
      }

      else {
        if (!epsf) {
          strext (outf, outf, "ps", 1);
          if (choose_outname) strext (outf, in_file[j], "ps", 1);
          open_output_file(outf,in_file[j]);
        }
        printf ("%s: ", in_file[j]);
        if ((vb>=3) || interactive) printf ("\n");
        process_file (fin,fout,xref_str,npat,pat,select_all,search_field);
        printf ("\n");
      }

    }
    if (!interactive) break;
  }

  if ((!interactive) && (do_mode==DO_INDEX))
    printf ("Selected %d title%s of %d\n", tnum1, tnum1==1?"":"s", tnum2);

  close_output_file ();

  if ((do_mode == DO_OUTPUT) && make_index) close_index_file ();
  exit (0);
}






