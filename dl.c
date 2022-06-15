/*
 * DeskLink for *nix (dl)
 * Copyright (C) 2004
 * Stephen Hurd
 *
 * Redistribution of modified and unmodified copies
 * is premitted provided the copyright remains intact
 */

/*
DeskLink+
2005     John R. Hogerhuis Extensions and enhancements
2019     Brian K. White - repackaging, reorganizing, bootstrap function
         Kurt McCullum - TS-DOS loaders
2022     Gabriele Gorla - Add support for TS-DOS subdirectories

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

/* Some basic info about TPDD protocol formatting that explains 
 * some frequent idioms in here. TPDD Operation-mode transactions, both
 * commands issued by the client, and responses issued by the server,
 * have this general form:
 * 
 * type     - 1 byte         the format or type of this packet
 * length   - 1 byte         number of bytes that come next
 * payload  - length bytes   range is 0-128
 * checksum - 1 byte         includes type, length, and payload
 * 
 * Most functions pass around a buffer containing this entire
 * structure, often minus the checksum. checksum() itself
 * takes this as input for instance.
 * 
 * Frequently a buffer will be declared with a SIZE+3, which is
 * SIZE will be a pertinent payload size of a given command,
 * like 128 for the max possible, or 11 for a DME message, etc,
 * and the +3 is 3 extra bytes for type, length, and checksum.
 * 
 * Similarly, most functions include frequent references to these
 * byte offsets gb[0], gb[1], gb[2], gb+2, gb[gb[1]+2].
 * 
 * functions named req_*() receive a command in this format
 * functions named ret_*() generate a response in this format
 * 
 * There is also an FDC-mode that TPDD1/FB-100 drives have, which has
 * a completely different format. This program only implements
 * Operation-mode. TPDD2 drives do not have FDC-mode, but they do have
 * extra Operation-mode commands that TPDD1 does not have,
 * some of which this program does implement.
 * 
 * See the ref/ directory for more details, including a copy of the
 * TPDD1 software manual. There is no TPDD2 software manual known yet.
 */

#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "constants.h"
#include "dir_list.h"

#if defined(__darwin__)
#include <util.h>
#endif

#if defined(__FreeBSD__)
#include <libutil.h>
#endif

#if defined(__NetBSD__) || defined(OpenBSD)
#include <util.h>
#endif

#if defined(__linux__)
#include <utmp.h>
#include <netinet/in.h>
#endif

#define STRINGIFY2(X) #X
#define S_(X) STRINGIFY2(X)

/*** config **************************************************/

#ifndef APP_LIB_DIR
#define APP_LIB_DIR .
#endif

#ifndef DEFAULT_CLIENT_TTY
#define DEFAULT_CLIENT_TTY ttyS0
#endif

#ifndef DEFAULT_CLIENT_BAUD
#define DEFAULT_CLIENT_BAUD B19200
#endif

// most things get away with 5 here.
// rxcini.do for REXCPM setup, requires 6.
// new high-water mark: TS-DOS.200 requires 7.
// If this is 6, a "?" (print) on line 3 gets dropped every time.
#define DEFAULT_BASIC_BYTE_MS 7

#define DEFAULT_TPDD_FILE_ATTR 0x46 // F

// These defaults are the same as what the original Desk-Link does.
// But you can change them to pretty much anything. The parent label is
// picky because whatever you use has to look like a valid filename
// to ts-dos. ".." doesn't work, but "^" does for instance.
#define DEFAULT_DME_ROOT_LABEL   " ROOT " // ROOT_LABEL='0:'  '-root-'
#define DEFAULT_DME_PARENT_LABEL "PARENT" // PARENT_LABEL'^:' '-back-'
// this you can't change unless you also hack ts-dos
#define DEFAULT_DME_DIR_LABEL    "<>"     // DIR_LABEL='/'

// Support for Ultimate ROM-II TS-DOS loader: see ref/ur2.txt
// files from share root dir that are always readable in any cd path
// TODO
// * put these in an array
// * allow user to specify an arbitrary list of magic files
// * 3 or more search paths: cwd, share-root, ~/.local/app, app-lib
//   any file in the list is searched first in the current dir,
//   then in the root shared dir, then in a specified lib/share dir.
#define DOS100 "DOS100.CO"
#define DOS200 "DOS200.CO"
#define DOSNEC "DOSNEC.CO"
//#define DOSK85 "DOSK85.CO" // probably don't exist
//#define DOSM10 "DOSM10.CO"

// termios VMIN & VTIME
#define C_CC_VMIN 1
#define C_CC_VTIME 5

/*************************************************************/

// configuration
int debug = 0;
bool upcase = false;
bool rtscts = false;
unsigned dot_offset = 6; // 0 for raw, 6 for KC-85, 8 for WP-2
int client_baud = DEFAULT_CLIENT_BAUD;
int BASIC_byte_us = DEFAULT_BASIC_BYTE_MS*1000;
char dme_root_label[7] = DEFAULT_DME_ROOT_LABEL;
char dme_parent_label[7] = DEFAULT_DME_PARENT_LABEL;
char dme_dir_label[3] = DEFAULT_DME_DIR_LABEL;
char default_attr = DEFAULT_TPDD_FILE_ATTR;
bool enable_ur2_dos_hack = true;
bool getty_mode = false;
bool bootstrap_mode = false;

// globals
char **args;
int f_open_mode = F_OPEN_NONE;
int client_tty_fd = -1;
struct termios client_termios;
int o_file_h = -1;
unsigned char gb[TPDD_DATA_MAX+3];
char cwd[PATH_MAX] = {0x00};
char dme_cwd[7] = DEFAULT_DME_ROOT_LABEL;
char client_tty_name[PATH_MAX];
char bootstrap_file[PATH_MAX] = {0x00};
int opr_mode = 1; // 0=FDC-mode 1=Operation-mode
bool dme_detected = false;
bool dme_fdc = false;
bool dme_disabled = false;
char ch[2] = {0xFF};

FILE_ENTRY *cur_file;
int dir_depth=0;

// blarghamagargle
void ret_std(unsigned char err);

/* primitives and utilities */

// (verbosity_threshold, printf_format , args...)
// dbg(3,"err %02X",err); // means only show this message if debug>=3
void dbg( const int v, const char* format, ... ) {
	if (debug<v) return;
	va_list args;
	va_start( args, format );
	vfprintf( stderr, format, args );
	fflush(stderr);
	va_end( args );
}

