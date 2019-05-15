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
#include "syms.h"

/*  subroutines to define postscript macros which draw symbols  */

/* ----- def_misc ------- */
void def_misc (fp)
FILE *fp;
{
  fprintf (fp,
           "\n/cshow { %% usage: string cshow  - center at current pt\n"
           "   dup stringwidth pop 2 div neg 0 rmoveto\n"
           "   bx {box} if show\n"
           "} bind def\n"
           "\n/lshow { %% usage: string lshow - show left-aligned\n"
           "   dup stringwidth pop neg 0 rmoveto bx {box} if show\n"
           "} bind def\n"
           "\n/rshow { %% usage: string rshow - show right-aligned\n"
           "   bx {box} if show\n"
           "} bind def\n"
           );

  fprintf (fp,
           "\n/box {  %% usage: str box  - draw box around string\n"
           "  gsave 0.5 setlinewidth dup stringwidth pop\n"
           "  -2 -2 rmoveto 4 add fh 4 add 2 copy\n"
           "  0 exch rlineto 0 rlineto neg 0 exch rlineto neg 0 rlineto\n"
           "  stroke grestore\n"
           "} bind def\n");

  fprintf (fp,
           "\n/wd { moveto bx {box} if show } bind def\n"
           "/wln {\n"
           "dup 3 1 roll moveto gsave 0.6 setlinewidth lineto stroke grestore\n"
           "} bind def\n");

  fprintf (fp,
           "/whf {moveto gsave 0.5 1.2 scale (-) show grestore} bind def\n");

}

/* ----- def_typset ------- */
void def_typeset(fp)
FILE *fp;
{

  fprintf (fp,
           "\n/WS {   %%usage:  w nspaces str WS\n"
           "   dup stringwidth pop 4 -1 roll\n"
           "   sub neg 3 -1 roll div 0 8#040 4 -1 roll\n"
           "   widthshow\n"
           "} bind def\n");


  fprintf (fp,
           "\n/W1 { show pop pop } bind def\n");


  fprintf (fp,
           "\n/str 50 string def\n"
           "/W0 {\n"
           "   dup stringwidth pop str cvs exch show (  ) show show pop pop\n"
           "} bind def\n");

  fprintf (fp,
           "\n/WC { counttomark 1 sub dup 0 eq { 0 }\n"
           "  {  ( ) stringwidth pop neg 0 3 -1 roll\n"
           "  {  dup 3 add index stringwidth pop ( ) stringwidth pop add\n"
           "  dup 3 index add 4 index lt 2 index 1 lt or\n"
           "  {3 -1 roll add exch 1 add} {pop exit} ifelse\n"
           "  } repeat } ifelse\n"
           "} bind def\n");

  fprintf (fp,
           "\n/P1 {\n"
           "  {  WC dup 0 le {exit} if\n"
           "      exch pop gsave { exch show ( ) show } repeat grestore LF\n"
           "   } loop pop pop pop pop\n"
           "} bind def\n");

  fprintf (fp,
           "\n/P2 {\n"
           "   {  WC dup 0 le {exit} if\n"
           "      dup 1 sub dup 0 eq\n"
           "      { pop exch pop 0.0 }\n"
           "      { 3 2 roll 3 index exch sub exch div } ifelse\n"
           "      counttomark 3 sub 2 index eq { pop 0 } if exch gsave\n"
           "      {  3 2 roll show ( ) show dup 0 rmoveto } repeat\n"
           "      grestore LF pop\n"
           "   } loop pop pop pop pop\n"
           "} bind def\n");

}

/* ----- define_font ------- */
void define_font (fp,name,num)
FILE *fp;
char name[];
int  num;
{

  if (!strcmp(name,"Symbol")) {
    fprintf (fp,
             "/F%d { 1 eq {/bx true def} { /bx false def} ifelse\n"
             "  dup 0.72 mul  /fh exch def\n"
             "  /%s exch selectfont } bind def\n",
             num, name);
    return;
  }

  fprintf (fp,
           "\n/%s findfont\n"
           "dup length dict begin\n"
           "   {1 index /FID ne {def} {pop pop} ifelse} forall\n"
           "   /Encoding ISOLatin1Encoding def\n"
           "   currentdict\n"
           "end\n"
           "/%s-ISO exch definefont pop\n",
           name, name);

   fprintf (fp,
            "/F%d { 1 eq {/bx true def} { /bx false def} ifelse\n"
            "  dup 0.72 mul  /fh exch def\n"
            "  /%s-ISO exch selectfont } bind def\n",
            num, name);

}


/* ----- def_tsig ------- */
void def_tsig (fp)
FILE *fp;
{
  fprintf (fp,
           "\n/tsig { %% usage: x (top) (bot) tsig -- draw time signature\n"
           "   3 -1 roll 0 moveto /bx false def\n"
           "   gsave /Times-Bold 16 selectfont 1.2 1 scale\n"
           "   0 1.0 rmoveto currentpoint 3 -1 roll cshow\n"
           "   moveto 0 12 rmoveto cshow grestore\n"
           "} bind def\n"
           );
}

/* ----- add_cv ------- */
void add_cv (FILE *fp, float f1, float f2, float p[][2], int i0, int ncv)
{
  int i,i1,m;

  i1=i0;
  for (m=0; m<ncv; m++) {
    fprintf (fp, " ");
    for (i=0; i<3; i++)
      fprintf (fp, " %.2f %.2f",
               f1*(p[i1+i][0]-p[i1-1][0]),
               f2*(p[i1+i][1]-p[i1-1][1]));
    fprintf (fp, " rcurveto\n");
    i1=i1+3;
  }
}

/* ----- add_sg ------- */
void add_sg (FILE *fp, float f1, float f2, float p[][2], int i0, int nseg)
{
  int i;
  for (i=0; i<nseg; i++)
    fprintf (fp, "  %.2f %.2f rlineto\n",
             f1*(p[i0+i][0]-p[i0+i-1][0]),
             f2*(p[i0+i][1]-p[i0+i-1][1]));
}

