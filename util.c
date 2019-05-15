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

#include <sys/stat.h>
#include <ctype.h>
#include <string.h>

#include "abc2ps.h"

#include "util.h"

/*  low-level utilities  */


/* ----- error warning ----- */
void wng (char msg[], char str[])
{
  printf ("+++ %s%s\n", msg, str);
}

/* ----- error exit ----- */
void rx (msg, str)
char msg[],str[];
{
  printf ("\n+++ %s%s\n", msg, str);
  exit (1);
}

void rx1 (char msg[], char c)
{
  printf ("\n+++ %s%c\n", msg, c);
  exit (1);
}

void rxi (msg, i)
char msg[];
int i;
{
  printf ("\n+++ %s%d\n", msg, i);
  exit (1);
}

/* ----- bug: print message for internal error and maybe stop -----  */
void bug (msg,fatal)
char msg[];
int fatal;
{
  printf ("\n\nThis cannot happen!");
  if (strlen(msg)>0) printf ("\nInternal error: %s.\n", msg);
  if (fatal) {
    printf ("Emergency stop.\n\n");
    exit (1);
  }
  else {
    printf ("Trying to continue...\n\n");
  }
}

/* ----- ranf(x1,x2): return random float between x1 and x2 --- */
float ranf(float x1, float x2)
{
  static int m=259200;   /* generator constants */
  static int a=421;
  static int c=54773;
  static int j=1;        /* seed */
  float r;

  j=(j*a+c)%m;
  r=x1+(x2-x1)*(double)j/(double)m;
  return r;
}


/* ----- getline ----- */
/*
* Added by jc:
* This routine reads a line from fp into  buf,  and  trims  away  any
* trailing  whitespace.   We  are  paranoid  about whether isspace(c)
* returns true for CR, so this routine should work even if the  input
* came from a DOS system.
*/
char * get_a_line(buf,len,fp)
	char* buf;
	int   len;
	FILE* fp;
{	char* rp;
	int   c, l;
	if ((rp = fgets(buf,len,fp))) {
		l = strlen(buf);
		while ((l > 0) && (((c = buf[l-1]) && isspace(c)) || (c == '\r')))
			buf[--l] = 0;
	}
	return rp;
}


/* ----- strip: remove leading and trailing blanks ----- */
void strip (str1,str)
char str[],str1[];
{
  int l,i,i1,i2;
  l=strlen(str);

  i1=0;
  for (i=0; i<l; i++)
    if ((str[i]!=' ') && (str[i]!='\n')) { i1=i; break; }
  i2=0;
  for (i=l-1; i>=0; i--)
    if ((str[i]!=' ') && (str[i]!='\n')) { i2=i+1; break; }
  for (i=i1;i<i2;i++) str1[i-i1]=str[i];
  str1[i2-i1]=0;
/*  printf (" l=%d i1=%d i2=%d <%s> <%s>\n", l, i1, i2, str, str1);*/
}


/* ----- nwords: count words in string ----- */
int nwords (str)
char *str;
{
  int w,k;
  char *c;
  c=str;
  w=0;
  for(k=0;k<=50;k++) {
    while (*c==' ') c++;
    if (*c=='\0') break;
    w++;
    while ((*c!=' ') && (*c!='\0')) c++;
    if (*c=='\0') break;
  }
  return w;
}



/* ----- getword: return n-th word from string ---- */
int getword (iw,str,str1)
int iw;
char *str,*str1;
{
  int w,k;
  char *c,*cc;
  if (iw<0) { *str1='\0'; return 0;}
  c=str;
  w=0;
  for(k=0;k<=50;k++) {
    while (*c==' ') c++;
    if (*c=='\0') break;
    if (w==iw) {
      cc=str1;
      while ((*c!=' ')&&(*c!='\0')) { *cc=*c; c++; cc++; }
      *cc='\0';
      return 1;
    }
    w++;
    while ((*c!=' ') && (*c!='\0')) c++;
    if (*c=='\0') break;
  }
  *str1='\0';
  return 0;
}


