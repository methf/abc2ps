#ifndef _utilH
#define _utilH
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

void wng (char msg[], char str[]);

void rx (char msg[], char str[]);

void rx1 (char msg[], char c);

void rxi (char msg[], int i);

void bug (char msg[], int fatal);

float ranf(float x1, float x2);

char * get_a_line(char *buf, int len, FILE *fp);

void strip (char str1[], char str[]);

int nwords (char *str);

int getword (int iw, char *str, char *str1);

int abbrev (char str[], char ab[], int nchar);

void strext (char fid1[], char fid[], char ext[], int force);

void cutext (char fid[]);

void getext (char fid[], char ext[]);

float scan_u(char str[]);

int match (char str[], char pat[]);

int isblankstr(char str[]);

void cap_str(char str[]);

float cwid(char c);

int get_file_size (char fname[]);

int get_file_size1 (char fname[]);

#endif _utilH