/* ----- add_mv ------- */
void add_mv (FILE *fp, float f1, float f2, float p[][2], int i0)
{
  if (i0==0)
    fprintf (fp, "  %.2f %.2f rmoveto\n",
             f1*p[i0][0], f2*p[i0][1]);
  else
    fprintf (fp, "  %.2f %.2f rmoveto\n",
             f1*(p[i0][0]-p[i0-1][0]),
             f2*(p[i0][1]-p[i0-1][1]));
}


/* ----- def_stems ------- */
void def_stems (fp)
FILE *fp;
{
  fprintf (fp,
           "\n/su {  %% usage: len su   - up stem\n"
           "  x y moveto %.1f %.1f rmoveto %.1f sub 0 exch rlineto stroke\n"
           "} bind def\n",
           STEM_XOFF, STEM_YOFF, STEM_YOFF );

  fprintf(fp,
          "\n/sd {  %% usage: len td   - down stem\n"
          "  x y moveto %.1f %.1f rmoveto neg %.1f add 0 exch rlineto stroke\n"
          "} bind def\n",
          -STEM_XOFF, -STEM_YOFF, STEM_YOFF);
}

/* ----- def_dot ------- */
void def_dot (fp)
FILE *fp;
{
  fprintf(fp,
          "\n/dt {  %% usage: dx dy dt  - dot shifted by dx,dy\n"
          "  y add exch x add exch 1.2 0 360 arc fill\n"
          "} bind def\n");
}

/* ----- def_deco ------- */
void def_deco (fp)
FILE *fp;
{

  float p[7][2] = {
    {-10,-2},{0,15},{1,-11},{10,2},{0,-15},{-1,11},{-10,-2} };

/*  float q[7][2] = {
    {-13,0},{-2,9},{2,9},{13,0},{3,5},{-3,5},{-13,-0} }; */

/*  float q[7][2] = {
    {-11,0},{-9,10},{9,10},{11,0},{5,7},{-5,7},{-11,-0} }; */

  /* Walsh suggestion, scale 1.8 in y */
  float q[7][2] = {
    {-13,0},{-12,9},{12,9},{13,0},{10,7.4},{-10,7.4},{-13,-0} };

  float s[7][2] = {
    {-8,-4.8},{-6,-5.5},{-3,-4.6},{0,0},{-2.3,-5},{-6,-6.8},{-8.5,-6} };

  float f1,f2;
  int i;

  f1=0.5;
  f2=0.5;

  fprintf (fp, "\n/grm {  %% usage:  y grm  - gracing mark\n"
           "  x exch moveto\n");
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,2);
  fprintf (fp, "   fill\n} bind def\n");
  fprintf (fp, "\n/stc {  %% usage:  y stc  - staccato mark\n"
           "  x exch 1.2 0 360 arc fill } bind def\n");

  fprintf (fp, "\n/hat {  %% usage:  y hat\n"
           "  x exch moveto\n"
           "  -4 -2 rmoveto 4 6 rlineto currentpoint stroke moveto\n"
           "  4 -6 rlineto -2 0 rlineto -3 4.5 rlineto fill\n"
           " } bind def\n");

  fprintf (fp, "\n/att {  %% usage:  y att\n"
           "  x exch moveto\n"
           "  -4 -3 rmoveto 8 3 rlineto -8 3 rlineto stroke\n"
           " } bind def\n");

  f2=f2*1.8;

  if (temp_switch==3) { f1=0.8*f1; f2=0.8*f2; }
  else                { f1=0.9*f1; f2=0.9*f2; }

  fprintf (fp, "\n/cpu {  %% usage:  y cpu  - roll sign above head\n"
           "  x exch moveto\n");
  add_mv (fp,f1,f2,q,0);
  add_cv (fp,f1,f2,q,1,2);
  fprintf (fp, "   fill\n} bind def\n");

  for (i=0;i<7;i++) q[i][1]=-q[i][1];

  fprintf (fp, "\n/cpd {  %% usage:  y cpd  - roll sign below head\n"
           "  x exch moveto\n");
  add_mv (fp,f1,f2,q,0);
  add_cv (fp,f1,f2,q,1,2);
  fprintf (fp, "   fill\n} bind def\n");

  f1=0.9;
  f2=1.0;
  fprintf (fp, "\n/sld {  %% usage:  y dx sld  - slide\n"
           "  x exch sub exch moveto\n");
  add_mv (fp,f1,f2,s,0);
  add_cv (fp,f1,f2,s,1,2);
  fprintf (fp, "   fill\n} bind def\n");

  fprintf (fp, "\n/emb {  %% usage:  y emb  - empahsis bar\n"
           "  gsave 1.4 setlinewidth 1 setlinecap x exch moveto \n"
           " -3.5 0 rmoveto 7 0 rlineto stroke grestore\n"
           "} bind def\n");

  fprintf (fp, "\n/trl {  %% usage:  y trl  - trill sign\n"
           "  gsave /Times-BoldItalic 14 selectfont\n"
           "  x 4 sub exch moveto (tr) show grestore\n"
           "} bind def\n");

}



/* ----- def_deco1 ------- */
void def_deco1 (fp)
FILE *fp;
{

  float p[8][2] = {     /* for hold sign */
    {-15,0},{-15,23},{15,23},{15,0},
    {14.5,0},{12,18},{-12,18},{-14.5,0} };

  float q[8][2] = {    /* for down bow sign */
    {-4,0},{-4,9},{4,9},{4,0},
    {-4,6},{-4,9},{4,9},{4,6} };

  float r[3][2] = {    /* for up bow sign */
    {-3.2,11},{0,0},{3.2,11} };

  float f1,f2;

  f1=f2=0.5;
  fprintf (fp, "\n/hld {  %% usage:  y hld  - hold sign\n"
           "  x exch 2 copy 1.5 add 1.3 0 360 arc moveto\n");
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,1);
  add_sg (fp,f1,f2,p,4,1);
  add_cv (fp,f1,f2,p,5,1);
  fprintf (fp, "   fill\n} bind def\n");

  f1=f2=0.8;
  fprintf (fp, "\n/dnb {  %% usage:  y dnb  - down bow\n"
           "  x exch moveto\n");
  add_mv (fp,f1,f2,q,0);
  add_sg (fp,f1,f2,q,1,3);
  fprintf (fp, "   currentpoint stroke moveto\n");
  add_mv (fp,f1,f2,q,4);
  add_sg (fp,f1,f2,q,5,3);
  fprintf (fp, "   fill\n} bind def\n");

  fprintf (fp, "\n/upb {  %% usage:  y upb  - up bow\n"
           "  x exch moveto\n");
  add_mv (fp,f1,f2,r,0);
  add_sg (fp,f1,f2,r,1,2);
  fprintf (fp, "   stroke\n} bind def\n");

}