// (verbosity_threshold, buffer , len)
// dbg_b(3, b , 24); // like dbg() except
// print the buffer as hex pairs with a single trailing newline
// if len<0, then assume the max tpdd buffer TPDD_DATA_MAX+3 (131)
void dbg_b(const int v, unsigned char *b, int n) {
	if (debug<v) return;
	unsigned i;
	if (n<0) n = TPDD_DATA_MAX+3;
	for (i=0;i<n;i++) fprintf (stderr,"%02X ",b[i]);
	fprintf (stderr, "\n");
	fflush(stderr);
}

// like dbg_b, except assume the buffer is a tpdd Operation-mode
// block and parse it to display cmd, len payload, checksum.
// length is read from the data itself
void dbg_p(const int v, unsigned char *b) {
	dbg(v,"cmd: %1$02X\nlen: %2$02X (%2$u)\nchk: %3$02X\ndat: ",b[0],b[1],b[b[1]+2]);
	dbg_b(v,b+2,b[1]);
}

void set_baud (char * s) {
	client_baud=atoi(s)==9600?B9600:B19200;
}

// set termios VMIN & VTIME
void client_tty_vmt(int m,int t) {
	if (m<-1 || t<-1) tcgetattr(client_tty_fd,&client_termios);
	if (m<0) m = C_CC_VMIN;
	if (t<0) t = C_CC_VTIME;
	if (client_termios.c_cc[VMIN] == m && client_termios.c_cc[VTIME] == t) return;
	client_termios.c_cc[VMIN] = m;
	client_termios.c_cc[VTIME] = t;
	tcsetattr(client_tty_fd,TCSANOW,&client_termios);
}

int write_client_tty(void *b, int n) {
	dbg(4,"%s(%u)\n",__func__,n);
	n = write(client_tty_fd,b,n);
	dbg(3,"SENT: "); dbg_b(3,b,n);
	return n;
}

// it's correct that this waits forever
// the one time we don't want to block, we don't use this
int read_client_tty(void *b, const unsigned int n) {
	dbg(4,"%s(%u)\n",__func__,n);
	unsigned t = 0;
	int i = 0;
	while (t<n) if ((i = read(client_tty_fd, b+t, n-t))) t+=i;
	dbg(3,"RCVD: "); dbg_b(3,b,n);
	return t;
}

// cat a file to terminal, for bootstrap directions
void cat(char *f) {
	char b[4097];
	int h;
	if ((h=open(f,O_RDONLY))<0) return;
	while (read(h,&b,4096)>0) printf("%s",b);
	close(h);
}

// b[0] = cmd
// b[1] = len
// b[2] to b[1+len] = 0 to 128 bytes of payload
// ignore everything after b[1+len]
unsigned char checksum(unsigned char *b) {
	unsigned short s=0;
	for (int i=0;i<2+b[1];i++) s+=b[i];
	return ((s&0xFF)^0xFF);
}

char *collapse_padded_name(char *fname) {
	dbg(3,"%s(\"%s\")\n",__func__,fname);
	if (!dot_offset) return fname;

	int i;
	for (i=dot_offset;i>1;i--) if (fname[i-1]!=' ') break;

	if (fname[dot_offset+1]==dme_dir_label[0] && fname[dot_offset+2]==dme_dir_label[1]) {
		fname[i]=0x00;
	} else {
		fname[i]=fname[dot_offset];
		fname[i+1]=fname[dot_offset+1];
		fname[i+2]=fname[dot_offset+2];
		fname[i+3]=0x00;
	}
	return fname;
}

void lsx (char *path,char *match) {
	struct dirent *files;
	DIR *dir = opendir(path);
	int i;
	if (dir == NULL){dbg(0,"Cannot open \"%s\"",path); return;}

	while ((files = readdir(dir)) != NULL) {
		for (i=strlen(files->d_name);files->d_name[i]!='.';i--);
		if (files->d_name[i+1]==match[0] && files->d_name[i+2]==match[1] && files->d_name[i+3]==match[2])
			dbg(0," %s",files->d_name);
	}

	closedir(dir);
}

int do_magic_file(char *b) {
	dbg(3,"%s(\"%s\")\n",__func__,b);
	if (!enable_ur2_dos_hack) return 1;
	if (!dir_depth) return 1; // fake root hack not needed in actual root
	if (dot_offset!=6) return 1; // There's no UR2 for WP-2 or CP/M etc
	if (strncmp(b,DOS100,9)==0) return 0;
	if (strncmp(b,DOS200,9)==0) return 0;
	if (strncmp(b,DOSNEC,9)==0) return 0;
	return 1;
}

FILE_ENTRY *make_file_entry(char *namep, u_int32_t len, u_int8_t flags)
{
	dbg(3,"%s(\"%s\")\n",__func__,namep);
	static FILE_ENTRY f;
	int i;

	/** fill the entry */
	strncpy (f.local_fname, namep, sizeof (f.local_fname) - 1);
	memset(f.client_fname,0x20,TPDD_FILENAME_LEN);
	f.len = len;
	f.flags = flags;

	if (dot_offset) {
		// if not in raw mode, reformat the client filename

		// find the last dot in the local filename
		for(i=strlen(namep);i>0;i--) if (namep[i]=='.') break;

		// write client extension
		if (flags&DIR_FLAG) {
			// directory - put TS-DOS DME ext on client fname
			f.client_fname[dot_offset+1]=dme_dir_label[0];
			f.client_fname[dot_offset+2]=dme_dir_label[1];
			f.len=0;
		} else if (i>0) {
			// file - put first 2 bytes of ext on client fname
			f.client_fname[dot_offset+1]=namep[i+1];
			f.client_fname[dot_offset+2]=namep[i+2];
		}

		// replace ".." with dme_parent_label
		if (f.local_fname[0]=='.' && f.local_fname[1]=='.') {
			memcpy (f.client_fname, dme_parent_label, 6);
		} else {
			for(i=0;i<dot_offset && i<strlen(namep) && namep[i]; i++) {
				if (namep[i]=='.') break;
				f.client_fname[i]=namep[i];
			}
		}

		f.client_fname[dot_offset]='.';
		f.client_fname[dot_offset+3]=0x00;
		if (upcase) for(i=0;i<TPDD_FILENAME_LEN;i++) f.client_fname[i]=toupper(f.client_fname[i]);

		if (f.client_fname[dot_offset+1]==0x00) f.client_fname[dot_offset+1]=0x20;
		if (f.client_fname[dot_offset+2]==0x00) f.client_fname[dot_offset+2]=0x20;

	} else {
		// raw mode - don't reformat or filter anything
		snprintf(f.client_fname,25,"%-24.24s",namep);
	}

	dbg(1,"\"%s\"\t%s%s\n",f.client_fname,f.local_fname,f.flags==DIR_FLAG?"/":"");
	return &f;
}

