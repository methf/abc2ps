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

#include <time.h>

#include "abc2ps.h"
#include "syms.h"

#include "pssubs.h"

/*  subroutines for postscript output  */


/* ----- level1_fix: special defs for level 1 Postscript ------- */
void level1_fix (fp)
FILE *fp;
{

  fprintf (fp,
           "/selectfont { exch findfont exch dup   %% emulate level 2 op\n"
           "  type /arraytype eq {makefont}{scalefont} ifelse setfont\n"
           "} bind def\n"
           );

  /* fix to define ISOLatin1Encoding for ps level 1 (david weisman) */

  fprintf (fp,
           "/ISOLatin1Encoding\n"
           "[ /.notdef    /.notdef    /.notdef    /.notdef\n"
           "/.notdef    /.notdef    /.notdef    /.notdef\n"
           "/.notdef    /.notdef    /.notdef    /.notdef\n"
           "/.notdef    /.notdef    /.notdef    /.notdef\n"
           "/.notdef    /.notdef    /.notdef    /.notdef\n"
           "/.notdef    /.notdef    /.notdef    /.notdef\n"
           "/.notdef    /.notdef    /.notdef    /.notdef\n"
           "/.notdef    /.notdef    /.notdef    /.notdef\n"
           "/space      /exclam     /quotedbl   /numbersign\n"
           "/dollar     /percent    /ampersand  /quoteright\n"
           "/parenleft  /parenright /asterisk   /plus\n"
           "/comma      /hyphen     /period     /slash\n"
           "/zero       /one        /two        /three\n"
           "/four       /five       /six        /seven\n"
           "/eight      /nine       /colon      /semicolon\n"
           "/less       /equal      /greater    /question\n"
           "/at         /A/B/C/D/E/F/G/H/I/J/K/L/M/N/O/P/Q/R/S/T/U/V/W/X/Y/Z /bracketleft\n"
           "/backslash  /bracketright/asciicircum/underscore\n"
           "/quoteleft  /a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v/w/x/y/z /braceleft\n"
           "/bar        /braceright/asciitilde  /.notdef\n"
           "/.notdef    /.notdef    /.notdef    /.notdef\n"
           "/.notdef    /.notdef    /.notdef    /.notdef\n"
           "/.notdef    /.notdef    /.notdef    /.notdef\n"
           "/.notdef    /.notdef    /.notdef    /.notdef\n"
           "/dotlessi   /grave      /acute      /circumflex\n"
           "/tilde      /macron     /breve      /dotaccent\n"
           "/dieresis   /.notdef    /ring       /cedilla\n"
           "/.notdef    /hungarumlaut/ogonek    /caron\n"
           "/space      /exclamdown /cent       /sterling\n"
           "/currency   /yen        /brokenbar  /section\n"
           "/dieresis   /copyright  /ordfeminine/guillemotleft\n"
           "/logicalnot /hyphen     /registered /macron\n"
           "/degree     /plusminus  /twosuperior/threesuperior\n"
           "/acute      /mu         /paragraph  /periodcentered\n"
           "/cedilla    /onesuperior/ordmasculine/guillemotright\n"
           "/onequarter /onehalf    /threequarters/questiondown\n"
           "/Agrave     /Aacute     /Acircumflex/Atilde\n"
           "/Adieresis  /Aring      /AE         /Ccedilla\n"
           "/Egrave     /Eacute     /Ecircumflex/Edieresis\n"
           "/Igrave     /Iacute     /Icircumflex/Idieresis\n"
           "/Eth        /Ntilde     /Ograve     /Oacute\n"
           "/Ocircumflex/Otilde     /Odieresis  /multiply\n"
           "/Oslash     /Ugrave     /Uacute     /Ucircumflex\n"
           "/Udieresis  /Yacute     /Thorn      /germandbls\n"
           "/agrave     /aacute     /acircumflex/atilde\n"
           "/adieresis  /aring      /ae         /ccedilla\n"
           "/egrave     /eacute     /ecircumflex/edieresis\n"
           "/igrave     /iacute     /icircumflex/idieresis\n"
           "/eth        /ntilde     /ograve     /oacute\n"
           "/ocircumflex/otilde     /odieresis  /divide\n"
           "/oslash     /ugrave     /uacute     /ucircumflex\n"
           "/udieresis  /yacute     /thorn      /ydieresis\n"
           "] def\n\n"
           );

  /* end fix to define ISOLatin1Encoding for ps level 1 */


}