/* ----- def_hl ------- */
void def_hl (fp)
FILE *fp;
{
  fprintf(fp,
          "\n/hl {  %% usage: y hl  - helper line at height y\n"
          "   gsave 1 setlinewidth x exch moveto \n"
          "   -5.5 0 rmoveto 11 0 rlineto stroke grestore\n"
          "} bind def\n");

  fprintf(fp,
          "\n/hl1 {  %% usage: y hl1  - longer helper line\n"
          "   gsave 1 setlinewidth x exch moveto \n"
          "   -7 0 rmoveto 14 0 rlineto stroke grestore\n"
          "} bind def\n");
}

/* ----- def_beam ------- */
void def_beam (fp)
FILE *fp;
{
  fprintf(fp,
          "\n/bm {  %% usage: x1 y1 x2 y2 t bm  - beam, depth t\n"
          "  3 1 roll moveto dup 0 exch neg rlineto\n"
          "  dup 4 1 roll sub lineto 0 exch rlineto fill\n"
          "} bind def\n");

  fprintf(fp,
          "\n/bnum {  %% usage: x y (str) bnum  - number on beam\n"
          "  3 1 roll moveto gsave /Times-Italic 12 selectfont\n"
          "  /bx false def cshow grestore\n"
          "} bind def\n");


  fprintf(fp,
          "\n/hbr {  %% usage: x1 y1 x2 y2 hbr  - half bracket\n"
          "  moveto lineto 0 -3 rlineto stroke\n"
          "} bind def\n");
}


/* ----- def_flags1 ------- */
void def_flags1 (fp)
FILE *fp;
{
  float p[13][2] = {
    {0.0, 0.0},  {1.5, -3.0},  {1.0, -2.5},  {4.0, -6.0},  {9.0, -10.0},
    {9.0, -16.0},  {8.0, -20.0},  {7.0, -24.0},  {4.0, -26.0},
    {6.5, -21.5},  {9.0, -15.0},  {4.0, -9.0},  {0.0, -8.0} } ;

  float f1,f2;
  int i;

  f1=f2=6.0/9.0;
  fprintf (fp, "\n/f1u {  %% usage:  len f1u  - single flag up\n"
           "  y add x %.1f add exch moveto\n", STEM_XOFF);
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,4);
  fprintf (fp, "   fill\n} bind def\n");

  f1=1.2*f1;
  for (i=0;i<13;i++) p[i][1]=-p[i][1];
  fprintf (fp, "\n/f1d {  %% usage:  len f1d  - single flag down\n"
           "  neg y add x %.1f sub exch moveto\n", STEM_XOFF);
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,4);
  fprintf (fp, "   fill\n} bind def\n");

}

/* ----- def_flags2 ------- */
void def_flags2 (fp)
FILE *fp;
{

  float p[13][2] = {
    {0.0, 0.0},
    {2.0, -5.0},  {9.0, -6.0},  {7.5, -18.0},
    {7.5, -9.0},  {1.5, -6.5},  {0.0, -6.5},
    {2.0, -14.0},  {9.0, -14.0}, {7.5, -26.0},
    {7.5, -17.0},  {1.5, -14.5},  {0.0, -14.0},
  };

  float f1,f2;
  int i;

  f1=f2=6.0/9.0;                       /* up flags */
  fprintf (fp, "\n/f2u {  %% usage:  len f2u  - double flag up\n"
           "  y add x %.1f add exch moveto\n", STEM_XOFF);
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,4);
  fprintf (fp, "   fill\n} bind def\n");

  f1=1.2*f1;                           /* down flags */
  for (i=0;i<13;i++) p[i][1]=-p[i][1];
  fprintf (fp, "\n/f2d {  %% usage:  len f2d  - double flag down\n"
           "  neg y add x %.1f sub exch moveto\n", STEM_XOFF);
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,4);
  fprintf (fp, "   fill\n} bind def\n");

}



/* ----- def_xflags ------- */
void def_xflags (fp)
FILE *fp;
{

  float p[7][2] = {
    {0.0, 0.0},
    {2.0, -7.5},  {9.0, -7.5}, {7.5, -19.5},
    {7.5, -10.5},  {1.5, -8.0},  {0.0, -7.5}
  };

  float f1,f2;
  int i;

  f1=f2=6.0/9.0;                       /* extra up flag */
  fprintf (fp, "\n/xfu {  %% usage:  len xfu  - extra flag up\n"
           "  y add x %.1f add exch moveto\n", STEM_XOFF);
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,2);
  fprintf (fp, "   fill\n} bind def\n");

  f1=1.2*f1;                           /* extra down flag */
  for (i=0;i<7;i++) p[i][1]=-p[i][1];
  fprintf (fp, "\n/xfd {  %% usage:  len xfd  - extra flag down\n"
           "  neg y add x %.1f sub exch moveto\n", STEM_XOFF);
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,2);
  fprintf (fp, "   fill\n} bind def\n");

  fprintf (fp,
           "\n/f3d {dup f2d 9.5 sub xfd} bind def\n");

  fprintf (fp,
           "\n/f4d {dup dup f2d 9.5 sub xfd 14.7 sub xfd} bind def\n");

  fprintf (fp,
           "\n/f3u {dup f2u 9.5 sub xfu} bind def\n");

  fprintf (fp,
           "\n/f4u {dup dup f2u 9.5 sub xfu 14.7 sub xfu} bind def\n");

}

