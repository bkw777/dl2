/*
DeskLink+
Extensions and enhancements Copyright (C) 2005 John R. Hogerhuis

DeskLink+ is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 or any
later as version as published by the Free Software Foundation.  

DeskLink+ is distributed in the hope that it will be useful, but
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

typedef unsigned LocalID;
typedef unsigned char Char;
typedef int Err;

int file_list_init ();
int file_list_cleanup();
   
int add_file (Char *namep, u_int32_t len, LocalID dbID);
void file_list_clear_all ();
Char **get_str_table(u_int16_t *lenp);
   
Err get_first_file (Char *namep, u_int32_t *lenp, LocalID *dbIDp);
Err get_next_file (Char *namep, u_int32_t *lenp, LocalID *dbIDp);
Err get_prev_file (Char *namep, u_int32_t *lenp, LocalID *dbIDp);

Err find_file (Char *find_namep, u_int32_t *lenp, LocalID *dbIDp);
Err addto_file_len (u_int32_t len_delta);
Err delete_file (LocalID dbID);
Err rename_file (LocalID dbID, Char *namep);

#endif