/* ----- abbrev: check for valid abbreviation ----- */
int abbrev (str,ab,nchar)
char str[],ab[];
int nchar;
{
  int i,nc;
  if (strlen(str) > strlen(ab)) return 0;
  nc=strlen(str);
  if (nc<nchar) nc=nchar;
  for (i=0;i<nc;i++) if (str[i] != ab[i]) return 0;
  return 1;
}

/* ----- strext: set extension on a file identifier ----- */
/*  force=1 forces change even if fid already has an extension */
/*  force=0 does not change the extension if there already is one */
void strext (fid1, fid, ext, force)
char fid[],ext[],fid1[];
int force;
{
  int i,l;
  char *p,*q;

  strcpy (fid1, fid);
  l=strlen(fid1);
  p=fid1;
  for (i=0;i<l;i++)
    if (fid1[i]=='/') p=fid1+i;

  if (!force) {
    q=strchr(p,'.');
    if (q && (q!=fid1+strlen(fid1)-1)) return;
  }
  if (!strchr(p,'.')) strcat (fid1,".");
  q=strchr(p,'.');
  if (strlen(ext)>0) q++;
  *q = 0;
  strcat(fid1,ext);

}

/* ----- cutext: cut off extension on a file identifier ----- */
void cutext (fid)
char fid[];
{
  int i,l;

  l=strlen(fid);
  for (i=0;i<l;i++)
    if (fid[i]=='.') fid[i]='\0';
}

/* ----- getext: get extension on a file identifier ----- */
void getext (fid,ext)
char fid[],ext[];
{
  int i,l,k;

  l=strlen(fid);
  k=l-1;
  for (i=0;i<l;i++)
    if (fid[i]=='.') k=i;

  for (i=k+1;i<l;i++)
    ext[i-k-1]=fid[i];
  ext[l-k-1]='\0';

}


/* ----- sscanu ----- */
float scan_u(str)
char str[];
{
  char unit[81];
  float a,b;

  strcpy(unit,"pt");
  sscanf(str,"%f%s", &a, unit);

  if      (!strcmp(unit,"cm")) b=a*CM;
  else if (!strcmp(unit,"in")) b=a*IN;
  else if (!strcmp(unit,"pt")) b=a*PT;
  else {
    printf ("+++ Unknown unit \"%s\" in: %s\n",unit,str);
    exit (3);
  }
  return b;
}


/* ----- match ------- */
int match (str, pat)
char str[], pat[];
{
  char *p,*s;
  p=pat;
  s=str;

  if (strlen(pat)==0) return 1;

  while (*p != 0) {

    if (*p == '*') {           /* found wildcard '*' in pattern */
      p++;
      while (*p == '*') p++;
      if (*p == 0) return 1;   /* trailing '*' matches all */
      for (;;) {               /* find match to char after '*' */
        if (*s == 0) return 0;
        if ((*s == *p) || (*p == '+'))
          if (match(s+1,p+1)) return 1;   /* ok if rest matches */
        s++;
      }
    }

    else {                     /* no wildcard -- char must match */
      if (*s == 0) return 0;
      if ((*p != *s) && (*p != '+')) return 0;
      s++;
    }
    p++;
  }

  if (*s != 0) return 0;       /* pattern but not string exhausted */
  return 1;
}

/* ----- isblankstr: check for blank string ---- */
int isblankstr (str)
char str[];
{
  int i;
  for (i=0;i<strlen(str);i++) if (str[i] != ' ') return 0;
  return 1;
}

/* ----- cap_str: capitalize a string ----- */
void cap_str(str)
char str[];
{
  char *c;
  for (c=str; *c!='\0'; c++)
	*c = toupper(*c);
}