/* ----- def_acc ------- */
void def_acc (fp)
FILE *fp;
{
  float p[12][2]={
    {-2,3},{6,6.5},{6,-1},{-2,-4.5},{4,0},{4,4},{-2,2},{-2,10},{-2,-4}};
  float q[14][2]={
    {4,4},{4,7},{-4,5},{-4,2},{4,4},{4,-5},{4,-2},{-4,-4},{-4,-7},{4,-5},
    {2,-10},{2,11.5},{-2,-11.5},{-2,10} };
  float r[14][2]={
    {-2.5,-6}, {2.5,-5}, {2.5,-2}, {-2.5,-3}, {-2.5,6},
    {-2.5,2}, {2.5,3}, {2.5,6}, {-2.5,5}, {-2.5,2},
    {-2.5,11}, {-2.5,-5.5},
    {2.5,5.5}, {2.5,-11} };
  float s[25][2]={
    {0.7,0},{3.9,3},{6,3},{6.2,6.2},{3,6},{3,3.9},
    {0,0.7},{-3,3.9},{-3,6},{-6.2,6.2},{-6,3},{-3.9,3},
    {-0.7,0},{-3.9,-3},{-6,-3},{-6.2,-6.2},{-3,-6},{-3,-3.9},
    {0,-0.7},{3,-3.9},{3,-6},{6.2,-6.2},{6,-3},{3.9,-3},
    {0.7,0} };


  float f1,f2;

  f2=8.0/9.0;
  f1=f2*0.9;
  fprintf (fp, "\n/ft0 { %% usage:  x y ft0  - flat sign\n"
           "  moveto\n");
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,2);
  fprintf (fp, "  currentpoint fill moveto\n");
  add_mv (fp,f1,f2,p,7);
  add_sg (fp,f1,f2,p,8,1);
  fprintf (fp, "  stroke\n } bind def\n");
  fprintf (fp, "/ft { %% usage: dx ft  - flat relative to head\n"
           " neg x add y ft0 } bind def\n");

  f2=8.0/9.0;    /* more narrow flat sign for double flat */
  f1=f2*0.8;
  fprintf (fp, "\n/ftx { %% usage:  x y ftx  - narrow flat sign\n"
           "  moveto\n");
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,2);
  fprintf (fp, "  currentpoint fill moveto\n");
  add_mv (fp,f1,f2,p,7);
  add_sg (fp,f1,f2,p,8,1);
  fprintf (fp, "  stroke\n } bind def\n");

  fprintf (fp, "/dft0 { %% usage: x y dft0 ft  - double flat sign\n"
           "  2 copy exch 2.5 sub exch ftx exch 1.5 add exch ftx } bind def\n"
           "/dft { %% usage: dx dft  - double flat relative to head\n"
           "  neg x add y dft0 } bind def\n");


  f2=6.5/9.0;
  f1=f2*0.9;
  fprintf (fp, "\n/sh0 {  %% usage:  x y sh0  - sharp sign\n"
           "  moveto\n");
  add_mv (fp,f1,f2,q,0);
  add_sg (fp,f1,f2,q,1,4);
  add_mv (fp,f1,f2,q,5);
  add_sg (fp,f1,f2,q,6,4);
  fprintf (fp, "  currentpoint fill moveto\n");
  add_mv (fp,f1,f2,q,10);
  add_sg (fp,f1,f2,q,11,1);
  fprintf (fp, "  currentpoint stroke moveto\n");
  add_mv (fp,f1,f2,q,12);
  add_sg (fp,f1,f2,q,13,1);
  fprintf (fp, "  stroke\n } bind def\n");
  fprintf (fp, "/sh { %% usage: dx sh  - sharp relative to head\n"
           " neg x add y sh0 } bind def\n");

  f2=6.5/9.0;
  f1=f2*0.9;
  fprintf (fp, "\n/nt0 {  %% usage:  x y nt0  - neutral sign\n"
           "  moveto\n");
  add_mv (fp,f1,f2,r,0);
  add_sg (fp,f1,f2,r,1,4);
  add_mv (fp,f1,f2,r,5);
  add_sg (fp,f1,f2,r,6,4);
  fprintf (fp, "  currentpoint fill moveto\n");
  add_mv (fp,f1,f2,r,10);
  add_sg (fp,f1,f2,r,11,1);
  fprintf (fp, "  currentpoint stroke moveto\n");
  add_mv (fp,f1,f2,r,12);
  add_sg (fp,f1,f2,r,13,1);
  fprintf (fp, "  stroke\n } bind def\n");
  fprintf (fp, "/nt { %% usage: dx nt  - neutral relative to head\n"
           " neg x add y nt0 } bind def\n");

  f1=5.0/9.0;
  f2=f1;
  fprintf (fp, "\n/dsh0 {  %% usage:  x y dsh0  - double sharp \n"
           "  moveto\n");
  add_mv (fp,f1,f2,s,0);
  add_sg (fp,f1,f2,s,1,24);
  fprintf (fp, "  fill\n } bind def\n");
  fprintf (fp, "/dsh { %% usage: dx dsh  - double sharp relative to head\n"
           " neg x add y dsh0 } bind def\n");
}

