/*
DeskLink++
Extensions and enhancements Copyright (C) 2005 John R. Hogerhuis
Copyright (c) 2022 Gabriele Gorla

DeskLink++ is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 or any
later as version as published by the Free Software Foundation.  

DeskLink++ is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (in the file "COPYING"); if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111, USA.
*/


#ifndef DIR_LIST
#define DIR_LIST

#include <stdint.h>

#define QUANTUM 10
#define FNAME_MAX 32

#define DIR_FLAG 0x01


//typedef unsigned LocalID;
//typedef int Err;

typedef struct
{
  char     tsname[12];  // ts-dos file name
  char     ufname[FNAME_MAX];  // unix file name
  u_int32_t len;        // length
  u_int8_t  flags;
} FILE_ENTRY;


int file_list_init ();
int file_list_cleanup();

int add_file (FILE_ENTRY *nfe);
void file_list_clear_all ();

FILE_ENTRY * find_file (char *tsname);
FILE_ENTRY * get_first_file (void);
FILE_ENTRY * get_next_file (void);
FILE_ENTRY * get_prev_file (void);

#endif