/* ----- init_ps ------- */
void init_ps (FILE *fp, char str[], int is_epsf, float bx1, float by1,
              float bx2, float by2)
{
  time_t  ltime;
  char    tstr[41];
  int i;

  if (is_epsf) {
    if (vb>=8) printf("Open EPS file with title \"%s\"\n", str);
    fprintf (fp, "%%!PS-Adobe-3.0 EPSF-3.0\n");
    fprintf (fp, "%%%%BoundingBox: %.0f %.0f %.0f %.0f\n",
             bx1,by1,bx2,by2);
  }
  else {
    if (vb>=8) printf("Open PS file with title \"%s\"\n", str);
    fprintf (fp, "%%!PS-Adobe-3.0\n");
  }

  /* Title */
  fprintf (fp, "%%%%Title: %s\n", str);

  /* CreationDate */
  time(&ltime);
  strcpy (tstr,  ctime(&ltime));
  tstr[24]='\0';
  tstr[16]='\0';
  fprintf (fp, "%%%%Creator: abc2ps %s.%s\n", VERSION, REVISION);
  fprintf (fp, "%%%%CreationDate: %s %s\n", tstr+4,tstr+20);

  if (PS_LEVEL == 2) fprintf (fp, "%%%%LanguageLevel: 2\n");
  fprintf (fp, "%%%%EndComments\n\n");

  if (is_epsf)
    fprintf (fp, "gsave /origstate save def mark\n100 dict begin\n\n");

  fprintf (fp, "%%%%BeginSetup\n");
  if (PS_LEVEL < 2) level1_fix (fp);
  if (vb>=7) printf ("\nDefining ISO fonts in file header:\n");
  for (i=0;i<nfontnames;i++) {
    define_font (fp,fontnames[i],i);
    if (vb>=7) printf ("   F%d   %s\n", i,fontnames[i]);
  }
  define_symbols (fp);
  fprintf (fp, "\n0 setlinecap 0 setlinejoin 0.8 setlinewidth\n");
  fprintf (fp, "\n/T {translate} bind def\n/M {moveto} bind def\n");
  fprintf (fp, "%%%%EndSetup\n");
  file_initialized=1;
}

/* ----- close_ps ------- */
void close_ps (fp)
FILE *fp;
{
  if(vb>=8) printf ("closing PS file\n");
  fprintf (fp,
           "%%EOF\n\n"
           );
}

/* ----- init_page: initialize postscript page ----- */
void init_page (fp)
FILE *fp;
{

  if (vb>=10) printf ("init_page called; in_page=%d\n", in_page);
  if (in_page) return;

  if (!file_initialized) {
    if (vb>=10) printf ("file not yet initialized; do it now\n");
    init_ps (fp,infostr, 0,0.0,0.0,0.0,0.0);
  }
  in_page=1;
  pagenum++;

  if (verbose==0) ;
  else if (verbose==1) printf ("[%d] ", pagenum);
  else if (verbose==2) printf ("[%d] ", pagenum);
  else                 printf ("[%d]\n", pagenum);
  fflush (stdout);
  fprintf (fp,
           "\n%% --- page %d\n"
           "%%%%Page: %d %d\n"
           "%%%%BeginPageSetup\n",
           pagenum, pagenum, pagenum);

  if (cfmt.landscape) fprintf(fp,"%%%%PageOrientation: Landscape\n");
  fprintf(fp,"gsave ");
  if (cfmt.landscape) fprintf(fp,"90 rotate 0 %.1f translate ",
                              -cfmt.pageheight);
  fprintf (fp,"%.2f %.2f translate\n",
           cfmt.leftmargin, cfmt.pageheight-cfmt.topmargin);
  fprintf (fp, "%%%%EndPageSetup\n");


  /* write page number */
  if (pagenumbers) {
    fprintf (fp, "/Times-Roman 12 selectfont ");

    /* page numbers always at right */
    fprintf(fp, "%.1f %.1f moveto (%d) /bx false def lshow\n",
            cfmt.staffwidth, cfmt.topmargin-30.0, pagenum);

    /* page number right/left for odd/even pages */
/*|      if (pagenum%2==0)   |*/
/*|        fprintf(fp, "%.1f %.1f moveto (%d) show\n",  |*/
/*|                0.0, cfmt.topmargin-30.0, pagenum);  |*/
/*|      else  |*/
/*|        fprintf(fp, "%.1f %.1f moveto (%d) lshow\n",  |*/
/*|                cfmt.staffwidth, cfmt.topmargin-30.0, pagenum);  |*/

  }

}