int read_next_dirent(DIR *dir,int m) {
	dbg(3,"%s()\n",__func__);
	struct stat st;
	struct dirent *dire;
	int flags;

	if (dir == NULL) {
		dire=NULL;
		dbg(0,"%s(NULL) ???\n",__func__);
		if (m) ret_std(ERR_NO_DISK);
		return 0;
	}

	while ((dire=readdir(dir)) != NULL) {
		flags=0;

		if (stat(dire->d_name,&st)) {
			if (m) ret_std(ERR_NO_FILE);
			return 0;
		}

		if (S_ISDIR(st.st_mode)) flags=DIR_FLAG;
		else if (!S_ISREG (st.st_mode)) continue;

		if (flags==DIR_FLAG && !dme_detected) continue;

		if (dot_offset) {
			if (dire->d_name[0]=='.') continue; // skip "." ".." and hidden files
			if (strlen(dire->d_name)>LOCAL_FILENAME_MAX) continue; // skip long filenames
		}

		add_file(make_file_entry(dire->d_name, st.st_size, flags));
		break;
	}

	if (dire == NULL) return 0;

	return 1;
}

void update_file_list(int m) {
	dbg(3,"%s()\n",__func__);
	DIR * dir;

	dir=opendir(".");
	file_list_clear_all();
	dbg(1,"-------------------------------------------------------------------------------\n");
	if (dir_depth) add_file(make_file_entry("..", 0, DIR_FLAG));
	while (read_next_dirent(dir,m));
	dbg(1,"-------------------------------------------------------------------------------\n");
	closedir(dir);
}

////////////////////////////////////////////////////////////////////////
//
//  OPERATION MODE

// standard return - return for: error open close delete status write
void ret_std(unsigned char err)
{
	dbg(3,"%s()\n",__func__);
	gb[0]=RET_STD;
	gb[1]=0x01;
	gb[2]=err;
	gb[3]=checksum(gb);
	dbg(3,"Response: %02X\n",err);
	write_client_tty(gb,4);
	if (gb[2]!=ERR_SUCCESS) dbg(2,"ERROR RESPONSE TO CLIENT\n");
}

// return for dirent
int ret_dirent(FILE_ENTRY *ep)
{
	dbg(2,"%s(\"%s\")\n",__func__,ep->client_fname);
	int i;

	memset(gb,0x00,TPDD_DATA_MAX+3);
	gb[0]=RET_DIRENT;
	gb[1]=LEN_RET_DIRENT;

	if (ep && ep->client_fname) {

		// name
		memset (gb + 2, ' ', TPDD_FILENAME_LEN);
		if (dot_offset) for (i=0;i<dot_offset+3;i++)
			gb[i+2]=(ep->client_fname[i])?ep->client_fname[i]:' ';
		else memcpy (gb+2,ep->client_fname,TPDD_FILENAME_LEN);

		// attribute
		gb[26] = default_attr;

		// size
		gb[27]=(uint8_t)(ep->len >> 0x08); // most significant byte
		gb[28]=(uint8_t)(ep->len & 0xFF);  // least significant byte
	}

	dbg(3,"\"%24.24s\"\n",gb+2);

	gb[29] = TPDD_FREE_SECTORS;
	gb[30] = checksum (gb);

	return (write_client_tty(gb,31) == 31);
}

void dirent_set_name(unsigned char *b) {
	dbg(2,"%s(%-24.24s)\n",__func__,b+2);
	char *p;
	char filename[TPDD_FILENAME_LEN+1]={0x00};
	int f = 0;
	if (b[2]) {
		dbg(3,"filename: \"%-24.24s\"\n",b+2);
		dbg(3,"    attr: \"%c\" (%1$02X)\n",b[26]);
	}
	// update before every set-name for at least 2 reasons
	// * clients may open files without ever listing (teeny, ur2, etc)
	// * local files may be changed at any time by other processes
	update_file_list(ALLOW_RET);
	strncpy(filename,(char *)b+2,TPDD_FILENAME_LEN);
	filename[TPDD_FILENAME_LEN]=0;
	// Remove trailing spaces
	for (p = strrchr(filename,' '); p >= filename && *p == ' '; p--) *p = 0x00;
		cur_file=find_file(filename);
	if (cur_file) {
		dbg(3,"Exists: \"%s\"  %u\n", cur_file->local_fname, cur_file->len);
		ret_dirent(cur_file);
	} else if (do_magic_file(filename)==0) {
		// let UR2 load <root>/DOSxxx.CO from anywhere
		cur_file=make_file_entry(filename,0,0);
		char t[LOCAL_FILENAME_MAX+1] = {0x00};
		for (int i=dir_depth;i>0;i--) strncat(t,"../",3);
		strncat(t,cur_file->local_fname,LOCAL_FILENAME_MAX-dir_depth*3);
		memset(cur_file->local_fname,0x00,LOCAL_FILENAME_MAX);
		memcpy(cur_file->local_fname,t,LOCAL_FILENAME_MAX);
		struct stat st; if (!stat(cur_file->local_fname,&st)) cur_file->len=st.st_size;
		dbg(3,"Virtual: \"%s\" <-- \"%s\"\n",cur_file->client_fname,cur_file->local_fname);
		ret_dirent(cur_file);
	} else {
		if (filename[dot_offset+1]==dme_dir_label[0] && filename[dot_offset+2]==dme_dir_label[1]) f = DIR_FLAG;
		cur_file=make_file_entry(collapse_padded_name(filename), 0, f);
		dbg(3,"New %s: \"%s\"\n",f==DIR_FLAG?"Directory":"File",cur_file->local_fname);
		ret_dirent(NULL);
	}
}

void dirent_get_first() {
	if (debug==1) dbg(2,"Directory Listing\n");
	// update every time before get-first,
	// because set-name is not required before get-first
	update_file_list(ALLOW_RET);
	ret_dirent(get_first_file());
	dme_fdc = 0; // see req_fdc() & ref/fdc.txt
}

// b[0] = cmd
// b[1] = len
// b[2]-b[25] = filename
// b[26] = attr
// b[27] = action (search form)
//
// Ignore the name & attr until after determining the action.
// TS-DOS submits get-first & get-next requests with junk data
// in the filename & attribute fields left over from previous actions.
int req_dirent(unsigned char *b) {
	dbg(2,"%s(%s)\n",__func__,
		b[27]==DIRENT_SET_NAME?"set_name":
		b[27]==DIRENT_GET_FIRST?"get_first":
		b[27]==DIRENT_GET_NEXT?"get_next":
		b[27]==DIRENT_GET_PREV?"get_prev":
		b[27]==DIRENT_CLOSE?"close":
		"UNKNOWN");
	dbg(5,"b[]\n"); dbg_b(5,b,-1);
	dbg_p(4,b);

	switch (b[27]) {
		case DIRENT_SET_NAME:  dirent_set_name(b);          break;
		case DIRENT_GET_FIRST: dirent_get_first();          break;
		case DIRENT_GET_NEXT:  ret_dirent(get_next_file()); break;
		case DIRENT_GET_PREV:  ret_dirent(get_prev_file()); break;
		case DIRENT_CLOSE:                                  break;
	}
	return 0;
}