/* ----- def_rests ------- */
void def_rests (fp)
FILE *fp;
{
  float p[14][2]={
    {-1,17}, {15,4}, {-6,8}, {6.5,-5}, {-2,-2}, {-5,-11}, {1,-15},
    {-9,-11}, {-6,0}, {1,-1},   {-9,7}, {7,5}, {-1,17} };
  float q[16][2]={
    {8,14}, {5,9}, {3,5}, {-1.5,4},
    {4,11}, {-9,14}, {-9,7},
    {-9,4}, {-6,2}, {-3,2},
    {4,2}, {5,7}, {7,11},
    {-1.8,-20},  {-0.5,-20}, {8.5,14}};
  float r[29][2]={
    {8,14}, {5,9}, {3,5}, {-1.5,4},
    {4,11}, {-9,14}, {-9,7},
    {-9,4}, {-6,2}, {-3,2},
    {4,2}, {5,7}, {7,11},
    {8,14}, {5,9}, {3,5}, {-1.5,4},
    {4,11}, {-9,14}, {-9,7},
    {-9,4}, {-6,2}, {-3,2},
    {4,2}, {5,7}, {7.3,11},
    {-1.8,-21},  {-0.5,-21}, {8.5,14} };
  float f1,f2;
  int i;

  fprintf (fp, "\n/r4 {  %% usage:  x y r4  -  quarter rest\n"
           "   dup /y exch def exch dup /x exch def exch moveto\n");
  f1=f2=6.0/11.5;
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,4);
  fprintf (fp, "  fill\n } bind def\n");

  fprintf (fp, "\n/r8 {  %% usage:  x y r8  -  eighth rest\n"
           "   dup /y exch def exch dup /x exch def exch moveto\n");
  f1=f2=7/18.0;
  add_mv (fp,f1,f2,q,0);
  add_cv (fp,f1,f2,q,1,4);
  add_sg (fp,f1,f2,q,13,3);
  fprintf (fp, "  fill\n } bind def\n");

  for (i=13;i<26;i++) { r[i][0]-=4.2; r[i][1]-=14; }
  fprintf (fp, "\n/r16 {  %% usage:  x y r16  -  16th rest\n"
           "   dup /y exch def exch dup /x exch def exch moveto\n");
  f1=f2=7/18.0;
  add_mv (fp,f1,f2,r,0);
  add_cv (fp,f1,f2,r,1,4);
  add_sg (fp,f1,f2,r,13,1);
  add_cv (fp,f1,f2,r,14,4);
  add_sg (fp,f1,f2,r,26,3);
  fprintf (fp, "  fill\n } bind def\n");


  fprintf (fp,
           "\n/r1 {  %% usage:  x y r1  -  whole rest\n"
           "  dup /y exch def exch dup /x exch def exch moveto\n"
           "  -3 6 rmoveto 0 -3 rlineto 6 0 rlineto 0 3 rlineto fill\n"
           "} bind def\n");

  fprintf (fp,
           "\n/r2 {  %% usage:  x y r2  -  half rest\n"
           "  dup /y exch def exch dup /x exch def exch moveto\n"
           "  -3 0 rmoveto 0 3 rlineto 6 0 rlineto 0 -3 rlineto fill\n"
           "} bind def\n"
           );

  /* get 32nd, 64th rest by overwriting 8th and 16th rests */
  fprintf (fp,
           "\n/r32 {\n"
           "2 copy r16 5.5 add exch 1.6 add exch r8\n"
           "} bind def\n");
  fprintf (fp,
           "\n/r64 {\n"
           "2 copy 5.5 add exch 1.6 add exch r16\n"
           "5.5 sub exch 1.5 sub exch r16\n"
           "} bind def\n");

}


/* ----- def_bars ------ */
void def_bars (fp)
FILE *fp;
{

  fprintf(fp, "\n/bar {  %% usage: x bar  - single bar\n"
          "  0 moveto  0 24 rlineto stroke\n"
          "} bind def\n"

          "\n/dbar {  %% usage: x dbar  - thin double bar\n"
          "   0 moveto 0 24 rlineto -3 -24 rmoveto\n"
          "   0 24 rlineto stroke\n"
          "} bind def\n"

          "\n/fbar1 {  %% usage: x fbar1  - fat double bar at start\n"
          "  0 moveto  0 24 rlineto 3 0 rlineto 0 -24 rlineto \n"
          "  currentpoint fill moveto\n"
          "  3 0 rmoveto 0 24 rlineto stroke\n"
          "} bind def\n"

          "\n/fbar2 {  %% usage: x fbar2  - fat double bar at end\n"
          "  0 moveto  0 24 rlineto -3 0 rlineto 0 -24 rlineto \n"
          "  currentpoint fill moveto\n"
          "  -3 0 rmoveto 0 24 rlineto stroke\n"
          "} bind def\n"

          "\n/rdots {  %% usage: x rdots  - repeat dots \n"
          "  0 moveto 0 9 rmoveto currentpoint 2 copy 1.2 0 360 arc \n"
          "  moveto 0 6 rmoveto  currentpoint 1.2 0 360 arc fill\n"
          "} bind def\n");
}

/* ----- def_ends ------ */
void def_ends (fp)
FILE *fp;
{
  /* use dy=20 for tall boxes */
  int y=50,dy=6;

  fprintf(fp, "\n/end1 {  %% usage: x1 x2 (str) end1  - mark first ending\n"
          "  3 1 roll %d moveto 0 %d rlineto dup %d lineto 0 %d rlineto stroke\n"
          "  4 add %d moveto gsave /Times-Roman 13 selectfont 1.2 0.95 scale\n"
          "  show grestore\n"
          "} bind def\n",
          y-dy,  dy, y, -dy,  y-10);

  fprintf(fp, "\n/end2 {  %% usage: x1 x2 (str) end2  - mark second ending\n"
          "  3 1 roll %d moveto dup %d lineto 0 %d rlineto stroke\n"
          "  4 add %d moveto gsave /Times-Roman 13 selectfont 1.2 0.95 scale\n"
          "  show grestore\n"
          "} bind def\n",
          y, y, -dy,  y-10);
}

/* ----- def_gchord ------ */
void def_gchord (fp)
FILE *fp;
{
  fprintf(fp,"\n/gc { %% usage: x y (str) gc  -- draw guitar chord string\n"
          "  3 1 roll moveto rshow\n"
          "} bind def\n");
}

/* ----- def_sl ------ */
void def_sl (fp)
FILE *fp;
{
/*  fprintf(fp, "\n/sl {  %% usage: x1 y2 x2 y2 x3 y3 x0 y0 sl\n"
          "  gsave %.1f setlinewidth moveto curveto stroke grestore\n"
          "} bind def\n", SLURWIDTH); */

  fprintf(fp, "\n/SL {  %% usage: pp2x pp1x p1 pp1 pp2 p2 p1 sl\n"
          "  moveto curveto rlineto curveto fill\n"
          "} bind def\n");

}