/* ----- cwid ----- */
/*  These are char widths for Times-Roman */
float cwid(char c)
{
  float w;
  if      (c=='a') w=44.4;
  else if (c=='b') w=50.0;
  else if (c=='c') w=44.4;
  else if (c=='d') w=50.0;
  else if (c=='e') w=44.4;
  else if (c=='f') w=33.3;
  else if (c=='g') w=50.0;
  else if (c=='h') w=50.0;
  else if (c=='i') w=27.8;
  else if (c=='j') w=27.8;
  else if (c=='k') w=50.0;
  else if (c=='l') w=27.8;
  else if (c=='m') w=77.8;
  else if (c=='n') w=50.0;
  else if (c=='o') w=50.0;
  else if (c=='p') w=50.0;
  else if (c=='q') w=50.0;
  else if (c=='r') w=33.3;
  else if (c=='s') w=38.9;
  else if (c=='t') w=27.8;
  else if (c=='u') w=50.0;
  else if (c=='v') w=50.0;
  else if (c=='w') w=72.2;
  else if (c=='x') w=50.0;
  else if (c=='y') w=50.0;
  else if (c=='z') w=44.4;

  else if (c=='A') w=72.2;
  else if (c=='B') w=66.7;
  else if (c=='C') w=66.7;
  else if (c=='D') w=72.2;
  else if (c=='E') w=61.1;
  else if (c=='F') w=55.6;
  else if (c=='G') w=72.2;
  else if (c=='H') w=72.2;
  else if (c=='I') w=33.3;
  else if (c=='J') w=38.9;
  else if (c=='K') w=72.2;
  else if (c=='L') w=61.1;
  else if (c=='M') w=88.9;
  else if (c=='N') w=72.2;
  else if (c=='O') w=72.2;
  else if (c=='P') w=55.6;
  else if (c=='Q') w=72.2;
  else if (c=='R') w=66.7;
  else if (c=='S') w=55.6;
  else if (c=='T') w=61.1;
  else if (c=='U') w=72.2;
  else if (c=='V') w=72.2;
  else if (c=='W') w=94.4;
  else if (c=='X') w=72.2;
  else if (c=='Y') w=72.2;
  else if (c=='Z') w=61.1;

  else if (c=='0') w=50.0;
  else if (c=='1') w=50.0;
  else if (c=='2') w=50.0;
  else if (c=='3') w=50.0;
  else if (c=='4') w=50.0;
  else if (c=='5') w=50.0;
  else if (c=='6') w=50.0;
  else if (c=='7') w=50.0;
  else if (c=='8') w=50.0;
  else if (c=='9') w=50.0;

  else if (c=='~') w=54.1;
  else if (c=='!') w=33.3;
  else if (c=='@') w=92.1;
  else if (c=='#') w=50.0;
  else if (c=='$') w=50.0;
  else if (c=='%') w=83.3;
  else if (c=='^') w=46.9;
  else if (c=='&') w=77.8;
  else if (c=='*') w=50.0;
  else if (c=='(') w=33.3;
  else if (c==')') w=33.3;
/*|   else if (c=='-') w=33.3; |*/
  else if (c=='-') w=40.0;
  else if (c=='_') w=50.0;
  else if (c=='+') w=56.4;
  else if (c=='=') w=55.0;
  else if (c=='[') w=33.3;
  else if (c==']') w=33.3;
  else if (c=='{') w=48.0;
  else if (c=='}') w=48.0;
  else if (c=='|') w=20.0;
  else if (c==':') w=27.8;
  else if (c==';') w=27.8;
  else if (c=='.') w=27.8;
  else if (c==',') w=27.8;
  else if (c=='\\') w=27.8;
  else if (c=='\'') w=33.3;
  else if (c=='\"') w=40.8;
  else if (c=='<') w=56.4;
  else if (c=='>') w=56.4;
  else if (c=='?') w=44.4;
  else if (c=='/') w=27.8;
  else if (c=='`') w=33.3;
  else if (c==' ') w=25.0;
  else             w=50.0;
  return w/100.0;
}


/* ----- get_file_size ------- */
/* version using standard function stat */
#include <sys/stat.h>
int get_file_size (fname)
char fname[];
{
  int m,rc;
  struct stat statbuf;
  rc = stat(fname,&statbuf);
  if (rc == -1) {
    printf ("Unsuccessful call to stat for file %s\n", fname);
    return -1;
  }
  m=statbuf.st_size;
  return m;
}

/* version which counts bytes by hand */
int get_file_size1 (fname)
char fname[];
{
  int m,i;
  FILE *fp;

  if ((fp = fopen (fname,"r")) == NULL) {
    printf ("Cannot open file to determine size: %s", fname);
    return -1;
  }

  m=0;
  i=getc(fp);
  while (i != EOF) {
    m++;
    i=getc(fp);
  }
  fclose (fp);
  return m;
}