// update dme_cwd with current dir, truncated & padded both required
// TS-DOS doesn't blank all 6 chars if you don't send all 6
void update_dme_cwd() {
	dbg(2,"%s()\n",__func__);
	int i;
	memset(cwd,0x00,PATH_MAX);
	(void)(getcwd(cwd,PATH_MAX-1)+1);
	dbg(0,"Changed Dir: %s\n",cwd);
	if (dir_depth) {
		for (i=strlen(cwd); i>=0 ; i--) {
			if (cwd[i]=='/') break;
			if (upcase && cwd[i]>='a' && cwd[i]<='z') cwd[i]=cwd[i]-32;
		}
		snprintf(dme_cwd,7,"%-6.6s",cwd+1+i);
	} else {
		memcpy(dme_cwd,dme_root_label,6);
	}
}

// TS-DOS DME return
// Construct a DME packet around dme_cwd and send it to the client
void ret_dme_cwd() {
	dbg(2,"%s(\"%s\")\n",__func__,dme_cwd);
	gb[0]=RET_STD;
	gb[1]=LEN_RET_DME;
	gb[2]=0x00;
	memcpy(gb+3,dme_cwd,6);
	gb[9]=0x00;   // gb[9]='.'; // contents don't matter but length does
	gb[10]=0x00;  // gb[10]=dme_dir_label[0];
	gb[11]=0x00;  // gb[11]=dme_dir_label[1];
	gb[12]=0x00;  // gb[12]=0x20;
	gb[13]=checksum(gb);
	write_client_tty(gb,14);
}

// Any FDC request might actually be a DME request
// See ref/dme.txt for the full explaination because it's a lot.
// dme_fdc is only retained for the duration of one directory listing
// dme_detected is retained forever
void req_fdc() {
	dbg(2,"%s()\n",__func__);

	dbg(3,"dme detection %s\n",dme_disabled?"disabled":"allowed");
	dbg(3,"dme %spreviously detected\n",dme_fdc?"":"not ");

	if (!dme_fdc && !dme_disabled) {
		dbg(3,"testing for dme\n");
		gb[0] = 0x00;
		client_tty_vmt(0,1);   // allow this read to time out
		(void)(read(client_tty_fd,gb,1)+1);
		client_tty_vmt(-1,-1); // restore normal VMIN/VTIME
		if (gb[0]==0x0D) dme_fdc = true;
	}
	if (dme_fdc) {
		dme_detected=true;
		dbg(3,"dme detected\n");
		ret_dme_cwd();
	} else {
		opr_mode = 0;
		dbg(1,"Switching to \"FDC\" mode\n");
	}
}

// b[0] = fmt  0x01
// b[1] = len  0x01
// b[2] = mode 0x01 write new
//             0x02 write append
//             0x03 read
// b[3] = chk
int req_open(unsigned char *b)
{
	dbg(2,"%s(\"%s\")\n",__func__,cur_file->client_fname);
	dbg(5,"b[]\n"); dbg_b(5,b,-1);
	dbg_p(4,b);

	unsigned char omode = b[2];

	switch(omode) {
	case F_OPEN_WRITE:
		dbg(3,"mode: write\n");
		if (o_file_h >= 0) {
			close (o_file_h);
			o_file_h=-1;
		}
		if (cur_file->flags&DIR_FLAG) {
			if(mkdir(cur_file->local_fname,0775)==0) {
				ret_std(ERR_SUCCESS);
			} else {
				ret_std(ERR_FMT_MISMATCH);
			}
		} else {
			o_file_h = open (cur_file->local_fname,O_CREAT|O_TRUNC|O_WRONLY|O_EXCL,0666);
			if (o_file_h<0)
				ret_std(ERR_FMT_MISMATCH);
			else {
				f_open_mode=omode;
				dbg(1,"Open for write: \"%s\"\n",cur_file->local_fname);
				ret_std(ERR_SUCCESS);
			}
		}
		break;
	case F_OPEN_APPEND:
		dbg(3,"mode: append\n");
		if (o_file_h >= 0) {
			close(o_file_h);
			o_file_h=-1;
		}
		if (cur_file==0) {
			ret_std(ERR_FMT_MISMATCH);
			return -1;
		}
		o_file_h = open (cur_file->local_fname, O_WRONLY | O_APPEND);
		if (o_file_h < 0)
			ret_std(ERR_FMT_MISMATCH);
		else {
			f_open_mode=omode;
			dbg(1,"Open for append: \"%s\"\n",cur_file->local_fname);
			ret_std (ERR_SUCCESS);
		}
		break;
	case F_OPEN_READ:
		dbg(3,"mode: read\n");
		if (o_file_h >= 0) {
			close (o_file_h);
			o_file_h=-1;
		}
		if (cur_file==0) {
			ret_std (ERR_NO_FILE);
			return -1;
		}

		if (cur_file->flags&DIR_FLAG) {
			int err=0;
			// directory
			if (cur_file->local_fname[0]=='.' && cur_file->local_fname[1]=='.') {
				// parent dir
				if (dir_depth>0) {
					err=chdir (cur_file->local_fname);
					if (!err) dir_depth--;
				}
			} else {
				// enter dir
				err=chdir(cur_file->local_fname);
				if (!err) dir_depth++;
			}
			update_dme_cwd();
			if (err) ret_std (ERR_FMT_MISMATCH);
			else ret_std (ERR_SUCCESS);
		} else {
			// regular file
			o_file_h = open(cur_file->local_fname, O_RDONLY);
			if (o_file_h<0)
				ret_std (ERR_NO_FILE);
			else {
				f_open_mode = omode;
				dbg(1,"Open for read: \"%s\"\n",cur_file->local_fname);
				ret_std (ERR_SUCCESS);
			}
		}
		break;
	}
	return o_file_h;
}