/* ----- init_index_page ----- */
void init_index_page (fp)
FILE *fp;
{
  float hsize;

  index_pagenum++;

  fprintf (fp,
           "\n%% --- page %d\n"
           "%%%%Page: %d %d\n"
           "%%%%BeginPageSetup\n",
           index_pagenum, index_pagenum, index_pagenum);

  if (cfmt.landscape) fprintf(fp,"%%%%PageOrientation: Landscape\n");
  if (cfmt.landscape) fprintf(fp,"90 rotate 0 %.1f translate ",
                              -cfmt.pageheight);
  fprintf (findex, "gsave\n");
  fprintf (fp, "%%%%EndPageSetup\n\n");

  index_posx=cfmt.leftmargin;
  index_posy=cfmt.pageheight-cfmt.topmargin;
  /* extra space at top.. */

  index_posy = index_posy - 2*cfmt.indexfont.size;

  /* write heading */
  if (index_pagenum == 1) {
    hsize = 1.5*cfmt.indexfont.size;
    index_posy = index_posy - hsize;
    fprintf (findex, "%.1f %d F1 \n", hsize,cfmt.indexfont.box);
    fprintf (findex, "%.2f %.2f M (Contents) S\n", index_posx, index_posy);
    index_posy = index_posy - cfmt.indexfont.size;
  }

  fprintf (findex, "%.1f %d F1 \n",
           cfmt.indexfont.size, cfmt.indexfont.box);

}



/* ----- init_index_file ------- */
void init_index_file ()
{
  time_t  ltime;
  char    tstr[41];

  fprintf (findex, "%%!PS-Adobe-3.0\n");
  fprintf (findex, "%%%%Title: abc2ps index\n");
  time(&ltime);
  strcpy (tstr,  ctime(&ltime));
  tstr[24]='\0';
  tstr[16]='\0';
  fprintf (findex, "%%%%Creator: abc2ps %s.%s\n", VERSION, REVISION);
  fprintf (findex, "%%%%CreationDate: %s %s\n", tstr+4,tstr+20);
  if (PS_LEVEL == 2) fprintf (findex, "%%%%LanguageLevel: 2\n");
  fprintf (findex, "%%%%EndComments\n\n");

  fprintf (findex, "%%%%BeginSetup\n");
  if (PS_LEVEL < 2) level1_fix (findex);

  define_font (findex,cfmt.indexfont.name,1);
  fprintf (findex, "\n/T {translate} bind def\n/M {moveto} bind def\n");
  fprintf (findex, "/S {show} bind def\n");
  def_misc (findex);
  fprintf (findex, "%%%%EndSetup\n\n");

  index_pagenum=0;
  init_index_page (findex);

  index_initialized=1;

}


/* ----- close_index_page-------- */
void close_index_page (fp)
FILE *fp;
{

  fprintf (fp,
           "\n%%%%PageTrailer\n"
           "grestore\n"
           "showpage\n");
}


/* ----- close_page-------- */
void close_page (fp)
FILE *fp;
{
  if (vb>=10) printf ("close_page called; in_page=%d\n", in_page);

  if (! in_page) return;
  in_page=0;

  fprintf (fp,
           "\n%%%%PageTrailer\n"
           "grestore\n"
           "showpage\n");
}


/* ----- init_epsf: initialize epsf file ----- */
void init_epsf (fp)
FILE *fp;
{
  float px,py;
  px=cfmt.leftmargin;
  py=cfmt.pageheight-cfmt.topmargin;
  fprintf (fp, "%.2f %.2f translate\n", px, py);
}

/* ----- close_epsf: close epsf file ----- */
void close_epsf (fp)
FILE *fp;
{
  fprintf (fp,
           "\nshowpage\nend\n"
           "cleartomark origstate restore grestore\n\n");
}


/* ----- write_pagebreak ----- */
void write_pagebreak (fp)
FILE *fp;
{

  close_page (fp);
  init_page  (fp);
  if (strlen(page_init)>0) fprintf(fp,"%s\n", page_init);
  posy=cfmt.pageheight-cfmt.topmargin;

}

