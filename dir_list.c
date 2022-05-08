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

#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include "dir_list.h"


typedef struct
{
   Char     name[12] ;  // file name
   u_int32_t len      ;  // length
   LocalID  dbID     ;  // database ID
} FILE_ENTRY;

#define MAX(a,b) ((a)>(b)?(a):(b))

static u_int16_t allocated;
static u_int16_t ndx;
static int16_t  cur; // current must be signed (get-prev)
static FILE_ENTRY *tblp = 0;

static Err get_current_record (Char *namep, u_int32_t *lenp, LocalID *dbIDp);

int file_list_init ()
{
   tblp = malloc (sizeof (FILE_ENTRY) * QUANTUM + sizeof (Char *) * QUANTUM);
   if (!tblp)
      return (-1);
   allocated = QUANTUM;
   ndx = 0;
   cur = 0;

   return (0);
}

int file_list_cleanup()
{


   allocated = 0;
   ndx = 0;
   cur = 0;
   if (tblp)
      free (tblp);
   tblp = NULL;

   return (0);
}

void file_list_clear_all ()
{
   cur = ndx = 0;
}
   

Err find_file (Char *find_namep, u_int32_t *lenp, LocalID *dbIDp)
{

   for (cur = 0; cur < ndx; cur++)
      if (strncasecmp ((char *) find_namep, (char *) tblp[cur].name, sizeof (tblp[cur].name) - 1) == 0)
      {
         if (lenp)  *lenp  = tblp[cur].len;
         if (dbIDp) *dbIDp = tblp[cur].dbID;
         return (0);
      }

   return (-1);
}

Err addto_file_len (u_int32_t len_delta)
{
   tblp[cur].len += len_delta;

   return (0);
}

Err delete_file (LocalID dbID)
{
   u_int16_t i, j;
   FILE_ENTRY *ep;
   Char **cpp;
   
   /** find the entry */
   for (i = 0, ep = tblp; i < ndx; i++, ep++)
      if (ep->dbID == dbID) break;

   /** no matching entry; return error */
   if (i >= ndx) return (-1);

   /** move up all entries to cover */
   memmove (ep, ep + 1, (ndx - i - 1) * sizeof (FILE_ENTRY));
   
   /** adjust indices */
   ndx--;
   if (cur > i) cur--;

   /** correct trailing pointers */
   cpp = (Char **) (tblp + allocated);
   for (j = i; j < ndx; j++)
       cpp[j] = tblp[j].name;
   cpp[ndx] = NULL;
  
   return (0);

}

Err rename_file (LocalID dbID, Char *namep)
{
   u_int16_t i;
   FILE_ENTRY *ep;

   /** find the entry */
   for (i = 0, ep = tblp; i < ndx; i++, ep++)
      if (ep->dbID == dbID) break;

   /** no matching entry; return error */
   if (i >= ndx) return (-1);

   /** update the name */
   strncpy ((char *) ep->name, (char *) namep, sizeof (ep->name) * sizeof (Char) - 1);

   return (0);
}

int add_file (Char *namep, u_int32_t len, LocalID dbID)
{
   FILE_ENTRY *ep;
   Char **cpp;
   int i;

   /** reallocate QUANTUM more records if out of space */
   if (ndx >= allocated)
   {

      /** resize the array */
      tblp = realloc (tblp, (allocated + QUANTUM) * (sizeof (FILE_ENTRY) + sizeof (Char *)));
      if (!tblp) return (-1);
      allocated += QUANTUM;

      /** update the char *  array */
      for (i = 0, ep = tblp, cpp = (Char **) (tblp + allocated); i < ndx;
            i++, ep++, cpp++)
         *cpp = ep->name;
   }

   /** reference the entry */
   if (!tblp) return (-1);
   ep = tblp + ndx;

   /** fill the entry */
   strncpy ((char *) ep->name, (char *)namep, sizeof (ep->name) * sizeof (Char) - 1);
   ep->len = len;
   ep->dbID = dbID;

   /** set the pointer for use in filling list controls */
   ((Char **) (tblp + allocated))[ndx] = ep->name;

   /** adjust cur to address this record, ndx to next avail */
   cur = ndx;
   ndx += 1;

   return (0);
}

Char **get_str_table(u_int16_t *lenp)
{
   /** valchk */
   if (!tblp)
   {
      *lenp = 0;
      return (NULL);
   }

   /** return string table */
   if (lenp) 
      *lenp = ndx;
   
   return ((Char **) (tblp + allocated));
}


Err get_first_file (Char *namep, u_int32_t *lenp, LocalID *dbIDp)
{
   Err err;

   /** reset cur, get, next */
   cur = 0;
   if ((err = get_current_record (namep, lenp, dbIDp))) return (err);

   return (0);
}

Err get_next_file (Char *namep, u_int32_t *lenp, LocalID *dbIDp)
{
   Err err;

   /** return error if out-of-range */
   if (cur + 1 > ndx)
      return (-1);

   /** get, next */
   cur++;
   if ((err = get_current_record (namep, lenp, dbIDp))) return (err);

   return (0);
}
   
Err get_prev_file (Char *namep, u_int32_t *lenp, LocalID *dbIDp)
{
   Err err;

   /** move back by one, but floor at -1 */
   cur = MAX (cur -1, -1);
   
   /** ensure don't go off the shallow end */
   if (cur < 0) return (-1);

   /** get */
   if ((err = get_current_record (namep, lenp, dbIDp))) return (err);

   return (0);
}

static Err get_current_record (Char *namep, u_int32_t *lenp, LocalID *dbIDp)
{
    FILE_ENTRY *ep;
   
   /** return error if out-of-range */
   if (cur >= ndx) return (-1);

   /** reference the record, fill in caller's vars */
   if (!tblp) return (-1);
   ep = tblp + cur;
   if (namep) strcpy ((char *) namep, (char *) ep->name);
   if (lenp) *lenp = ep->len;
   if (dbIDp) *dbIDp = ep->dbID;
   
   return (0);
}