// b[0] = 0x03
// b[1] = 0x00
// b[2] = chk
void req_read(void) {
	if (ch[1]!=REQ_READ || debug>2) dbg(2,"%s()\n",__func__);
	int i;

	gb[0]=RET_READ;
	if (o_file_h<0) {
		ret_std(ERR_CMDSEQ);
		return;
	}
	if (f_open_mode!=F_OPEN_READ) {
		ret_std(ERR_FMT_MISMATCH);
		return;
	}

	i = read(o_file_h, gb+2, TPDD_DATA_MAX);

	gb[1] = (unsigned char) i;
	gb[2+i] = checksum(gb);

	if(ch[1]==REQ_READ && debug<4) {
		dbg(1,".");
		if (i<TPDD_DATA_MAX) dbg(1,"\n");
	}

	dbg(4,"...outgoing packet...\n");
	dbg(5,"gb[]\n"); dbg_b(5,gb,-1);
	dbg_p(4,gb);
	dbg(4,".....................\n");

	write_client_tty(gb, 3+i);
}

// b[0] = 0x04
// b[1] = 0x01 - 0x80
// b[2] = b[1] bytes
// b[2+len] = chk
void req_write(unsigned char *b) {
	if (ch[1]!=REQ_WRITE || debug>2) dbg(2,"%s()\n",__func__);
	dbg(4,"...incoming packet...\n");
	dbg(5,"b[]\n"); dbg_b(5,b,-1);
	dbg_p(4,b);
	dbg(4,".....................\n");

	if (o_file_h<0) {ret_std(ERR_CMDSEQ); return;}

	if (f_open_mode!=F_OPEN_WRITE && f_open_mode !=F_OPEN_APPEND) {
		ret_std(ERR_FMT_MISMATCH);
		return;
	}

	if(ch[1]==REQ_WRITE && debug<4) {
		dbg(1,".",b[1]);
		if (b[1]<TPDD_DATA_MAX) dbg(1,"\n");
	}

	if (write (o_file_h,b+2,b[1]) != b[1]) ret_std (ERR_SECTOR_NUM);
	else ret_std (ERR_SUCCESS);
}

void req_delete(void) {
	dbg(2,"%s()\n",__func__);
	if (cur_file->flags&DIR_FLAG) rmdir(cur_file->local_fname);
	else unlink (cur_file->local_fname);
	dbg(1,"Deleted: %s\n",cur_file->local_fname);
	ret_std (ERR_SUCCESS);
}

// TPDD2 sector cache write - but not really doing it
// Previously called "TS-DOS mystery command 1"
// This is just something TS-DOS does to detect TPDD2. Respond just enough
// to satisfy TS-DOS that it may use TPDD2 features like dirent(get-prev).
// Just return a canned packet that means "cache write suceeded".
// http://bitchin100.com/wiki/index.php?title=TPDD-2_Sector_Access_Protocol
// https://github.com/bkw777/pdd.sh search for "pdd2_write_cache
// FIXME: We should really only respond success if the payload exactly
// matches TS-DOS's, and error any other attempt to use this function.
void ret_cache_write() {
	dbg(3,"%s()\n",__func__);
	gb[0]=RET_CACHE_STD;
	gb[1]=0x01;
	gb[2]=ERR_SUCCESS;
	gb[3]=checksum(gb);
	write_client_tty(gb,4);
}

// Another part of TS-DOS's drive/server capabilities detection scheme.
// Previously called "TS-DOS mystery command 2"
// The function of the command in a real drive is unknown.
// The meaning of the response is unkmnown.
// But the command apparently takes no parameters, and a real TPDD2 always
// responds with the same string of bytes, and TPDD1 ignores it.
// not counting ZZ or checksums:
// Client sends  : 23 00
// TPDD2 responds: 14 0F 41 10 01 00 50 05 00 02 00 28 00 E1 00 00 00
// TPDD1 does not respond.
void ret_pdd2_unk23() {
	dbg(3,"%s()\n",__func__);
	static unsigned char canned[] = {RET_PDD2_UNK23, 0x0F, 0x41, 0x10, 0x01, 0x00, 0x50, 0x05, 0x00, 0x02, 0x00, 0x28, 0x00, 0xE1, 0x00, 0x00, 0x00};
	memcpy(gb, canned, canned[1]+2);
	gb[canned[1]+2] = checksum(gb);
	write_client_tty(gb, gb[1]+3);
}

// Similar to unk23, except the response is different, and not used by TS-DOS
// Nothing uses this command that I know of. I just found it by feeding
// abitrary commands to a real drive with github/bkw777/pdd.sh
// 0x11 and 0x33 both produce the same response. Possibly 0x11 and 0x33 are
// just different versions of the same function, like how 0x4# commands are
// really just 0x0# commands for bank 1 instead of bank 0? Just a guess.
// not counting ZZ or checksums:
// Client sends  : 11 00
//     or sends  : 33 00
// TPDD2 responds: 3A 06 80 13 05 00 10 E1
void ret_pdd2_unk11() {
	dbg(3,"%s()\n",__func__);
	static unsigned char canned[] = {RET_PDD2_UNK11, 0x06, 0x80, 0x13, 0x05, 0x00, 0x10, 0xE1};
	memcpy(gb, canned, canned[1]+2);
	gb[canned[1]+2] = checksum(gb);
	write_client_tty(gb, gb[1]+3);
}

void req_rename(unsigned char *b) {
	dbg(3,"%s(%-24.24s)\n",__func__,b+2);
	char *t = (char *)b + 2;
	memcpy(t,collapse_padded_name(t),TPDD_FILENAME_LEN);
	if (rename(cur_file->local_fname,t))
		ret_std(ERR_SECTOR_NUM);
	else {
		dbg(1,"Renamed: %s -> %s\n",cur_file->local_fname,t);
		ret_std(ERR_SUCCESS);
	}
}

void req_close() {
	if (o_file_h>=0) close(o_file_h);
	o_file_h = -1;
	dbg(2,"\nClosed: \"%s\"\n",cur_file->local_fname);
	ret_std(ERR_SUCCESS);
}

void req_status() {
	dbg(2,"%s()\n",__func__);
	ret_std(ERR_SUCCESS);
}

void req_condition() {
	dbg(2,"%s()\n",__func__);
	ret_std(ERR_SUCCESS);
}

void req_format() {
	dbg(2,"%s()\n",__func__);
	ret_std(ERR_SUCCESS);
}

void dispatch_opr_cmd(unsigned char *b) {
	dbg(3,"%s(%02X)\n",__func__,b[0]);
	switch(b[0]) {
		case REQ_DIRENT:        req_dirent(b);       break;
		case REQ_OPEN:          req_open(b);         break;
		case REQ_CLOSE:         req_close();         break;
		case REQ_READ:          req_read();          break;
		case REQ_WRITE:         req_write(b);        break;
		case REQ_DELETE:        req_delete();        break;
		case REQ_FORMAT:        req_format();        break;
		case REQ_STATUS:        req_status();        break;
		case REQ_FDC:           req_fdc();           break;
		case REQ_CONDITION:     req_condition();     break;
		case REQ_RENAME:        req_rename(b);       break;
		case REQ_PDD2_UNK23:    ret_pdd2_unk23();    break;
		case REQ_CACHE_WRITE:   ret_cache_write();   break;
		case REQ_PDD2_UNK11:    ret_pdd2_unk11();    break;
		case REQ_PDD2_UNK33:    ret_pdd2_unk11();    break;
	}
}

