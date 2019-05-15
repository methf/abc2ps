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

#define NMAX 5001

#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[])
{
  int i,n,p,m;
  float x1,x2,dx,x,f1,f2,f,a,x0,w,gg,c;
  float pi = 3.1415926;
  float s1[NMAX],s2[NMAX],s[NMAX],t[NMAX],g[NMAX];

  w=0.2;

  a=1/w;
  x1=-5;
  x2=+5;
  n=501;
  m=0;
  c = sqrt(pi/a);

  dx=(x2-x1)/(n-1);


  for (i=0;i<n;i++) {
    t[i]=i*dx+x1;
    g[i]=s1[i]=s2[i]=s[i]=0;
  }

  for (p=-m; p<=m; p++) {
    for (i=0;i<n;i++) {
      x=t[i];
      f1=2*(x-p)*c*(erf(a*(2*x-2*p+1))-erf(a*(2*x-2*p-1)));
      f2=(0.5/(a*a))*( exp(-a*a*(2*x-2*p+1)*(2*x-2*p+1))
                      -exp(-a*a*(2*x-2*p-1)*(2*x-2*p-1)));

      s1[i] += f1;
      s2[i] += f2;
      s[i]  += f1+f2;

/*|       gg = -2*c*erf(x-x0+1); |*/
/*|       g[i] += gg; |*/

    }

  }

/*|   for (i=0;i<n;i++) {  |*/
/*|     f1=(x+2*m)*c*erf(a*(x+2*m+1));  |*/
/*|     f2=(x-2*m)*c*erf(a*(x-2*m-1));  |*/
/*|     s2[i] = f1-f2;  |*/
/*|   }    |*/

  for (i=0;i<n;i++) {
    printf ("%10.5f %10.5f %10.5f %10.5f\n", t[i],s1[i],s2[i],s2[i]);

  }

  return 0;


}

