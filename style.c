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
#include "style.h"

/*   Global style parameters for the note spacing and Knuthian glue. */

/*   Parameters here are used to set spaces around notes.
     Names ending in p: prefered natural spacings
     Names ending in x: expanded spacings
     Units: everything is based on a staff which is 24 points high
     (ie. 6 pt between two staff lines). */

/* ----- set_style_pars: set the parameters which control note spacing ---- */
void set_style_pars (float strictness)
{
  float y;

  f0p=0.10;     f0x=0.15;
  f5p=0.60;     f5x=0.7;
  f1p=1.0;      f1x=1.0;

  lnnp=40;      lnnx=90;
  bnnp=1.0;     bnnx=1.0;
  fnnp=1.0;     fnnx=1.0;

  lbnp=30;      lbnx=50;
  bbnp=0.2;     bbnx=0.2;
  rbnp=0.12;    rbnx=0.10;

  lnbp=30;      lnbx=55;
  bnbp=0.9;     bnbx=0.9;
  rnbp=0.10;    rnbx=0.1;

  /* set parameters f0p,f1p according to strictness */
  /* the y*y makes the scale a bit more intuitive .. */
  y=1-strictness;
  f0p=y*y*0.40;
  f0x=y*y*0.60;
  if (verbose>3)
    printf ("Set style parameters, strictness %.2f (f0p=%.3f, f0x=%.3f)\n",
            strictness,f0p,f0x);

}


/* ----- Function of the spacing parameters ----- */

/* Parameters for the length-to-width mapping:
   f0p, f5p, f1p are the return values for notes of zero duration,
   half notes, and whole notes. A parabolic interpolation is
   used for other note durations. The purpose is to allow a non-linear
   relation between the note duration and the spacing on the paper.

   Parameters for the note-note spacing:
   (the internote spacing between two notes that follow
   each other without a bar in between.)

   - lnn is an overall multiplier, i.e. the final note width in points
     is the return value of function nwid times lnn.
   - bnn determines how strongly the first note enters into the spacing.
     For bnn=1, the spacing is calculated using the first note.
     For bnn=0, the spacing is the average for the two notes.
   - fnn multiplies the spacing under a beam, to compress the notes a bit

   Parameters for the bar-note spacing:
   (the spacing between a bar and note at the measure start.)

   - lbn is the overall multiplier for the return values from nwid.
   - rbn is the note duration which defines the default spacing.
   - bbn determines how strongly the note duration enters into the spacing.
     For bbn=1, the spacing is lbn times the return value of nwid.
     For bbn=0, the spacing is lbn times the width of rbn times timesig.

     Parameters for the note-bar spacing:
     (the spacing between a note at the measure end and the bar.)

   - lnb is the overall multiplier for the return values from nwid.
   - rnb is the note duration which defines the default spacing.
   - bnb determines how strongly the note duration enters into the spacing.
     For bnb=1, the spacing is lnb times the return value of nwid.
     For bnb=0, the spacing is lnb times the width of rbn times timesig. */