void get_opr_cmd(void)
{
	dbg(3,"%s()\n",__func__);
	unsigned char b[TPDD_DATA_MAX+3] = {0x00};
	unsigned i = 0;
	memset(gb,0x00,TPDD_DATA_MAX+3);

	while (read_client_tty(&b,1) == 1) {
		if (b[0]==0x5A) i++; else { i=0; b[0]=0x00; continue; }
		if (i<2) { b[0]=0x00; continue; }
		if (read_client_tty(&b,2) == 2) if (read_client_tty(&b[2],b[1]+1) == b[1]+1) break;
		i=0; memset(b,0x00,TPDD_DATA_MAX+3);
	}

	dbg_p(3,b);

	i = checksum(b);
	if (b[b[1]+2]!=i) {
		dbg(0,"Failed checksum: received: %02X  calculated: %02X\n",b[b[1]+2],i);
		return; // real drive does not return anything
	}

	ch[1]=ch[0]; ch[0]=b[0];

	dispatch_opr_cmd(b);
}

////////////////////////////////////////////////////////////////////////
//
//  FDC MODE

// Just a stub yet, but one operation works, which is switching back
// and forth between FDC-mode and Operation-mode.
//
// You can see it happen by running "OPR_MODE=0 dl -vv"
// See it starts on get_fdc_cmd() instead of get_opr_cmd()
// Then load the directory from TS-DOS.
// standard 8-character FDC-mode response

void ret_fdc_std(unsigned char e, unsigned char d, unsigned short l) {
	dbg(2,"%s()\n",__func__);
	char b[9] = { 0x00 };
	snprintf(b,9,"%02X%02X%04X",e,d,l);
	dbg(1,"FDC: response: \"%s\"\n",b);
	write_client_tty(b,8);
}

void req_fdc_set_mode(char *b) {
	dbg(2,"%s(%s)\n",__func__,b+1);
	int m = atoi(&b[1]);
	dbg(1,"FDC: Switching to \"%s\" mode\n",m==0?"FDC":m==1?"Operation":"-invalid-");
	opr_mode=m; // no response, just switch modes
}

void req_fdc_condition(char *b) {
	dbg(2,"%s(%s)\n",__func__,b+1);
	ret_fdc_std(ERR_FDC_SUCCESS,0,0);
}
void req_fdc_format(char *b) {
	dbg(2,"%s(%s)\n",__func__,b+1);
	ret_fdc_std(ERR_FDC_SUCCESS,0,0);
}
void req_fdc_format_nv(char *b) {
	dbg(2,"%s(%s)\n",__func__,b+1);
	ret_fdc_std(ERR_FDC_SUCCESS,0,0);
}
void req_fdc_read_id(char *b) {
	dbg(2,"%s(%s)\n",__func__,b+1);
	ret_fdc_std(ERR_FDC_COMMAND,0,0);
}
void req_fdc_read_sector(char *b) {
	dbg(2,"%s(%s)\n",__func__,b+1);
	ret_fdc_std(ERR_FDC_COMMAND,0,0);
}
void req_fdc_search_id(char *b) {
	dbg(2,"%s(%s)\n",__func__,b+1);
	ret_fdc_std(ERR_FDC_COMMAND,0,0);
}
void req_fdc_write_id(char *b) {
	dbg(2,"%s(%s)\n",__func__,b+1);
	ret_fdc_std(ERR_FDC_COMMAND,0,0);
}
void req_fdc_write_id_nv(char *b) {
	dbg(2,"%s(%s)\n",__func__,b+1);
	ret_fdc_std(ERR_FDC_COMMAND,0,0);
}
void req_fdc_write_sector(char *b) {
	dbg(2,"%s(%s)\n",__func__,b+1);
	ret_fdc_std(ERR_FDC_COMMAND,0,0);
}
void req_fdc_write_sector_nv(char *b) {
	dbg(2,"%s(%s)\n",__func__,b+1);
	ret_fdc_std(ERR_FDC_COMMAND,0,0);
}

/* ref/fdc.txt */
void get_fdc_cmd(void) {
	dbg(3,"%s()\n",__func__);
	char b[TPDD_DATA_MAX] = {0x00};
	unsigned i = 0;
	bool eol = false;

	// see if the command byte was collected already by req_fdc()
	if (gb[0]>0x00 && gb[0]!=0x0D && gb[1]==0x00) {b[0]=gb[0];i=1;}

	// TODO canonical mode
	// read command
	while (i<TPDD_DATA_MAX && !eol) {
		if (read_client_tty(&b[i],1)==1) {
			switch (b[i]) {
				case 0x0D: eol=true;
				case 0x20: b[i]=0x00; break;
				default: i++;
			}
		}
	}

	// debug
	dbg(3,"\"%s\"\n",b);

	// dispatch
	switch (b[0]) {
		case FDC_SET_MODE:        req_fdc_set_mode(b);        break;
		case FDC_CONDITION:       req_fdc_condition(b);       break;
		case FDC_FORMAT:          req_fdc_format(b);          break;
		case FDC_FORMAT_NV:       req_fdc_format_nv(b);       break;
		case FDC_READ_ID:         req_fdc_read_id(b);         break;
		case FDC_READ_SECTOR:     req_fdc_read_sector(b);     break;
		case FDC_SEARCH_ID:       req_fdc_search_id(b);       break;
		case FDC_WRITE_ID:        req_fdc_write_id(b);        break;
		case FDC_WRITE_ID_NV:     req_fdc_write_id_nv(b);     break;
		case FDC_WRITE_SECTOR:    req_fdc_write_sector(b);    break;
		case FDC_WRITE_SECTOR_NV: req_fdc_write_sector_nv(b); break;
		case 0x00: if (!i) {dbg(1,"FDC: empty command\n");    break;}
		default: dbg(1,"FDC: unknown command\n");
	}
}

////////////////////////////////////////////////////////////////////////
//
//  BOOTSTRAP