/* ----- def_hd1 ------- */
void def_hd1 (fp)
FILE *fp;
{
  float p[7][2] = {
    {8.0, 0.0},  {8.0, 8.0},  {-8.0, 8.0},  {-8.0, 0.0},  {-8.0, -8.0},
    {8.0, -8.0},  {8.0, 0.0} };

  float c,s,xx,yy,f1,f2;
  int i;
/*float phi; */

/*phi=0.6;
  c=cos(phi);
  s=sin(phi); */

  c=0.825; s=0.565;

  for (i=0;i<7;i++) {
    xx = c*p[i][0] - s*p[i][1];
    yy = s*p[i][0] + c*p[i][1];
    p[i][0]=xx;
    p[i][1]=yy;
  }

  f1=f2=6.0/12.0;
  fprintf (fp, "\n/hd {  %% usage: x y hd  - full head\n"
           "  dup /y exch def exch dup /x exch def exch moveto\n");
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,2);
  fprintf (fp, "   fill\n} bind def\n");
}

/* ----- def_hd2 ------- */
void def_hd2 (fp)
FILE *fp;
{

  float p[14][2] = {
    {8.0, 0.0},  {8.0, 8.5},  {-8.0, 8.5},  {-8.0, 0.0},  {-8.0, -8.5},
    {8.0, -8.5},  {8.0, 0.0},  {7.0, 0.0},  {7.0, -4.0},  {-7.0, -4.0},
    {-7.0, 0.0},  {-7.0, 4.0},  {7.0, 4.0},  {7.0, 0.0} };

/*  float phi; */
  float c,s,xx,yy,f1,f2;
  int i;

/*phi=0.5;
  c=cos(phi);
  s=sin(phi); */

  c=0.878; s=0.479;

  for (i=0;i<14;i++) {
    xx = c*p[i][0] - s*p[i][1];
    yy = s*p[i][0] + c*p[i][1];
    p[i][0]=xx;
    p[i][1]=yy;
  }

  f1=f2=6.0/12.0;
  fprintf (fp, "\n/Hd {  %% usage: x y Hd  - open head for half\n"
           "  dup /y exch def exch dup /x exch def exch moveto\n");
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,2);
  add_mv (fp,f1,f2,p,7);
  add_cv (fp,f1,f2,p,8,2);
  fprintf (fp, "   fill\n} bind def\n");
}

/* ----- def_hd3 ------- */
void def_hd3 (fp)
FILE *fp;
{

  float p[13][2] = {
    {11.0, 0.0}, {11.0, 2.0},  {6.0, 6.5},  {0.0, 6.5},  {-6.0, 6.5},
    {-11.0, 2.0},  {-11.0, 0.0},  {-11.0, -2.0},  {-6.0, -6.5},
    {0.0, -6.5},  {6.0, -6.5},  {11.0, -2.0},  {11.0, 0.0}  };

  float q[8][2] = {
    {11.0, 0.0},  {5.0, 0.0},  {5.0, -5.0},  {-5.0, -5.0},  {-5.0, 0.0},
    {-5.0, 5.0},  {5.0, 5.0},  {5.0, 0.0}};

/*  float phi; */
  float c,s,xx,yy,f1,f2;
  int i;

/*phi=2.5;
  c=cos(phi);
  s=sin(phi); */

  c=-0.801; s=0.598;

  for (i=1;i<8;i++) {
    xx = c*q[i][0] - s*q[i][1];
    yy = s*q[i][0] + c*q[i][1];
    q[i][0]=xx;
    q[i][1]=yy;
  }

  f1=f2=6.5/12.0;
  fprintf (fp, "\n/HD { %% usage: x y HD  - open head for whole\n"
           "  dup /y exch def exch dup /x exch def exch moveto\n");
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,4);
  add_mv (fp,f1,f2,q,1);
  add_cv (fp,f1,f2,q,2,2);
  fprintf (fp, "   fill\n} bind def\n");

}

/* ----- def_gnote ------- */
void def_gnote (fp)
FILE *fp;
{
  float p[7][2] = {
    {0,10}, {16,10}, {16,-10}, {0,-10}, {-16,-10}, {-16,10}, {0,10} };

/*  float phi; */
  float c,s,xx,yy,f1,f2;
  int i;

/*phi=0.7;
  c=cos(phi);
  s=sin(phi); */

  c=0.765; s=0.644;

  for (i=0;i<7;i++) {
    xx = c*p[i][0] - s*p[i][1];
    yy = s*p[i][0] + c*p[i][1];
    p[i][0]=xx;
    p[i][1]=yy;
  }

  f1=f2=2./10.0;

  fprintf (fp, "\n/gn1 {  %% usage: x y l gnt  - grace note w. tail\n"
           "  3 1 roll 2 copy moveto\n");
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,2);
  fprintf (fp, "  fill moveto %.2f 0 rmoveto 0 exch rlineto\n"
           "3 -4 4 -5 2 -8 rcurveto -5 2 rmoveto 7 4 rlineto   \n"
           "stroke\n",
           GSTEM_XOFF);
  fprintf (fp, "} bind def\n");

  fprintf (fp, "\n/gnt {  %% usage: x y l gnt  - grace note\n"
           "  3 1 roll 2 copy moveto\n");
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,2);
  fprintf (fp, "  fill moveto %.2f 0 rmoveto 0 exch rlineto stroke\n",
           GSTEM_XOFF);
  fprintf (fp, "} bind def\n");

  fprintf(fp, "\n/gbm2 {  %% usage: x1 y1 x2 y2 gbm2  - double note beam\n"
          "  gsave 1.4 setlinewidth\n"
          "  4 copy 0.5 sub moveto 0.5 sub lineto stroke\n"
          "  3.4 sub moveto 3.4 sub lineto stroke grestore\n"
          "} bind def\n");

  fprintf(fp, "\n/gbm3 {  %% usage: x1 y1 x2 y2 gbm3  - triple gnote beam\n"
          "  gsave 1.2 setlinewidth\n"
          "  4 copy 0.3 sub moveto 0.3 sub lineto stroke\n"
          "  4 copy 2.5 sub moveto 2.5 sub lineto stroke\n"
          "  4.7 sub moveto 4.7 sub lineto stroke grestore\n"
          "} bind def\n");

  fprintf(fp, "\n/ghl {  %% usage: x y ghl  - grace note helper line\n"
          "   gsave 0.7 setlinewidth moveto \n"
          "   -3 0 rmoveto 6 0 rlineto stroke grestore\n"
          "} bind def\n");

  fprintf(fp, "\n/gsl {  %% usage: x1 y2 x2 y2 x3 y3 x0 y0 gsl\n"
          "  moveto curveto stroke\n"
          "} bind def\n");

  fprintf(fp, "\n/gsh0 {  %% usage: x y gsh0\n"
          "gsave translate 0.7 0.7 scale 0 0 sh0 grestore\n"
          "} bind def\n");

  fprintf(fp, "\n/gft0 {  %% usage: x y gft0\n"
          "gsave translate 0.7 0.7 scale 0 0 ft0 grestore\n"
          "} bind def\n");

  fprintf(fp, "\n/gnt0 {  %% usage: x y gnt0\n"
          "gsave translate 0.7 0.7 scale 0 0 nt0 grestore\n"
          "} bind def\n");

  fprintf(fp, "\n/gdf0 {  %% usage: x y gdf0\n"
          "gsave translate 0.7 0.6 scale 0 0 dft0 grestore\n"
          "} bind def\n");

  fprintf(fp, "\n/gds0 {  %% usage: x y gds0\n"
          "gsave translate 0.7 0.7 scale 0 0 dsh0 grestore\n"
          "} bind def\n");
}


