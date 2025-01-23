/*
DeskLink2
Extensions and enhancements Copyright (C) 2005 John R. Hogerhuis
Copyright (c) 2022 Gabriele Gorla

DeskLink2 is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 or any
later as version as published by the Free Software Foundation.  

DeskLink2 is distributed in the hope that it will be useful, but
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
#include "constants.h"

typedef struct {
	char     client_fname[TPDD_FILENAME_LEN+1];
	char     local_fname[LOCAL_FILENAME_MAX+1];
	uint8_t  attr;
	uint16_t len;
	uint8_t  flags;
} FILE_ENTRY;

int file_list_init ();
int file_list_cleanup ();

void file_list_clear_all ();
int  add_file (FILE_ENTRY* fe);

FILE_ENTRY* find_file (char* client_fname, uint8_t attr);
FILE_ENTRY* get_first_file (void);
FILE_ENTRY* get_next_file (void);
FILE_ENTRY* get_prev_file (void);

#endif