void show_bootstrap_help() {
	dbg(0,
		"%1$s - DeskLink+ " S_(APP_VERSION) " - \"bootstrap\" help\n\n"
		"Available loader files (in " S_(APP_LIB_DIR) "):\n\n",args[0]);

	dbg(0,  "TRS-80 Model 100 & 102 :"); lsx(S_(APP_LIB_DIR),"100");
	dbg(0,"\nTANDY Model 200        :"); lsx(S_(APP_LIB_DIR),"200");
	dbg(0,"\nNEC PC-8201(a)/PC-8300 :"); lsx(S_(APP_LIB_DIR),"NEC");
	dbg(0,"\nKyotronic KC-85        :"); lsx(S_(APP_LIB_DIR),"K85");
	dbg(0,"\nOlivetti M-10          :"); lsx(S_(APP_LIB_DIR),"M10");

	dbg(0,
		"\n\n"
		"Filenames given without any leading path are taken from above.\n"
		"To specify a file in the current directory, include the \"./\"\n"
		"Examples:\n\n"
		"   %1$s -b TS-DOS.100\n"
		"   %1$s -b ~/Documents/LivingM100SIG/Lib-03-TELCOM/XMDPW5.100\n"
		"   %1$s -b ./rxcini.DO\n\n"
	,args[0]);
}

void slowbyte(char b) {
	write_client_tty(&b,1);
	tcdrain(client_tty_fd);
	usleep(BASIC_byte_us);
	switch (debug) {
		case 0: return;
		case 1: putchar('.'); break;
		case 2: // display nicely no matter if loader is CR, LF, or CRLF
			if (b!=0x0A && gb[0]==0x0A) {gb[0]=0x00; putchar(0x0A); putchar(b);}
			else if (b==0x0A || b==BASIC_EOL) gb[0]=0x0A;
			else if (isprint(b)) putchar(b);
			else printf("\033[7m%02X\033[m",b);
			break;
	}
	fflush(stdout);
}

int send_BASIC(char *f)
{
	int fd;
	char b;

	if ((fd=open(f,O_RDONLY))<0) {
		dbg(1,"Could not open \"%s\"\n",f);
		return 9;
	}

	printf("Sending \"%s\" ... ",f);
	if (debug) puts("");
	fflush(stdout);
	while(read(fd,&b,1)==1) slowbyte(b);
	close(fd);
	if (b!=0x0A && b!=BASIC_EOL && b!=BASIC_EOF) slowbyte(BASIC_EOL);
	if (b!=BASIC_EOF) slowbyte(BASIC_EOF);
	if (debug) puts("");
	puts("DONE");
	close(client_tty_fd);
	return 0;
}

int bootstrap(char *f)
{
	int r = 0;
	char loader_file[PATH_MAX]={0x00};
	char pre_install_txt_file[PATH_MAX]={0x00};
	char post_install_txt_file[PATH_MAX]={0x00};

	if (f[0]=='~' && f[1]=='/') {
		strcpy(loader_file,getenv("HOME"));
		strcat(loader_file,f+1);
	}

	if ((f[0]=='/') || (f[0]=='.' && f[1]=='/'))
		strcpy(loader_file,f);

	if (loader_file[0]==0) {
		strcpy(loader_file,S_(APP_LIB_DIR));
		strcat(loader_file,"/");
		strcat(loader_file,f);
	}

	strcpy(pre_install_txt_file,loader_file);
	strcat(pre_install_txt_file,".pre-install.txt");

	strcpy(post_install_txt_file,loader_file);
	strcat(post_install_txt_file,".post-install.txt");

	printf("Bootstrap: Installing \"%s\"\n\n", loader_file);

	if (access(loader_file,F_OK)==-1) {
		puts("Not found.");
		return 1;
	}

	if (access(pre_install_txt_file,F_OK)>=0) {
		cat(pre_install_txt_file);
	} else {
		puts("Prepare BASIC to receive:");
		puts("");
		puts("    RUN \"COM:98N1ENN\" [Enter]    <-- for TANDY/Olivetti/Kyotronic");
		puts("    RUN \"COM:9N81XN\"  [Enter]    <-- for NEC");
		puts("");
	}

	puts("");
	puts("Press [Enter] when ready...");
	getchar();

	if ((r=send_BASIC(loader_file))!=0) return r;

	cat(post_install_txt_file);

	printf("\n\n\"%s -b\" will now exit.\n",args[0]);
	printf("Re-run \"%s\" (without -b this time) to run the TPDD server.\n",args[0]);
	puts("");

	return 0;
}

////////////////////////////////////////////////////////////////////////
//
//  MAIN

void show_config () {
	dbg(0,"getty_mode      : %s\n",getty_mode?"true":"false");
	dbg(0,"upcase          : %s\n",upcase?"true":"false");
	dbg(0,"rtscts          : %s\n",rtscts?"true":"false");
	dbg(0,"verbosity       : %d\n",debug);
	dbg(0,"dot_offset      : %d\n",dot_offset);
	dbg(0,"BASIC_byte_ms   : %d\n",BASIC_byte_us/1000);
	dbg(0,"bootstrap_mode  : %s\n",bootstrap_mode?"true":"false");
	dbg(0,"bootstrap_file  : \"%s\"\n",bootstrap_file);
	dbg(0,"client_tty_name : \"%s\"\n",client_tty_name);
	dbg(0,"share_path      : \"%s\"\n",cwd);
	dbg(2,"opr_mode        : %d\n",opr_mode);
	dbg(2,"dot_offset      : %d\n",dot_offset);
	dbg(2,"baud            : %d\n",client_baud==B9600?9600:client_baud==B19200?19200:-1);
	dbg(0,"dme_disabled    : %s\n",dme_disabled?"true":"false");
	dbg(2,"dme_root_label  : \"%-6.6s\"\n",dme_root_label);
	dbg(2,"dme_parent_label: \"%-6.6s\"\n",dme_parent_label);
	dbg(2,"dme_dir_label   : \"%-2.2s\"\n",dme_dir_label);
	dbg(0,"ur2_dos_hack    : %s\n",enable_ur2_dos_hack?"enabled":"disabled");
	dbg(2,"default_attr    : '%c'\n",default_attr);
}