/* ----- def_csig ------- */
void def_csg (fp)
FILE *fp;
{
  float p[25][2]={
  {0,26},
  {4,26}, {11,23},  {11,14},
  {11,20},  {5,19}, {5,14},
  {5,9}, {12,9}, {12,15},
  {12,25}, {6,28},  {0,28},
  {-15,28}, {-25,17}, {-25,2},
  {-25,-10}, {-10,-28}, {11,-8},
  {-6,-20}, {-18,-11}, {-18,2},
  {-18,14}, {-14,26}, {0,26} };

  float f1,f2;
  int i;

  for (i=0;i<25;i++) {
    p[i][0]=p[i][0]+4;
    p[i][1]=p[i][1]+43;
  }
  f1 = f2 = 0.25;
  fprintf (fp, "\n/csig {  %% usage:  x csig  - C timesig \n"
           "  0 moveto\n");
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,8);
  fprintf (fp, "   fill\n} bind def\n");

  fprintf (fp, "\n/ctsig {  %% usage:  x ctsig  - C| timesig \n"
           "  dup csig 4 moveto 0 16 rlineto stroke\n"
           "} bind def\n");
}


/* ----- def_tclef ------- */
void def_tclef (fp)
FILE *fp;
{
  float p[71][2]={
    {-6, 16},  {-8, 13},  {-14, 19},  {-10, 35},  {2, 35},  {8, 37},
    {21, 30},  {21, 17},  {21, 5},  {10, -1},  {0, -1},  {-12, -1},
    {-23, 5},  {-23, 22},  {-23, 29},  {-22, 37},  {-7, 49},  {10, 61},
    {10, 68},  {10, 73},  {10, 78},  {9, 82},  {7, 82},  {2, 78},
    {-2, 68},  {-2, 62},  {-2, 25},  {10, 18},  {11, -8},  {11, -18},
    {5, -23},  {-4, -23},  {-10, -23},  {-15, -18},  {-15, -13},
    {-15, -8},  {-12, -4},  {-7, -4},  {3, -4},  {3, -20},  {-6, -17},
    {-5, -23},  {9, -20},  {9, -9},  {7, 24},  {-5, 30},  {-5, 67},
    {-5, 78},  {-2, 87},  {7, 91},  {13, 87},  {18, 80},  {17, 73},
    {17, 62},  {10, 54},  {1, 45},  {-5, 38},  {-15, 33},  {-15, 19},
    {-15, 7},  {-8, 1},  {0, 1},  {8, 1},  {15, 6},  {15, 14},  {15, 23},
    {7, 26},  {2, 26},  {-5, 26},  {-9, 21},  {-6, 16} };

  float f1,f2;

  f1 = f2 = 24.0/65.0;
  fprintf (fp, "\n/tclef {  %% usage:  x tclef  - treble clef \n"
           "  0 moveto\n");
  add_mv (fp,f1,f2,p,0);
  add_sg (fp,f1,f2,p,1,1);
  add_cv (fp,f1,f2,p,2,23);
  fprintf (fp, "   fill\n} bind def\n");
  fprintf (fp, "\n/stclef {\n"
           "  0.85 div gsave 0.85 0.85 scale tclef grestore\n"
           "} bind def\n");
}

/* ----- def_t8clef ------- */
void def_t8clef (fp)
FILE *fp;
{
  fprintf (fp, "\n/t8clef {  %% usage:  x t8clef  - treble 8va clef\n"
           "  dup tclef\n"
           "  /Times-Roman findfont 12 scalefont setfont\n"
           "  -20 moveto (8) cshow\n"
           "} bind def\n"
           "\n/st8clef {\n"
           "  0.85 div gsave 0.85 0.85 scale t8clef grestore\n"
           "} bind def\n");
}

/* ----- def_bclef ------- */
void def_bclef (fp)
FILE *fp;
{
  float p[42][2]={
    {-2.3,3}, {6,7}, {10.5,12}, {10.5,16},
    {10.5,20.5}, {8.5,23.5}, {6.2,23.3},
    {5.2,23.5}, {2,23.5}, {0.5,19.5},
    {2,20}, {4,19.5}, {4,18},
    {4,17}, {3.5,16}, {2,16},
    {1,16}, {0,16.9}, {0,18.5},
    {0,21}, {2.1,24}, {6,24},
    {10,24}, {13.5,21.5}, {13.5,16.5},
    {13.5,11}, {7,5.5}, {-2.0,2.8},
    {14.9,21},
    {14.9,22.5}, {16.9,22.5}, {16.9,21},
    {16.9,19.5}, {14.9,19.5}, {14.9,21},
    {14.9,15},
    {14.9,16.5}, {16.9,16.5}, {16.9,15},
    {16.9,13.5}, {14.9,13.5}, {14.9,15} };

  int i;
  float f1,f2;

  for (i=0;i<42;i++) {p[i][0]-=7.5; p[i][1]-=0.5; }
  f1 = f2 = 1.0;
  fprintf (fp, "\n/bclef {  %% usage:  x bclef  - bass clef \n"
           "  0 moveto\n");
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,9);
  add_cv (fp,f1,f2,p,1,9);

  add_mv (fp,f1,f2,p,28);
  add_cv (fp,f1,f2,p,29,2);

  add_mv (fp,f1,f2,p,25);
  add_cv (fp,f1,f2,p,36,2);

  fprintf (fp, "fill\n} bind def\n");

  fprintf (fp, "\n/sbclef {\n"
           "  0.85 div gsave 0.85 0.85 scale 0 4 translate bclef grestore\n"
           "} bind def\n");
}

