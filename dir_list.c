/*
DeskLink+
Extensions and enhancements Copyright (C) 2005 John R. Hogerhuis
Copyright (c) 2022 Gabriele Gorla

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

#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include "dir_list.h"

static uint16_t allocated;
static uint16_t ndx;
static uint16_t cur;
static FILE_ENTRY* tblp = 0;

static FILE_ENTRY* current_record (void);

int file_list_init () {
	tblp = malloc(sizeof (FILE_ENTRY) * FEQ );
	if (!tblp) return -1;
	allocated = FEQ;
	ndx = 0;
	cur = 0;
	return 0;
}

int file_list_cleanup() {
	allocated = 0;
	ndx = 0;
	cur = 0;
	if (tblp) free(tblp);
	tblp = NULL;
	return 0;
}

void file_list_clear_all () {
	cur = ndx = 0;
}
   
int add_file (FILE_ENTRY* fe) {
	/* allocate FEQ more records if out of space */
	if (ndx >= allocated) {
		/* resize the array */
		tblp = realloc(tblp, (allocated + FEQ) * sizeof (FILE_ENTRY) );
		if (!tblp) return -1;
		allocated += FEQ;
	}

	/* reference the entry */
	if (!tblp) return -1;

	memcpy(tblp+ndx, fe, sizeof(FILE_ENTRY));
	/* adjust cur to address this record, ndx to next avail */
	cur = ndx;
	ndx++;

	return 0;
}

FILE_ENTRY* find_file (char* client_fname) {
	int i;
	for (i=0;i<ndx;i++) {
		if (strcmp(client_fname,tblp[i].client_fname)==0) return &tblp[i];
	}
	return 0;
}

FILE_ENTRY* get_first_file (void) {
	cur = 0;
	return current_record();
}

FILE_ENTRY* get_next_file (void) {
	if (cur + 1 > ndx) return NULL;
	cur++;
	return current_record();
}
   
FILE_ENTRY* get_prev_file (void) {
	if (cur==0) return NULL;
	cur--;
	return current_record();
}

static FILE_ENTRY* current_record (void) {
	FILE_ENTRY* ep;
	if (cur >= ndx) return NULL;
	if (!tblp) return NULL;
	ep = tblp + cur;
	return ep;
}