void show_main_help() {
	dbg(0,
		"%1$s - DeskLink+ " S_(APP_VERSION) " - help\n\n"
		"usage: %1$s [options] [tty_device] [share_path]\n"
		"\n"
		"options:\n"
		"   -0       Raw mode. Do not munge filenames in any way.\n"
		"            Disables 6.2 or 8.2 filename trucating & padding\n"
		"            Changes the attribute byte to ' ' instead of 'F'\n"
		"            Disables adding the TS-DOS \".<>\" extension for directories\n"
		"            The entire 24 bytes of the filename field on a real drive is used.\n"
		"   -a c     Attr - attribute used for all files (F)\n"
		"   -b file  Bootstrap: Send loader file to client\n"
		"   -d tty   Serial device to client (" S_(DEFAULT_CLIENT_TTY) ")\n"
		"   -g       Getty mode - run as daemon\n"
		"   -h       Print this help\n"
		"   -l       List available loader files and bootstrap help\n"
		"   -p dir   Share path - directory with files to be served (.)\n"
		"   -r       RTS/CTS hardware flow control\n"
		"   -s #     Speed - serial port baud rate 9600 or 19200 (19200)\n"
		"   -u       Uppercase all filenames\n"
		"   -v       Verbose/debug mode - more v's = more verbose\n"
		"   -w       WP-2 mode - 8.2 filenames\n"
		"   -z #     Milliseconds per byte for bootstrap (" S_(DEFAULT_BASIC_BYTE_MS) ")\n"
		"\n"
		"Alternative to the -d and -p options,\n"
		"The 1st non-option argument is another way to specify the tty device.\n"
		"The 2nd non-option argument is another way to specify the share path.\n"
		"\n"
		"   %1$s\n"
		"   %1$s -vv /dev/ttyS0\n"
		"   %1$s ttyUSB1 -v -w ~/Documents/wp2files\n\n"
	,args[0]);
}

int main(int argc, char **argv)
{
	int off=0;
	int i;
	bool x = false;
	args = argv;

	// default client tty device
	strcpy (client_tty_name,S_(DEFAULT_CLIENT_TTY));
	if (client_tty_name[0]!='/') {
		strcpy(client_tty_name,"/dev/");
		strcat(client_tty_name,S_(DEFAULT_CLIENT_TTY));
	}

	// environment variable overrides for some things that don't have switches
	if (getenv("OPR_MODE")) opr_mode = atoi(getenv("OPR_MODE"));
	if (getenv("DISABLE_DME")) dme_disabled = true;
	if (getenv("DISABLE_UR2_DOS_HACK")) enable_ur2_dos_hack = false;
	if (getenv("DOT_OFFSET")) dot_offset = atoi(getenv("DOT_OFFSET"));
	if (getenv("BAUD")) set_baud(getenv("BAUD"));
	if (getenv("ROOT_LABEL")) {snprintf(dme_root_label,7,"%-6.6s",getenv("ROOT_LABEL"));
		memcpy(dme_cwd,dme_root_label,6);}
	if (getenv("PARENT_LABEL")) snprintf(dme_parent_label,7,"%-6.6s",getenv("PARENT_LABEL"));
	if (getenv("DIR_LABEL")) snprintf(dme_dir_label,3,"%-2.2s",getenv("DIR_LABEL"));
	if (getenv("ATTR")) default_attr = *getenv("ATTR");

	// commandline options
	while ((i = getopt (argc, argv, ":0a:b:d:ghlp:rs:uvwz:^")) >=0)
		switch (i) {
			case '0': dot_offset=0; upcase=false; default_attr=0x20;      break;
			case 'a': default_attr=*strndup(optarg,1);                    break;
			case 'b': bootstrap_mode=true; strcpy(bootstrap_file,optarg); break;
			case 'd': strcpy(client_tty_name,optarg);                     break;
			case 'g': getty_mode = true; debug = 0;                       break;
			case 'h': show_main_help(); exit(0);                          break;
			case 'l': show_bootstrap_help(); exit(0);                     break;
			case 'p': (void)(chdir(optarg)+1);                            break;
			case 'r': rtscts = true;                                      break;
			case 's': set_baud(optarg);                                   break;
			case 'u': upcase = true;                                      break;
			case 'v': debug++;                                            break;
			case 'w': dot_offset = 8;                                     break;
			case 'z': BASIC_byte_us=atoi(optarg)*1000;                    break;
			case '^': x=true;                                             break;
			case ':': dbg(0,"\"-%c\" requires a value\n",optopt);         break;
			case '?':
				if (isprint(optopt)) dbg(0,"Unknown option `-%c'.\n",optopt);
				else dbg(0,"Unknown option character `\\x%x'.\n",optopt);
			default: show_main_help();                                 return 1;
		}

	// commandline non-option arguments
	for (i=0; optind < argc; optind++) {
		if (x) dbg(1,"non-option arg %u: \"%s\"\n",i,argv[optind]);
		switch (i++) {
			case 0: // tty device
				switch (argv[optind][0]) {
					case '/':
						strcpy (client_tty_name,argv[optind]);
						break;
					case '-':
						if (argv[optind][1]==0x00) {
							strcpy (client_tty_name,"/dev/tty");
							client_tty_fd = 1;
							break;
						}
					default:
						strcpy(client_tty_name,"/dev/");
						strcat(client_tty_name,argv[optind]);
				} break;
			case 1: // share path
				(void)(chdir(argv[optind])+1); break;
			default: dbg(0,"Unknown argument: \"%s\"\n",argv[optind]);
		}
	}

	(void)(getcwd(cwd,PATH_MAX-1)+1);

	if (x) { show_config(); return 0; }

	dbg(0,"DeskLink+ " S_(APP_VERSION) "\n"
		  "Serial Device: %s\n"
		  "Working Dir: %s\n",client_tty_name,cwd);

	if (client_tty_fd<0)
		client_tty_fd=open((char *)client_tty_name,O_RDWR,O_NOCTTY);

	if (client_tty_fd<0) {
		dbg(1,"Can't open \"%s\"\n",client_tty_name);
		return 1;
	}

	// getty mode
	if (getty_mode) {
		if (login_tty(client_tty_fd)==0) client_tty_fd = STDIN_FILENO;
		else (void)(daemon(1,1)+1);
	}

	// serial line setup
	(void)(tcflush(client_tty_fd, TCIOFLUSH)+1);
	ioctl(client_tty_fd, FIONBIO, &off);
	ioctl(client_tty_fd, FIOASYNC, &off);
	if (tcgetattr(client_tty_fd,&client_termios)==-1) return 21;
	cfmakeraw(&client_termios);
	client_termios.c_cflag |= CLOCAL|CS8;
	if (rtscts) client_termios.c_cflag |= CRTSCTS;
	else client_termios.c_cflag &= ~CRTSCTS;
	if (cfsetspeed(&client_termios,client_baud)==-1) return 22;
	if (tcsetattr(client_tty_fd,TCSANOW,&client_termios)==-1) return 23;
	client_tty_vmt(-2,-2);

	// send loader and exit
	if (bootstrap_mode) return (bootstrap(bootstrap_file));

	// create the file list
	file_list_init();
	if (debug) update_file_list(NO_RET);

	// process commands forever
	while (1) if (opr_mode) get_opr_cmd(); else get_fdc_cmd();

	return 0;
}