/* ----- def_cclef ------- */
void def_cclef (fp)
FILE *fp;
{
  float p[30][2]={
    {0,0}, {2,5.5},
    {9,4.5}, {12,10}, {12,15.5},
    {12,19.5}, {11,23.3}, {6.5,23.5},
    {5.2,23.5}, {2,23.5}, {0.5,19.5},
    {2,20}, {4,19.5}, {4,18},
    {4,17}, {3.5,16}, {2,16},
    {1,16}, {0,16.9}, {0,18.5},
    {0,21}, {2.1,24}, {6,24},
    {12,24}, {15,21.5}, {15,16.5},
    {15,10}, {10,4.5}, {4,5},
    {3,0} };
  int i;
  float f1,f2;

  for (i=0;i<30;i++) p[i][1]+=24;

  f1 = 0.6;
  f2 = 0.5;
  fprintf (fp, "\n/cchalf {\n"
           "  0 moveto\n");
  add_mv (fp,f1,f2,p,0);
  add_sg (fp,f1,f2,p,1,1);
  add_cv (fp,f1,f2,p,2,9);
  add_sg (fp,f1,f2,p,29,1);
  fprintf (fp, "fill\n} bind def\n");

  fprintf (fp,
           "\n/cclef {   %% usage: x cclef\n"
           "   dup dup dup\n"
           "   cchalf gsave 0 24 translate 1 -1 scale cchalf\n"
           "   6.5 sub 0 moveto 0 24 rlineto 3 0 rlineto 0 -24 rlineto fill\n"
           "   1.8 sub 0 moveto 0 24 rlineto 0.8 setlinewidth stroke grestore \n"
           "} bind def\n");

  fprintf (fp, "\n/scclef { cclef } bind def\n");
}

/* ----- def_brace ------- */
void def_brace (fp)
FILE *fp;
{
  float p[8][2]={
    {7.2,60}, {-7,39}, {17,17}, {-1,0},
    {-1.4,0}, {13,13}, {-11,39}, {7,60} };

  float q[8][2]={
    {-3,0}, {2,0}, {4,1}, {5.5,5},
    {5.9,4.7}, {4.7,1.2}, {3.2,-.4}, {-1,-1.2} };

  float f1,f2;

  f1 = 0.9;
  f2 = 1.0;
  fprintf (fp, "\n/bracehalf {\n");
  add_mv (fp,f1,f2,p,0);
  add_cv (fp,f1,f2,p,1,1);
  add_sg (fp,f1,f2,p,4,1);
  add_cv (fp,f1,f2,p,5,1);
  fprintf (fp, "  fill\n} bind def\n");

  fprintf (fp,
           "\n/brace {   %% usage: scale x0 y0 brace\n"
           "   3 copy moveto gsave 1 exch scale bracehalf grestore\n"
           "   moveto gsave neg 1 exch scale bracehalf grestore\n"
           "} bind def\n");

  f1 = 1.0;
  f2 = 1.0;
  fprintf (fp, "\n/brackhead {\n");
  add_mv (fp,f1,f2,q,0);
  add_cv (fp,f1,f2,q,1,1);
  add_sg (fp,f1,f2,q,4,1);
  add_cv (fp,f1,f2,q,5,1);
  fprintf (fp, "  fill\n} bind def\n");

  fprintf (fp,
           "\n/bracket {   %% usage: h x0 y0 bracket\n"
           "   3 copy moveto 0 exch rmoveto brackhead\n"
           "   3 copy moveto pop gsave 1 -1 scale brackhead grestore \n"
           "   moveto -3 0 rlineto 0 exch rlineto 3 0 rlineto fill\n"
           "} bind def \n");

}


/* ----- def_staff ------- */
void def_staff (fp)
FILE *fp;
{
  fprintf (fp,
           "\n/staff {  %% usage: l staff  - draw staff\n"
           "  gsave 0.6 setlinewidth 0 0 moveto\n"
           "  dup 0 rlineto dup neg 6 rmoveto\n"
           "  dup 0 rlineto dup neg 6 rmoveto\n"
           "  dup 0 rlineto dup neg 6 rmoveto\n"
           "  dup 0 rlineto dup neg 6 rmoveto\n"
           "  dup 0 rlineto dup neg 6 rmoveto\n"
           "  pop stroke grestore\n"
           "} bind def\n");
}

/* ----- def_sep ------- */
void def_sep (fp)
FILE *fp;
{
  fprintf (fp,
           "\n/sep0 { %% usage: x1 x2 sep0  - hline separator \n"
           "   0 moveto 0 lineto stroke\n"
           "} bind def\n");
}

/* ----- define_symbols: write postscript macros to file ------ */
void define_symbols (fp)
FILE *fp;
{

  def_misc (fp);
  def_tclef (fp);
  def_t8clef (fp);
  def_bclef (fp);
  def_cclef (fp);
  def_hd1 (fp);
  def_hd2 (fp);
  def_hd3 (fp);
  def_stems (fp);
  def_beam (fp);
  def_sl (fp);
  def_dot (fp);
  def_deco (fp);
  def_deco1 (fp);
  def_hl (fp);
  def_flags1 (fp);
  def_flags2 (fp);
  def_xflags (fp);
  def_acc (fp);
  def_gchord (fp);
  def_rests (fp);
  def_bars (fp);
  def_ends (fp);
  def_gnote (fp);
  def_csg (fp);
  def_sep (fp);
  def_tsig (fp);
  def_staff (fp);
  def_brace (fp);
  def_typeset(fp);

}
