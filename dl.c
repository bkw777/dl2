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
2020     Kurt McCullum - TS-DOS loaders
2022     Gabriele Gorla - TS-DOS subdirectories

DeskLink2
2023     Brian K. White - disk image files, pdd1 FDC mode, pdd2 cache & memory

DeskLink2 is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 or any
later version as published by the Free Software Foundation.  

DeskLink2 is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (in the file "COPYING"); if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111, USA.
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
#include <errno.h>
#include <stdbool.h>
#include "constants.h"
#include "dir_list.h"

#if defined(__APPLE__) || defined(__NetBSD__) || defined(OpenBSD)
#include <util.h>
#endif

#if defined(__FreeBSD__)
#include <libutil.h>
#endif

#if defined(__linux__)
#include <utmp.h>
#endif

/*** config **************************************************/


#ifndef APP_NAME
#define APP_NAME "DeskLink2"
#endif

#ifndef APP_LIB_DIR
#define APP_LIB_DIR "."
#endif

#ifndef TTY_PREFIX
#define TTY_PREFIX "ttyS"
#endif

#ifndef DEFAULT_BAUD
#define DEFAULT_BAUD 19200
#endif

// default model emulation, 1=pdd1 2=pdd2
// TS-DOS directories requires tpdd1
#ifndef DEFAULT_MODEL
#define DEFAULT_MODEL 1
#endif

// if a loader fails in bootstrap(), try increasing this
#ifndef DEFAULT_BASIC_BYTE_MS
#define DEFAULT_BASIC_BYTE_MS 8
#endif

// TODO
// * What does Cambridge Z88 do?
// * What do CP/M clients do?
// * What do the original Brother knitting machines do?
#ifndef DEFAULT_ATTR
#define DEFAULT_ATTR 0x46 // F
#endif
#ifndef RAW_ATTR
#define RAW_ATTR     0x20 // space
#endif

#define DEFAULT_TPDD1_IMG_SUFFIX ".pdd1"
#define DEFAULT_TPDD2_IMG_SUFFIX ".pdd2"

#ifndef DEFAULT_UPCASE
#define DEFAULT_UPCASE false
#endif

#ifndef DEFAULT_RTSCTS
#define DEFAULT_RTSCTS false
#endif

#ifndef DEFAULT_DOTPOS
#define DEFAULT_DOTPOS 6
#endif

// To mimic the original Desk-Link from Travelling Software:
//#define DEFAULT_DME_ROOT_LABEL   "ROOT  "
//#define DEFAULT_DME_PARENT_LABEL "PARENT"
#ifndef TSDOS_ROOT_LABEL
#define TSDOS_ROOT_LABEL   "0:    "
#endif
#ifndef TSDOS_PARENT_LABEL
#define TSDOS_PARENT_LABEL "^     "
#endif
// you can't change this unless you also hack ts-dos
#define TSDOS_DIR_LABEL    "<>"

/*
 * Support for Ultimate ROM II TS-DOS & Sardine loader: see ref/ur2.txt
 * These filenames will always be loadable "by magic" in any cd path, even
 * if no such filename exists anywhere in the share tree. For any of these
 * filenames, search the following paths: cwd, share root, app_lib_dir.
 * TODO add $XDG_DATA_HOME (~/.local/share/myapp  mac: ~/Library/myapp/)
 */
const char * magic_files [] = {
	"DOS100.CO",
	"DOS200.CO",
	"DOSNEC.CO",
	"SAR100.CO",
	"SAR200.CO",
	"SARNEC.CO", // SARNEC.CO is known to have existed, but is currently lost.
	"DOSM10.CO", // The rest of these probably never existed.
	"DOSK85.CO", //
	"SARM10.CO", //
	"SARK85.CO"  //
};

// terminal emulation
#define SSO "\033[7m" // set standout
#define RSO "\033[m"  // reset standout
#define D8C "\033 F"  // disable 8-bit vtxx control bytes (0x80-0x9F)

// The roms in Brother FB-100, knitking FDD19, Purple Computing D103, and
// TANDY 26-3808 have all been dumped and compared, and are all identical.
// So the rom is from Brother not TANDY.
//#ifndef FB100_ROM
//#define FB100_ROM "fb100.rom"
//#endif

#ifndef TPDD2_ROM
#define TPDD2_ROM "tpdd2.rom"
#endif

// termios VMIN & VTIME
#define C_CC_VMIN 1
#define C_CC_VTIME 5

/*************************************************************/

int debug = 0;
bool upcase = DEFAULT_UPCASE;
bool rtscts = DEFAULT_RTSCTS;
unsigned dot_offset = DEFAULT_DOTPOS; // 0 for raw, 6 for KC-85, 8 for WP-2
uint8_t model = DEFAULT_MODEL;
int baud = DEFAULT_BAUD;
int BASIC_byte_us = DEFAULT_BASIC_BYTE_MS*1000;

char client_tty_name[PATH_MAX+1] = {0x00};
char disk_img_fname[PATH_MAX+1] = {0x00};
char app_lib_dir[PATH_MAX+1] = APP_LIB_DIR;
char share_path[2][PATH_MAX+1] = {{0},{0}};
char dme_root_label[7] = TSDOS_ROOT_LABEL;
char dme_parent_label[7] = TSDOS_PARENT_LABEL;
char dme_dir_label[3] = TSDOS_DIR_LABEL;
char default_attr = DEFAULT_ATTR;
bool enable_magic_files = true;
#if !defined(_WIN)
bool getty_mode = false;
#endif

char** args;
int f_open_mode = F_OPEN_NONE;
int client_tty_fd = -1;
int disk_img_fd = -1;
struct termios client_termios;
int o_file_h = -1;
uint8_t gb[TPDD_MSG_MAX];
char iwd[PATH_MAX+1] = {0x00};
char cwd[PATH_MAX+1] = {0x00};
char dme_cwd[7] = DEFAULT_DME_ROOT_LABEL;
char bootstrap_fname[PATH_MAX+1] = {0x00};
int opr_mode = 1;
uint8_t dme = 0;
uint8_t bank = 0;
bool dme_disabled = false;
uint8_t ch[2] = {0xFF}; // 0x00 is a valid Operation-mode command, so init to 0xFF
uint8_t rb[SECTOR_LEN] = {0x00}; // pdd1 disk image record buffer
FILE_ENTRY* cur_file;
int dir_depth=0;
uint8_t pdd1_condition = PDD1_COND_NONE; // pdd1 condition bit flags
uint8_t pdd2_condition = PDD2_COND_NONE; // pdd2 condition bit flags

// drive cpu memory map
uint8_t ioport[IOPORT_LEN] = {0x00}; // i/o port
uint8_t cpuram[CPURAM_LEN] = {0x00}; // 128 bytes cpu internal ram
uint8_t ga[GA_LEN] = {0x00};         // gate array interface
uint8_t ram[RAM_LEN] = {0x00};       // 2k ram (pdd2 disk image record buffer)
uint8_t rom[ROM_LEN] = {0x00};       // 4k cpu internal mask rom

void show_main_help();

/* primitives and utilities */

// dbg(verbosity_threshold, printf_format, args...)
// dbg(3,"err %02X",err); // means only show this message if debug>=3
void dbg( const int v, const char* format, ... ) {
	if (debug<v) return;
	va_list args;
	va_start( args, format );
	vfprintf( stderr, format, args );
	fflush(stderr);
	va_end( args );
}

// dbg_b(verbosity_threshold, buffer, len)
// dbg_b(3, b, 24); // like dbg() except
// print n bytes of b[] as hex pairs and a trailing newline
// if n<0, then use TPDD_MSG_MAX
void dbg_b(const int v, unsigned char* b, int n) {
	if (debug<v) return;
	unsigned i;
	if (n<0) n = TPDD_MSG_MAX;
	for (i=0;i<n;i++) fprintf (stderr,"%02X ",b[i]);
	fprintf (stderr, "\n");
	fflush(stderr);
}

// like dbg_b, except assume b[] is an Operation-mode req or ret block
// and parse it to display the parts: cmd, len, payload, checksum.
void dbg_p(const int v, unsigned char* b) {
	dbg(v,"cmd: %1$02X\nlen: %2$02X (%2$u)\nchk: %3$02X\ndat: ",b[0],b[1],b[b[1]+2]);
	dbg_b(v,b+2,b[1]);
}

// int-to-rate - given int 9600 return macro B9600
int itor (int i) {
	return
		i==75?B75:         // kc85 supports, drive does not
		i==110?B110:       // kc85 supports, drive does not
		i==150?B150:       // drive supports, kc85 does not
		i==300?B300:
		i==600?B600:
		i==1200?B1200:
		i==2400?B2400:
		i==4800?B4800:
		i==9600?B9600:     // default Brother FB-100, KnitKing FDD19, Purple Computing D103
		i==19200?B19200:   // default TANDY 26-3808, TANDY 26-3814
		i==38400?B38400:
#if defined(B76800) //#if defined(__sparc__) // ref/baud_linux.c
		i==76800?B76800:   // drive supports, local platform may
#endif
		0;
}

// given 19200 return 9 (the # in "COM:#8N1ENN")
int baud_to_stat_code (int r) {
	return
		r==75?1:     // kc85 supports, drive does not
		r==110?2:    // kc85 supports, drive does not
//		r==150?0:    // drive supports, kc85 does not
		r==300?3:
		r==600?4:
		r==1200?5:
		r==2400?6:
		r==4800?7:
		r==9600?8:
		r==19200?9:
//		b==76800?0: // drive supports, kc85 does not
		0;
}

void update_cwd () {
	memset(cwd,0x00,PATH_MAX);
	(void)!getcwd(cwd,PATH_MAX);

	// if the current directory is not writable, set the write-protected disk flag
	uint8_t wp = 0;
	if (access(cwd,W_OK|X_OK)) wp = 1;
	pdd1_condition |= wp << PDD1_COND_BIT_WPROT;
	pdd2_condition |= wp << PDD2_COND_BIT_WPROT;
}

void add_share_path (char* s) {
	dbg(3,"%s(%s)\n",__func__,s);
	if (!share_path[0][0]) { strcpy(share_path[0],s); return; }
	if (!share_path[1][0]) { strcpy(share_path[1],s); return; }
	dbg(2,"Discarded excess share path \"%s\"\n",s);
}

void cd_share_path () {
	if (!share_path[bank][0]) return;
	if (!strncmp(cwd,share_path[bank],PATH_MAX)) return;
	if (chdir(share_path[bank])) dbg(0,"FAILED CD TO \"%s\"\n",share_path[bank]);
	update_cwd();
}

// maybe rewrite f[] with /path/to/f
void find_lib_file (char* f) {
	if (f[0]==0x00) return;

	char t[PATH_MAX+1]={0x00};

	// rewrite ~/foo to $HOME/foo
	if (f[0]=='~' && f[1]=='/') {
		strcpy(t,f);
		memset(f,0x00,PATH_MAX);
		strcpy(f,getenv("HOME"));
		strcat(f,t+1);
	}

	if (f[0]=='/') return; // don't rewrite any absolute path
	if (f[0]=='.' && f[1]=='/') return; // don't rewrite explicit relative path
	if (f[0]=='.' && f[1]=='.' && f[2]=='/') return; // don't rewrite explicit relative path
	if (!access(f,F_OK)) return; // if pathless filename exists & accessible, use it as-is

	// none of above matched, look in app_lib_dir
	memset(t,0x00,PATH_MAX);
	strcpy(t,app_lib_dir);
	strcat(t,"/");
	strcat(t,f);
	// if found in app_lib_dir then rewrite with that path
	if (!access(t,F_OK)) {
		memset(f,0x00,PATH_MAX);
		strcpy(f,t);
	}
	// else leave f[] as it was, some consumers create the file if not exist

}

int check_disk_image () {
	// if they didn't ask for any disk image, then bail without error
	if (!disk_img_fname[0]) return 0;

	dbg(3,"looking for disk image \"%s\"\n",disk_img_fname);

	find_lib_file(disk_img_fname);

	// disk_img_fname has now been re-written if and as necessary,
	// and still may or may not exist
	struct stat info;
	if (!stat(disk_img_fname, &info) && info.st_size>0) {
		// if file exists and >0 bytes
		dbg(1,"Loading disk image file \"%s\"\n",disk_img_fname);

		// use file size to automatically set model 1 vs 2 or reject file
		if (info.st_size==PDD1_IMG_LEN) model = 1;
		if (info.st_size==PDD2_IMG_LEN) model = 2;
		if (model==1 && info.st_size != PDD1_IMG_LEN) {
			dbg(0,"%d bytes, expected %u bytes for TPDD1\n",info.st_size,PDD1_IMG_LEN);
			return 1;
		}
		if (model==2 && info.st_size != PDD2_IMG_LEN) {
			dbg(0,"%d bytes, expected %u bytes for TPDD2\n",info.st_size,PDD2_IMG_LEN);
			return 1;
		}
	} else {
		// if file doesn't exist or is 0 bytes
		dbg(1,"Disk image file \"%s\" is empty or does not exist.\nIt will be created if the client issues a format command.\n",disk_img_fname);

		// use file name to automatically set model 1 vs 2
		char ext[6] = {0};
		strncpy(ext,disk_img_fname+strlen(disk_img_fname)-5,5);
		if (!strcasecmp(ext,DEFAULT_TPDD1_IMG_SUFFIX)) model = 1;
		else if (!strcasecmp(ext,DEFAULT_TPDD2_IMG_SUFFIX)) model = 2;
	}

	// rewrite with leading path if not already
	// because we we may cd all over the place
	if (disk_img_fname[0]=='/') return 0;
	char t[PATH_MAX+1] = {0};
	strcpy(t,iwd);
	strcat(t,"/");
	strcat(t,disk_img_fname);
	memcpy(disk_img_fname,t,PATH_MAX+1);

	return 0;
}

// search for TTY(s) matching TTY_PREFIX
void find_ttys (char* f) {
	dbg(3,"%s(%s)\n",__func__,f);

	// open /dev
	char path[] = "/dev/";
	DIR* dir = opendir(path);
	if (!dir){dbg(0,"Cannot open \"%s\"\n",path); return;}

	// read /dev, look for all files beginning with prefix
	// add any matches to ttys[]
	char** ttys = malloc(sizeof(char*));
	struct dirent *files;
	uint16_t nttys = 0, l=strlen(f);

	dbg(2,"Searching for \"%s%s*\"\n",path,f);
	while ((files = readdir(dir))) {
		if (strncmp(files->d_name,f,l)) continue;
		nttys++;
		ttys = realloc(ttys, (nttys+1) * sizeof(char(*)));
		ttys[nttys] = files->d_name;
	}

	closedir(dir);

	int i=0;
	if (nttys==1) i=1; // if there is only one element in ttys[], use it
	if (nttys>1) while (!i) { // if more than 1 then menu
		dbg(0,"\n");
		for (i=1;i<=nttys;i++) dbg(0,"%d) %s\n",i,ttys[i]);
		i=0; char a[6]={0};
		dbg(0,"Which serial port is the TPDD client on (1-%d) ? ",nttys);
		if (fgets(a,sizeof(a),stdin)) i=atoi(a);
		if (i<1 || i>nttys) i=0;
		if (a[0]=='q'||a[0]=='Q') break;
	}

	// set client_tty_name[] with the final result
	client_tty_name[0]=0x00;
	if (i) {
		strcpy(client_tty_name,path);
		strcat(client_tty_name,ttys[i]);
	}

	free(ttys);
}

// take the user-supplied tty arg and figure out the actual /dev/ttyfoo
void resolve_client_tty_name () {
	dbg(3,"%s()\n",__func__);
	switch (client_tty_name[0]) {
		case 0x00:
			// nothing supplied, scan for any ttys matching the default prefix
			find_ttys(TTY_PREFIX);
			break;
		case '-':
			// stdin/stdout mode, silence all messages - untested
			debug = -1;
			strcpy (client_tty_name,"/dev/tty");
			client_tty_fd=1;
			break;
		default:
			// something given, try with and without prepending /dev/
			if (!access(client_tty_name,F_OK)) break;
			char t[PATH_MAX+1]={0x00};
			int i = 0;
			strcpy(t,client_tty_name);
			strcpy(client_tty_name,"/dev/");
			if (!strncmp(client_tty_name,t,5)) i=5;
			strcat(client_tty_name,t+i);
	}
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

int open_client_tty () {
	dbg(3,"%s()\n",__func__);

	if (!client_tty_name[0]) { show_main_help() ;dbg(0,"Error: No serial device specified\n(searched: /dev/%s*)\n",TTY_PREFIX); return 1; }

	dbg(0,"Opening \"%s\" ... ",client_tty_name);
	// open with O_NONBLOCK to avoid hang if client not ready, then unset later.
	if (client_tty_fd<0) client_tty_fd=open((char *)client_tty_name,O_RDWR|O_NOCTTY|O_NONBLOCK);
	if (client_tty_fd<0) { dbg(0,"%s\n",strerror(errno)); return 1; }
	dbg(0,"OK\n");

#ifdef TIOCEXCL
	ioctl(client_tty_fd,TIOCEXCL);
#endif

#if !defined(_WIN)
	if (getty_mode) {
		debug = -1;
		if (!login_tty(client_tty_fd)) client_tty_fd = STDIN_FILENO;
		else (void)!daemon(1,1);
	}
#endif

	(void)!tcflush(client_tty_fd, TCIOFLUSH);

	// unset O_NONBLOCK
	fcntl(client_tty_fd, F_SETFL, fcntl(client_tty_fd, F_GETFL, NULL) & ~O_NONBLOCK);

	if (tcgetattr(client_tty_fd,&client_termios)==-1) return 21;

	cfmakeraw(&client_termios);
	client_termios.c_cflag |= CLOCAL|CS8;

	if (rtscts) client_termios.c_cflag |= CRTSCTS;
	else client_termios.c_cflag &= ~CRTSCTS;

	if (cfsetspeed(&client_termios,itor(baud))==-1) return 22;

	if (tcsetattr(client_tty_fd,TCSANOW,&client_termios)==-1) return 23;

	client_tty_vmt(-2,-2);

	return 0;
}

int write_client_tty(void* b, int n) {
	dbg(4,"%s(%u)\n",__func__,n);
	n = write(client_tty_fd,b,n);
	dbg(3,"SENT: "); dbg_b(3,b,n);
	return n;
}

// It is correct that this blocks and waits forever.
// The one time we don't want to block, we don't use this.
int read_client_tty(void* b, const unsigned int n) {
	dbg(4,"%s(%u)\n",__func__,n);
	unsigned t = 0;
	int i = 0;
	while (t<n) if ((i = read(client_tty_fd, b+t, n-t))) t+=i;
	if (i<0) {
		dbg(0,"error: %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	dbg(3,"RCVD: "); dbg_b(3,b,n);
	return t;
}

// cat a file to terminal, for custom loader directions in bootstrap()
void dcat(char* f) {
	char b[4097]={0x00};
	int h=open(f,O_RDONLY);
	if (h<0) return;
	while (read(h,&b,4096)>0) dbg(0,"%s",b);
	close(h);
}

/*
 * The manual says:
 *
 * "The checksum is the one's complement of the least significant byte
 *  of the number of bytes from the block format through the data block."
 *
 * But the bytes are summed, not just counted!
 * Replace "number of" with "sum of the".
 *
 * Sum all the bytes in the specified range.
 * Take the least significant byte of that sum.
 * Invert all the bits in that byte.
 *
 * b[0] = cmd  (block format)
 * b[1] = len
 * b[2] to b[1+len] = 0 to 128 bytes of payload  (data block)
 * ignore everything after b[1+len]
 */
uint8_t checksum(unsigned char* b) {
	uint16_t s=0; uint8_t i, l=2+b[1];
	for (i=0;i<l;i++) s+=b[i];
	return ~(s&0xFF);
}

char* collapse_padded_fname(char* fname) {
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

void lsx (char* path,char* match,char* fmt) {
	struct dirent *files;
	DIR *dir = opendir(path);
	if (!dir){dbg(0,"Cannot open \"%s\"",path); return;}
	int i;
	while ((files = readdir(dir))) {
		for (i=strlen(files->d_name);files->d_name[i]!='.';i--);
		if (!strcmp(files->d_name+i+1,match)) dbg(0,fmt,files->d_name);
	}
	closedir(dir);
}

int check_magic_file(char* b) {
	dbg(3,"%s(\"%s\")\n",__func__,b);
	if (!enable_magic_files) return 1;
	int l = sizeof(magic_files)/sizeof(magic_files[0]);
	for (int i=0;i<l;++i) if (!strcmp(magic_files[i],b)) return 0;
	return 1;
}

// This is kind of silly but why not. Load a rom image file into rom[],
// then tpdd2 mem_read() in the ROM address range returns data from rom[].
void load_rom(char* f) {
	dbg(3,"%s(%s)\n",__func__,f);
	char t[PATH_MAX+1] = {0x00};
	strncpy(t,f,PATH_MAX);
	find_lib_file(t);
	int h = open(t,O_RDONLY);
	if (h<0) return;
	(void)!read(h,rom,ROM_LEN);
	close(h);
	dbg_b(3,rom,ROM_LEN);
}

////////////////////////////////////////////////////////////////////////
//
//  FDC MODE
//

/*
 * sectors: 0-79
 * sector: 1293 bytes
 * | LSC 1 byte | ID 12 bytes | DATA 1280 bytes |
 * LSC: logical sector size code
 * ID: 12 bytes of arbitrary data, searchable by req_fdc_search_id()
 * DATA: 1280 bytes of arbitrary data, read/writable in lsc_to_len(LSC)-sized chunks
 */

// standard fdc-mode 8-byte response
// e = error code ERR_FDC_* -> ascii hex pair
// s = status or data       -> ascii hex pair
// l = length or address    -> 2 ascii hex pairs
// TODO - don't assume endianness
void ret_fdc_std(uint8_t e, uint8_t s, uint16_t l) {
	dbg(2,"%s()\n",__func__);
	char b[9] = { 0x00 };
	snprintf(b,9,"%02X%02X%04X",e,s,l);
	dbg(2,"FDC: response: \"%s\"\n",b);
	write_client_tty(b,8);
}

// p   : physical sector to seek to
// m   : mode read-only / write-only / read-write
int open_disk_image (int p, int m) {
	dbg(2,"%s(%d,%d)\n",__func__,p,m);
	int of; int e=ERR_FDC_SUCCESS;

	if (!*disk_img_fname) e=ERR_FDC_NO_DISK;

	if (!e) switch (m) {
		case O_RDWR: of=O_RDWR; dbg(2,"edit rw\n");
			if (access(disk_img_fname,W_OK)) e=ERR_FDC_WRITE_PROTECT;
			break;
		case O_WRONLY: of=O_WRONLY;
			if (access(disk_img_fname,F_OK)) { of|=O_CREAT; dbg(2,"create\n");} else {
				dbg(2,"edit wo\n");
				if (access(disk_img_fname,W_OK)) e=ERR_FDC_WRITE_PROTECT;
			}
			break;
		default: of=O_RDONLY; dbg(2,"read\n"); break;
	}

	if (!e) {
		disk_img_fd=open(disk_img_fname,of|O_EXCL,0666);
		if (disk_img_fd<0) { dbg(0,"%s\n",strerror(errno)) ;e=ERR_FDC_READ;}
	}

	if (!e) {
		int s = (p*SECTOR_LEN); // initial seek position to start of physical sector
		if (lseek(disk_img_fd,s,SEEK_SET)!=s) e=ERR_FDC_READ;
	}

	if (opr_mode) switch (e) {
		//case ERR_FDC_SUCCESS: e=ERR_SUCCESS; break; // same
		case ERR_FDC_NO_DISK: e=ERR_NO_DISK; break;
		case ERR_FDC_WRITE_PROTECT: e=ERR_WRITE_PROTECT; break;
		case ERR_FDC_READ: e=ERR_READ_TIMEOUT; break;
	}

	return e;
}

void req_fdc_set_mode(uint8_t m) {
	dbg(2,"%s(%d)\n",__func__,m);
	opr_mode=m; // no response, just switch modes
	if (m==1) dbg(2,"Switched to \"Operation\" mode\n");
}

// disk state
// ret_fdc_std(e,s,l)
// e = ERR_FDC_SUCCESS
// s =
//   bit 7 = disk not inserted
//   bit 6 = disk changed
//   bit 5 = disk write-protected
//   0-4 not used
// l = 0
void req_fdc_condition() {
	dbg(2,"%s()\n",__func__);
	ret_fdc_std(ERR_FDC_SUCCESS,pdd1_condition,0);
}

// lc = logical sector size code
void req_fdc_format(uint8_t lc) {
	dbg(2,"%s(%d)\n",__func__,lc);
	uint16_t ll = FDC_LOGICAL_SECTOR_SIZE[lc];
	uint8_t rn = 0;     // physical sector number
	uint8_t rc = (PDD1_TRACKS*PDD1_SECTORS); // total record count

	dbg(0,"Format: Logical sector size: %d = %d\n",lc,ll);

	uint8_t e = open_disk_image(0,O_RDWR);
	if (e) { ret_fdc_std(e,0,0); return; }

	memset(rb,0x00,SECTOR_LEN);
	rb[0]=lc; // logical sector size code
	for (rn=0;rn<rc;rn++) {
		if (write(disk_img_fd,rb,SECTOR_LEN)<0) {
			dbg(0,"%s\n",strerror(errno));
			e = ERR_FDC_READ;
			break;
		}
	}

	close(disk_img_fd);
	if (!e) rn = 0;
	ret_fdc_std(e,rn,0);
}

// read ID section of a sector
// p = physical sector number 0-79
void req_fdc_read_id(uint8_t p) {
	dbg(2,"%s(%d)\n",__func__,p);

	uint8_t e = open_disk_image(p,O_RDONLY);
	if (e) { ret_fdc_std(e,0,0); return; }

	int r = read(disk_img_fd,rb,SECTOR_HEADER_LEN);
	close(disk_img_fd);
	dbg_b(2,rb,SECTOR_HEADER_LEN);
	if(r!=SECTOR_HEADER_LEN) {
		ret_fdc_std(ERR_FDC_READ,p,0);
		return;
	}
 
	uint16_t l = FDC_LOGICAL_SECTOR_SIZE[rb[0]];          // get logical size from header
	ret_fdc_std(ERR_FDC_SUCCESS,p,l);   // send OK
	char t=0x00;
	read_client_tty(&t,1); // read 1 byte from client
	if (t==FDC_CMD_EOL) write_client_tty(rb+1,r-1); // if 0D send data else silently abort
}

// read DATA section of a sector
// tp = target physical sector 0-79
// tl = target logical sector 1-20
void req_fdc_read_sector(uint8_t tp,uint8_t tl) {
	dbg(2,"%s(%d,%d)\n",__func__,tp,tl);

	uint8_t e = open_disk_image(tp,O_RDONLY);
	if (e) { ret_fdc_std(e,0,0); return; }

	if (read(disk_img_fd,rb,SECTOR_HEADER_LEN)!=SECTOR_HEADER_LEN) { // read header
		dbg(1,"failed read header\n");
		close(disk_img_fd);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}
	dbg_b(3,rb,SECTOR_HEADER_LEN);

	uint16_t l = FDC_LOGICAL_SECTOR_SIZE[rb[0]]; // get logical size from header
	if (l*tl>SECTOR_DATA_LEN) {
		close(disk_img_fd);
		ret_fdc_std(ERR_FDC_LSN_HI,tp,l);
		return;
	}

	// seek to target_physical*(id_len+physical_len) + id_len + (target_logical-1)*logical_len
	int s = (tp*SECTOR_LEN)+SECTOR_HEADER_LEN+((tl-1)*l);
	if (lseek(disk_img_fd,s,SEEK_SET)!=s) {
		dbg(1,"failed seek %d : %s\n",s,strerror(errno));
		close(disk_img_fd);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}
	memset(rb,0x00,l);
	if (read(disk_img_fd,rb,l)!=l) { // read one logical sector of DATA
		dbg(1,"failed logical sector read\n");
		close(disk_img_fd);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}
	close(disk_img_fd);
	ret_fdc_std(ERR_FDC_SUCCESS,tp,l); // 1st stage response
	char t=0x00;
	read_client_tty(&t,1); // read 1 byte from client
	if (t==FDC_CMD_EOL) write_client_tty(rb,l); // if 0D send data else silently abort
}

// ref/search_id_section.txt
void req_fdc_search_id() {
	dbg(2,"%s()\n",__func__);
	int rn = 0;     // physical sector number
	int rc = (PDD1_TRACKS*PDD1_SECTORS); // total record count
	char sb[SECTOR_ID_LEN] = {0x00}; // search data

	uint8_t e = open_disk_image(0,O_RDONLY);
	if (e) { ret_fdc_std(e,0,0); return; }

	ret_fdc_std(ERR_FDC_SUCCESS,0,0); // tell client to send data
	read_client_tty(sb,SECTOR_ID_LEN); // read 12 bytes from client

	uint16_t l = 0;
	bool found = false;
	for (rn=0;rn<rc;rn++) {
		memset(rb,0x00,SECTOR_HEADER_LEN);
		if (read(disk_img_fd,rb,SECTOR_LEN)!=SECTOR_LEN) {  // read one record
			dbg(0,"%s\n",strerror(errno));
			close(disk_img_fd);
			ret_fdc_std(ERR_FDC_READ,rn,0);
			return;
		}

		dbg(3,"%d ",rn);
		dbg_b(3,rb,SECTOR_HEADER_LEN);

		l = FDC_LOGICAL_SECTOR_SIZE[rb[0]]; // get logical size from header

		// does sb exactly match ID?
		if (!strncmp(sb,(char*)rb+1,SECTOR_ID_LEN)) {
			found = true;
			break;
		}
	}
	close(disk_img_fd);

	if (found) {
		ret_fdc_std(ERR_FDC_SUCCESS,rn,l);
	} else {
		ret_fdc_std(ERR_FDC_ID_NOT_FOUND,255,l);
	}
}

void req_fdc_write_id(int tp) {
	dbg(2,"%s(%d)\n",__func__,tp);

	uint8_t e = open_disk_image(tp,O_RDWR);
	if (e) { ret_fdc_std(e,0,0); return; }

	if (read(disk_img_fd,rb,1)!=1) { // read LSC
		dbg(0,"failed to read LSC\n");
		close(disk_img_fd);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}

	uint16_t l = FDC_LOGICAL_SECTOR_SIZE[rb[0]]; // get logical size from LSC

	ret_fdc_std(ERR_FDC_SUCCESS,tp,l); // tell client to send data

	read_client_tty(rb,SECTOR_ID_LEN); // read 12 bytes from client

	// write those to the file
	if (write(disk_img_fd,rb,SECTOR_ID_LEN)<0) {
		dbg(0,"%s\n",strerror(errno));
		e = ERR_FDC_READ;
		l = 0;
	}

	close(disk_img_fd);
	ret_fdc_std(e,tp,l); // send final response to client
}

void req_fdc_write_sector(int tp,int tl) {
	dbg(2,"%s(%d,%d)\n",__func__,tp,tl);

	uint8_t e = open_disk_image(tp,O_RDWR);
	if (e) { ret_fdc_std(e,0,0); return; }

	if (read(disk_img_fd,rb,SECTOR_HEADER_LEN)!=SECTOR_HEADER_LEN) { // read header
		dbg(0,"failed read ID\n");
		close(disk_img_fd);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}

	uint16_t l = FDC_LOGICAL_SECTOR_SIZE[rb[0]]; // get logical size from header

	// seek to target_physical*full_sectors + header + target_logical*logical_size
	int s = (tp*SECTOR_LEN)+SECTOR_HEADER_LEN+((tl-1)*l);
	if (lseek(disk_img_fd,s,SEEK_SET)!=s) {
		dbg(0,"failed seek %d : %s\n",s,strerror(errno));
		close(disk_img_fd);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}

	ret_fdc_std(ERR_FDC_SUCCESS,tp,l); // tell client to send data

	read_client_tty(rb,l); // read logical_size bytes from client

	// write them to the file
	if (write(disk_img_fd,rb,l)<0) {
		dbg(0,"%s\n",strerror(errno));
		close(disk_img_fd);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}

	close(disk_img_fd); // close file

	ret_fdc_std(ERR_FDC_SUCCESS,tp,l); // send final OK to client
}

// ref/fdc.txt
void get_fdc_cmd() {
	dbg(3,"%s()\n",__func__);
	uint8_t i = 0;
	bool eol = false;
	uint8_t c = 0x00;
	int p = -1;
	int l = -1;

	memset(gb,0x00,TPDD_MSG_MAX);
	if (ch[0]==0xFF) ch[0]=0x00; // blech figure out something better

	// scan for a valid command byte first
	while (!c) {
		if (ch[0]) {
			c = ch[0];
			ch[0] = 0x00;
			dbg(3,"Restored from req_fdc(): 0x%02X\n",c);
		} else {
			read_client_tty(&c,1);
		}
		if (c==FDC_CMD_EOL) { eol=true; c=0x20; break; } // fall through to ERR_FDC_COMMAND, important for Sardine
		if (!strchr(FDC_CMDS,c)) c=0x20 ; // eat bytes until valid cmd or eol
	}

	// read params
	i = 0;
	while (i<6 && !eol) {  // max params is "##,##"
		if (read_client_tty(&gb[i],1)==1) {
			dbg(3,"i:%d gb[]:\n%s\n",i,gb);
			switch (gb[i]) {
				case FDC_CMD_EOL: eol=true;    // fall through
				case 0x20: gb[i]=0x00; break;  // if 1st byte after cmd is space, ignore it
				default: i++;
			}
		}
	}

	// We can pre-parse & validate the params since they take the same
	// form (or a consistent subset) for all commands.
	// Parameters, if they exist, are always one of:
	//   P,L
	//   P
	//   <none>
	// where:
	// P = physical sector number 0-79 (decimal integer as 0-2 ascii characters)
	// L = logical sector number 1-20 (decimal integer as 0-2 ascii characters)
	// (P & L sometimes have other meanings but the format & type rule still holds)
	p=0; // real drive uses physical sector 0 when omitted
	l=1; // real drive uses logical sector 1 when omitted
	char* t;
	if ((t=strtok((char*)gb,","))!=NULL) p=atoi(t); // target physical sector number
	if ((t=strtok(NULL,","))!=NULL) l=atoi(t); // target logical sector number
	// for physical sector out of range, real drive error response will have dat=last_valid_p if any
	// if no command has ever supplied a valid physical sector number yet, then dat=FF
	if (p<0) {ret_fdc_std(ERR_FDC_PARAM,0xFF,0); return;}
	if (p>79) {ret_fdc_std(ERR_FDC_PSN_HI,0xFF,0); return;}
	if (l<1) {ret_fdc_std(ERR_FDC_LSN_LO,p,0); return;}
	if (l>20) {ret_fdc_std(ERR_FDC_LSN_HI,p,0); return;}

	// debug
	dbg(3,"command:%c  physical:%d  logical:%d\n",c,p,l);

	// dispatch
	switch (c) {
		case FDC_SET_MODE:        req_fdc_set_mode(p);        break;
		case FDC_CONDITION:       req_fdc_condition();        break;
		case FDC_FORMAT_NV:
		case FDC_FORMAT:          req_fdc_format(p);          break;
		case FDC_READ_ID:         req_fdc_read_id(p);         break;
		case FDC_READ_SECTOR:     req_fdc_read_sector(p,l);   break;
		case FDC_SEARCH_ID:       req_fdc_search_id();        break;
		case FDC_WRITE_ID_NV:
		case FDC_WRITE_ID:        req_fdc_write_id(p);        break;
		case FDC_WRITE_SECTOR_NV:
		case FDC_WRITE_SECTOR:    req_fdc_write_sector(p,l);  break;
		default: dbg(2,"FDC: invalid cmd \"%s\"\n",gb);
			ret_fdc_std(ERR_FDC_COMMAND,0,0); // required for model detection
	}
}

////////////////////////////////////////////////////////////////////////
//
//  OPERATION MODE
//

FILE_ENTRY* make_file_entry(char* namep, uint16_t len, char flags) {
	dbg(3,"%s(\"%s\")\n",__func__,namep);
	static FILE_ENTRY f;
	int i;

	strncpy (f.local_fname, namep, sizeof (f.local_fname) - 1);
	memset(f.client_fname,0x20,TPDD_FILENAME_LEN);
	f.len = len;
	f.flags = flags;

	if (dot_offset) {
		// if not in raw mode, reformat the client filename

		// find the last dot in the local filename
		for(i=strlen(namep);i>0;i--) if (namep[i]=='.') break;

		// write client extension
		if (flags&FE_FLAGS_DIR) {
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

		// upcase
		if (upcase) for(i=0;i<TPDD_FILENAME_LEN;i++) f.client_fname[i]=toupper(f.client_fname[i]);

		// nulls to spaces
		if (f.client_fname[dot_offset+1]==0x00) f.client_fname[dot_offset+1]=0x20;
		if (f.client_fname[dot_offset+2]==0x00) f.client_fname[dot_offset+2]=0x20;

		// dot and null terminator
		f.client_fname[dot_offset]='.';
		f.client_fname[dot_offset+3]=0x00;

	} else {
		// raw mode - don't reformat or filter anything
		snprintf(f.client_fname,25,"%-*.*s",TPDD_FILENAME_LEN,TPDD_FILENAME_LEN,namep);
	}

	dbg(1,"\"%s\"\t%s%s\n",f.client_fname,f.local_fname,f.flags&FE_FLAGS_DIR?"/":"");
	return &f;
}

// standard return - return for: error open close delete status write
void ret_std(unsigned char err) {
	dbg(3,"%s()\n",__func__);
	gb[0] = RET_STD[0];
	gb[1] = RET_STD[1];
	gb[2] = err;
	gb[3] = checksum(gb);
	dbg(3,"Response: %02X\n",err);
	write_client_tty(gb,gb[1]+3);
	if (gb[2]!=ERR_SUCCESS) dbg(2,"ERROR RESPONSE TO CLIENT\n");
}

int read_next_dirent(DIR* dir,int m) {
	dbg(3,"%s()\n",__func__);
	struct stat st;
	struct dirent* dire;
	int flags;

	if (dir == NULL) {
		dire=NULL;
		dbg(0,"%s(NULL) ???\n",__func__);
		if (m) ret_std(ERR_NO_DISK);
		return 0;
	}

	while ((dire=readdir(dir)) != NULL) {
		flags=FE_FLAGS_NONE;

		if (stat(dire->d_name,&st)) {
			if (m) ret_std(ERR_NO_FILE);
			return 0;
		}

		if (S_ISDIR(st.st_mode)) flags=FE_FLAGS_DIR;
		else if (!S_ISREG (st.st_mode)) continue;

		if (flags==FE_FLAGS_DIR && dme<2) continue;

		if (dot_offset) {
			if (dire->d_name[0]=='.') continue; // skip "." ".." and hidden files
			if (strlen(dire->d_name)>LOCAL_FILENAME_MAX) continue; // skip long filenames
		}

		// If filesize is too large for the tpdd 16 bit size field, then say
		// size=0 but allow the file to be accessed, because cpmupd.CO for
		// REXCPM violates the tpdd protocol to load a large CP/M disk image.
		if (st.st_size>UINT16_MAX) st.st_size=0;

		add_file(make_file_entry(dire->d_name, st.st_size, flags));
		break;
	}

	if (dire == NULL) return 0;

	return 1;
}

// read the current share directory
void update_file_list(int m) {
	dbg(3,"%s()\n",__func__);
	DIR* dir;

	if (model==2) cd_share_path();
	dir = opendir(".");
	file_list_clear_all();

	int w = dot_offset+3;
	if (dot_offset<1||w>TPDD_FILENAME_LEN) w = TPDD_FILENAME_LEN;
	dbg(1,"\nDirectory %s: %s\n",model==2?bank==1?"[Bank 1]":"[Bank 0]":"",cwd);
	dbg(1," %-*s \tlocal filename\n",w,"tpdd view");
	dbg(1,"-------------------------------------------------------------------------------\n");
	if (dir_depth) add_file(make_file_entry("..", 0, FE_FLAGS_DIR));
	while (read_next_dirent(dir,m));
	dbg(1,"-------------------------------------------------------------------------------\n");
	closedir(dir);
}

// return for dirent
int ret_dirent(FILE_ENTRY* ep) {
	dbg(2,"%s(\"%s\")\n",__func__,ep->client_fname);
	int i;

	memset(gb,0x00,TPDD_MSG_MAX);
	gb[0] = RET_DIRENT[0];
	gb[1] = RET_DIRENT[1];

	if (ep) {
		// name
		memset (gb + 2, ' ', TPDD_FILENAME_LEN);
		if (dot_offset) for (i=0;i<dot_offset+3;i++)
			gb[i+2] = (ep->client_fname[i])?ep->client_fname[i]:' ';
		else memcpy (gb+2,ep->client_fname,TPDD_FILENAME_LEN);

		// attribute
		gb[26] = default_attr;

		// size
		gb[27] = (uint8_t)(ep->len >> 0x08); // most significant byte
		gb[28] = (uint8_t)(ep->len & 0xFF);  // least significant byte
	}

	dbg(3,"\"%*.*s\"\n",TPDD_FILENAME_LEN,TPDD_FILENAME_LEN,gb+2);

	// free sectors
	gb[29] = model==2?(PDD2_TRACKS*PDD2_SECTORS):(PDD1_TRACKS*PDD1_SECTORS);

	gb[30] = checksum (gb);

	return (write_client_tty(gb,31) == 31);
}

void dirent_set_name() {
	dbg(2,"%s(%-*.*s)\n",__func__,TPDD_FILENAME_LEN,TPDD_FILENAME_LEN,gb+2);
	char* p;
	char filename[TPDD_FILENAME_LEN+1]={0x00};
	int f = 0;
	if (gb[2]) {
		dbg(3,"filename: \"%-*.*s\"\n",TPDD_FILENAME_LEN,TPDD_FILENAME_LEN,gb+2);
		dbg(3,"    attr: \"%c\" (%1$02X)\n",gb[26]);
	}
	// update before every set-name for at least 2 reasons
	// * clients may open files without ever listing (teeny, ur2, etc)
	// * local files may be changed at any time by other processes
	update_file_list(ALLOW_RET);
	strncpy(filename,(char*)gb+2,TPDD_FILENAME_LEN);
	filename[TPDD_FILENAME_LEN]=0;
	// Remove trailing spaces
	for (p = strrchr(filename,' '); p >= filename && *p == ' '; p--) *p = 0x00;
		cur_file=find_file(filename);
	if (cur_file) {
		dbg(3,"Exists: \"%s\"  %u\n", cur_file->local_fname, cur_file->len);
		ret_dirent(cur_file);
	} else if (!check_magic_file(filename)) {
		// let UR2/TSLOAD load DOSxxx.CO from anywhere
		cur_file=make_file_entry(filename,0,0);
		char t[LOCAL_FILENAME_MAX+1] = {0x00};
		// try share root
		// TODO - save initial path in share_path[0] and use that instead of "../"*depth
		for (int i=dir_depth;i>0;i--) strcat(t,"../");
		strncat(t,cur_file->local_fname,LOCAL_FILENAME_MAX-dir_depth*3);
		struct stat st; int e=stat(t,&st);
		if (e) { // try app_lib_dir
			strcpy(t,app_lib_dir);
			strcat(t,"/");
			strcat(t,cur_file->local_fname);
			e=stat(t,&st);
		}
		if (e) ret_dirent(NULL); // not found
		else { // found in share root or in app_lib_dir
			strcpy(cur_file->local_fname,t);
			cur_file->len=st.st_size;
			dbg(3,"Magic: \"%s\" <-- \"%s\"\n",cur_file->client_fname,cur_file->local_fname);
			ret_dirent(cur_file);
		}
	} else {
		if (!strncmp(filename+dot_offset+1,dme_dir_label,2)) f = FE_FLAGS_DIR;
		cur_file=make_file_entry(collapse_padded_fname(filename), 0, f);
		dbg(3,"New %s: \"%s\"\n",f==FE_FLAGS_DIR?"Directory":"File",cur_file->local_fname);
		ret_dirent(NULL);
	}
}

void dirent_get_first() {
	dbg(2,"Directory Listing\n");
	// update every time before get-first,
	// because set-name is not required before get-first
	update_file_list(ALLOW_RET);
	ret_dirent(get_first_file());
	dme = 0;
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
int req_dirent() {
	if (debug>1) {
		dbg(2,"%s(%s)\n",__func__,
			gb[27]==DIRENT_SET_NAME?"set_name":
			gb[27]==DIRENT_GET_FIRST?"get_first":
			gb[27]==DIRENT_GET_NEXT?"get_next":
			gb[27]==DIRENT_GET_PREV?"get_prev":
			gb[27]==DIRENT_CLOSE?"close":
			"UNKNOWN"
		);
		dbg(5,"gb[]\n");
		dbg_b(5,gb,-1);
		dbg_p(4,gb);
	}

	switch (gb[27]) {
		case DIRENT_SET_NAME:  dirent_set_name();           break;
		case DIRENT_GET_FIRST: dirent_get_first();          break;
		case DIRENT_GET_NEXT:  ret_dirent(get_next_file()); break;
		case DIRENT_GET_PREV:  ret_dirent(get_prev_file()); break;
		case DIRENT_CLOSE:                                  break;
	}
	return 0;
}

// update dme_cwd with current dir, truncated & padded both required
// If you don't send all 6 bytes, TS-DOS doesn't clear the previous
// contents from the display
void update_dme_cwd() {
	dbg(2,"%s()\n",__func__);
	if (dme_disabled) return;

	int i;
	update_cwd();
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
	if (dme_disabled) return;
	gb[0] = RET_STD[0];
	gb[1] = 0x0B;   // not RET_STD[1] because TS-DOS DME violates the spec
	gb[2] = 0x00;   // don't know why this byte is 0
	memcpy(gb+3,dme_cwd,6); // 6 bytes 3-8 display in top-right corner
	gb[9] = 0x00;   // gb[9]='.';  // remaining contents don't matter but length does
	gb[10] = 0x00;  // gb[10]=dme_dir_label[0];
	gb[11] = 0x00;  // gb[11]=dme_dir_label[1];
	gb[12] = 0x00;  // gb[12]=0x20;
	gb[13] = checksum(gb);
	write_client_tty(gb,14);
}

// The "switch to FDC-mode" command requires careful handling, because
// unlike the original Desk-Link, we actually support the FDC commands,
// and need the "switch-to-fdc-mode" command to work like a real drive.
//
// So here we always look for TS-DOS "DME" request and set a "we're doing dme"
// flag, but only for the duration of a single directory listing process.
// The first stage of a directory listing, dirent(get-first), clears the dme
// flag so that FDC commands immediatly go back to working like a real drive.
//
// Any FDC request might actually be a DME request
// See ref/dme.txt for the full explaination
void req_fdc() {
	dbg(2,"%s()\n",__func__);

	// TPDD1 does not send back any response
	// TPDD2 returns a standard 0x12 return packet with 0x36 payload
	//
	// You can't have both full TPDD2 emulation including banks,
	// and TS-DOS directories support at the same time.
	//
	// If we recognize a DME request and respond with a DME return, then you get
	// TS-DOS subdirecties, but then TS-DOS does not show a Bank button,
	// even if we had otherwise responded as a TPDD2 ie with the tpdd2 version
	// packet and working tpdd2-only features like dirent(get-prev).
	//
	// If we are in tpdd2 mode and reject the DME request like a real tpdd2,
	// then TS-DOS does show the Bank button and you can switch banks 0 & 1.
	if (model==2) { ret_std(ERR_PARAM); return; }

	// Some versions of TS-DOS send 2 FDC requests in a row, both with trailing
	// 0x0D. Some versions also send a 3rd FDC request without the trailing 0x0D.
	// Look for 2 consecutive FDC requests with trailing 0x0D. Once we see that,
	// don't try to read a trailing 0x0D any more to avoid reading the command
	// byte of a real FDC command, and respond to the 2nd and any other FDC
	// requests with DME response instead of switching to FDC mode, as long as dme>1.
	// dme is only set here, and only unset in dirent()
	if (dme<2 && !dme_disabled) {
		// Try to read one more byte, and store it in ch[0] where get_fdc_req()
		// can pick it up in case it was NOT the trailing 0x0D of a DME request
		// but instead was the first byte of an actual FDC command.
		// Timeout fast whether there is a byte or not.
		//dbg(3,"looking for dme req %d of 2\n",dme+1);
		ch[0] = 0x00;
		client_tty_vmt(0,1);   // allow this read to time out, and fast
		(void)!read(client_tty_fd,ch,1);
		client_tty_vmt(-1,-1); // restore normal VMIN/VTIME
		if (ch[0]==FDC_CMD_EOL) dbg(3,"Got dme req %d of 2\n",++dme);
		//if (ch[0]) dbg(3,"ate a byte: %02X\n",ch[0]);
	}
	if (dme>1) {
		ret_dme_cwd();
	} else {
		opr_mode = 0;
		dbg(2,"Switched to \"FDC\" mode\n"); // no response to client, just switch modes
	}
}

// b[0] = fmt  0x01
// b[1] = len  0x01
// b[2] = mode 0x01 write new
//             0x02 write append
//             0x03 read
// b[3] = chk
int req_open() {
	if (debug>1) {
		dbg(2,"%s(\"%s\")\n",__func__,cur_file->client_fname);
		dbg(5,"gb[]\n");
		dbg_b(5,gb,-1);
		dbg_p(4,gb);
	}

	uint8_t omode = gb[2];

	switch(omode) {
		case F_OPEN_WRITE:
			dbg(2,"mode: write\n");
			if (o_file_h >= 0) {
				close(o_file_h);
				o_file_h=-1;
			}
			if (cur_file->flags&FE_FLAGS_DIR) {
				if (!mkdir(cur_file->local_fname,0755)) {
					ret_std(ERR_SUCCESS);
				} else {
					ret_std(ERR_FMT_MISMATCH);
				}
			} else {
				o_file_h = open(cur_file->local_fname,O_CREAT|O_TRUNC|O_WRONLY|O_EXCL,0666);
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
			dbg(2,"mode: append\n");
			if (o_file_h >= 0) {
				close(o_file_h);
				o_file_h=-1;
			}
			if (cur_file==0) {
				ret_std(ERR_FMT_MISMATCH);
				return -1;
			}
			o_file_h = open(cur_file->local_fname, O_WRONLY | O_APPEND);
			if (o_file_h < 0)
				ret_std(ERR_FMT_MISMATCH);
			else {
				f_open_mode=omode;
				dbg(1,"Open for append: \"%s\"\n",cur_file->local_fname);
				ret_std(ERR_SUCCESS);
			}
			break;
		case F_OPEN_READ:
			dbg(2,"mode: read\n");
			if (o_file_h >= 0) {
				close(o_file_h);
				o_file_h=-1;
			}
			if (cur_file==0) {
				ret_std(ERR_NO_FILE);
				return -1;
			}
	
			if (cur_file->flags&FE_FLAGS_DIR) {
				int err=0;
				// directory
				if (cur_file->local_fname[0]=='.' && cur_file->local_fname[1]=='.') {
					// parent dir
					if (dir_depth>0) {
						err=chdir(cur_file->local_fname);
						if (!err) dir_depth--;
					}
				} else {
					// enter dir
					err=chdir(cur_file->local_fname);
					if (!err) dir_depth++;
				}
				update_dme_cwd();
				if (err) ret_std(ERR_FMT_MISMATCH);
				else ret_std(ERR_SUCCESS);
			} else {
				// regular file
				o_file_h = open(cur_file->local_fname, O_RDONLY);
				if (o_file_h<0)
					ret_std(ERR_NO_FILE);
				else {
					f_open_mode = omode;
					dbg(1,"Open for read: \"%s\"\n",cur_file->local_fname);
					ret_std(ERR_SUCCESS);
				}
			}
			break;
		default:
			dbg(2,"Unrecognized mode: \"0x%02X\"\n",omode);
			ret_std(ERR_PARAM);
			break;
	}
	return o_file_h;
}

void req_read() {
	dbg(2,"%s()\n",__func__);
	int i;

	if (o_file_h<0) {
		ret_std(ERR_NO_FNAME);
		return;
	}
	if (f_open_mode!=F_OPEN_READ) {
		ret_std(ERR_FMT_MISMATCH);
		return;
	}

	i = read(o_file_h, gb+2, REQ_RW_DATA_MAX);

	gb[0] = RET_READ;
	gb[1] = (uint8_t)i;
	gb[2+i] = checksum(gb);

	if (debug<2) {
		dbg(1,".");
		if (i<REQ_RW_DATA_MAX) dbg(1,"\n"); // final packet
	}

	if (debug>1) {
		dbg(4,"...outgoing packet...\n");
		dbg(5,"gb[]\n");
		dbg_b(5,gb,-1);
		dbg_p(4,gb);
		dbg(4,".....................\n");
	}

	write_client_tty(gb, 3+i);
}

// b[0] = 0x04
// b[1] = 0x01 - 0x80
// b[2] = b[1] bytes
// b[2+len] = chk
void req_write() {
	if (debug>1) {
		dbg(2,"%s()\n",__func__);
		dbg(4,"...incoming packet...\n");
		dbg(5,"gb[]\n");
		dbg_b(5,gb,-1);
		dbg_p(4,gb);
		dbg(4,".....................\n");
	}

	if (o_file_h<0) {ret_std(ERR_NO_FNAME); return;}

	if (f_open_mode!=F_OPEN_WRITE && f_open_mode !=F_OPEN_APPEND) {
		ret_std(ERR_FMT_MISMATCH);
		return;
	}

	if (debug<2) {
		dbg(1,".");
		if (gb[1]<REQ_RW_DATA_MAX) dbg(1,"\n"); // final packet
	}

	if (write (o_file_h,gb+2,gb[1]) != gb[1]) ret_std (ERR_SECTOR_NUM);
	else ret_std (ERR_SUCCESS);
}

void req_delete() {
	dbg(2,"%s()\n",__func__);
	if (cur_file->flags&FE_FLAGS_DIR) rmdir(cur_file->local_fname);
	else unlink (cur_file->local_fname);
	dbg(1,"Deleted: %s\n",cur_file->local_fname);
	ret_std (ERR_SUCCESS);
}


/*
 * PDD2 cache load, cache commit, mem read, mem write
 * 
 * Emulating access to the sector cache is straightforward.
 * Emulating access to the cpu memory is less so.
 *
 * The command allows to read from anywhere in the cpus address space,
 * but we wouldn't know what to return for much of that.
 *
 * We recognize a few special addresses and just return "success"
 * for all other access to the cpu area without actually doing anything.
 *
 * cpu memory map:
 * 0000-001F cpu i/o port
 * 0080-00FF cpu internal ram 128 bytes
 * 4000-4002 gate array (floppy controller)
 * 8000-87FF ram 2k bytes
 * F000-FFFF cpu internal rom 4k bytes
 *
 * Some cpu_memory writes observed from common clients, not including ZZ or checksum:
 *
 * fmt        len    area   offset      data
 * BACKUP.BA
 * 0x31,      0x04,  0x01,  0x00,0x83,  0x00,
 * 0x31,      0x04,  0x01,  0x00,0x96,  0x00,
 * 0x31,      0x07,  0x01,  0x80,0x04,  0x16,0x00,0x00,0x00    (data varies) this is the only one we actually do anything
 *
 * TS-DOS
 * 0x31,      0x04,  0x01,  0x00,0x84,  0xFF,
 * 0x31,      0x04,  0x01,  0x00,0x96,  0x0F,
 * 0x31,      0x04,  0x01,  0x00,0x94,  0x0F,
 *
 * pdd2 service manual p102 says:
 *   Reset Drive Status
 *     write FF to 0084
 *     write 0F to 0096
 *     write 0F to 0094
 *
 */

// also the return format for mem_write and undocumented 0x0F
void ret_cache(uint8_t e) {
	dbg(3,"%s()\n",__func__);
	gb[0] = RET_CACHE[0];
	gb[1] = RET_CACHE[1];
	gb[2] = e;
	gb[3] = checksum(gb);
	write_client_tty(gb,4);
}

/*
 * Load a sector from disk into ram[],
 * or commit ram[] to a sector on the disk.
 *
 * Committing the cache to disk does NOT clear the cache in ram.
 *
 * Load/Commit Cache
 * b[0] fmt 0x30
 * b[1] len 0x05
 *   b[2] action 0=load (cache<disk) 1=commit (cache>disk) 2=commit+verify
 *   b[3] track msb - (always 00)
 *   b[4] track lsb - 00-4F
 *   b[5] side (always 00)
 *   b[6] sector 0-1
 */
void req_cache() {
	dbg(3,"%s(action=%u track=%u sector=%u)\n",__func__,gb[2],gb[4],gb[6]);
	if (model==1) return;
	uint8_t a=gb[2];
	//uint_16_t t=b[3]*256+b[4]; // b[3] is always 0
	uint8_t t=gb[4];
	//int d=gb[5]; // side#? - always 0
	uint8_t s=gb[6]; // sector
	if (t>=PDD2_TRACKS || s>=PDD2_SECTORS) { ret_cache(ERR_PARAM); return; }
	uint8_t rn = t*2 + s; // convert track#:sector# to linear record#
	uint8_t e = ERR_SUCCESS;

	switch (a) {
		case CACHE_LOAD:
			dbg(2,"cache load: track:%u  sector:%u\n",t,s);

			// open disk image file and seek to record number
			if ((e = open_disk_image(rn,O_RDONLY))) break;

			// virtual 2k drive ram
			memset(ram,0x00,RAM_LEN); // 2k ram at 0x8000 - 0x87FF
			ram[0]=PDD2_CACHE_LEN_MSB; // len MSB - always 0x05
			ram[1]=PDD2_CACHE_LEN_LSB; // len LSB - always 0x13
			ram[2]=rn;   // linear sector number (0-159)
			//ram[0x03]=0x00; // side number? - always 0
			if (read(disk_img_fd,ram+PDD2_ID_REL,SECTOR_HEADER_LEN)!=SECTOR_HEADER_LEN) { e = ERR_DEFECTIVE; break; }
			//ram[0x11]= // unknown but changes when other data changes, crc msb?
			//ram[0x12]= // unknown but changes when other data changes, crc lsb?
			if (read(disk_img_fd,ram+PDD2_DATA_REL,SECTOR_DATA_LEN)!=SECTOR_DATA_LEN) { e = ERR_DEFECTIVE; break; }
			//ram[0x0513]= // unknown
			//...          //
			//ram[0x07FF]= // end of 2k ram
			break;

		case CACHE_COMMIT:   // write cache to disk
		case CACHE_COMMIT_VERIFY: // write cache to disk and verify

			// open disk image file and seek to record number
			dbg(2,"cache commit: track:%u  sector:%u\n",t,s);
			if ((e = open_disk_image(rn,O_WRONLY))) break;
			if (write(disk_img_fd,ram+PDD2_ID_REL,SECTOR_HEADER_LEN)!=SECTOR_HEADER_LEN) { e = ERR_DEFECTIVE; break; }
			if (write(disk_img_fd,ram+PDD2_DATA_REL,SECTOR_DATA_LEN)!=SECTOR_DATA_LEN) { e = ERR_DEFECTIVE; break; }
			break;
		default: e = ERR_PARAM;
	}
	close(disk_img_fd);
	dbg_b(3,ram,RAM_LEN);
	if (e) dbg(2,"FAILED\n");
	ret_cache(e);
}

/*
 * req:
 * b[0] fmt req_mem_read
 * b[1] len 4
 *      b[2] area        0=sector_cache 1=cpu_memory
 *      b[3] offset msb  0000-0500      0000-8FFF
 *      b[4] offset lsb
 *      b[5] dlen        00-FC
 * b[6] chk
 *
 * ret:
 * b[0] fmt ret_mem_read
 * b[1] len (dlen+3)
 *      b[2] area        0=sector_cache 1=cpu_memory
 *      b[3] offset msb
 *      b[4] offset lsb
 *      b[5+] data       dlen bytes
 * b[#] chk
 */
void req_mem_read() {
	dbg(3,"%s()\n",__func__);
	if (model==1) return;
	uint8_t a = gb[2];
	uint16_t o = gb[3]*256+gb[4];
	uint8_t l = gb[5];
	uint8_t e = ERR_SUCCESS;
	uint8_t* src = ram; // source of virtual ram data, ram[], rom[], etc
	switch (a) {
		case MEM_CACHE:
			dbg(2,"mem_read: cache  offset:0x%04X  len:0x%02X\n",o,l);
			if (o+l>SECTOR_DATA_LEN || l>PDD2_MEM_READ_MAX) e=ERR_PARAM;
			o+=PDD2_DATA_REL;
			break;
		case MEM_CPU:
			dbg(2,"mem_read: cpu  addr:0x%04X  len:0x%02X\n",o,l);
			if (o>=IOPORT_ADDR && o<IOPORT_ADDR+IOPORT_LEN) { src=ioport; o-=IOPORT_ADDR; break; }
			if (o>=CPURAM_ADDR && o<CPURAM_ADDR+CPURAM_LEN) { src=cpuram; o-=CPURAM_ADDR; break; }
			if (o>=GA_ADDR && o<GA_ADDR+GA_LEN) { src=ga; o-=GA_ADDR; break; }
			if (o>=RAM_ADDR && o<RAM_ADDR+RAM_LEN) { o-=RAM_ADDR; break; }
			if (o>=ROM_ADDR && o<ROM_ADDR+ROM_LEN) { src=rom; o-=ROM_ADDR; break; }
			break;
		default: e=ERR_PARAM;
	}
	if (e) { dbg(1,"mem_read: ERROR: 0x%02X  area:0x%02X  offset:0x%04X  len:0x%02X\n",e,a,o,l); ret_cache(e); return; }

	// copy some data from src[] and return to client
	gb[0] = RET_MEM_READ;
	gb[1] = 3+l;  // len = area(1 byte) + offset(2 bytes) + data(1-252 bytes)
	//gb[2] = gb[2]; // area
	//gb[3] = gb[3]; // offset msb
	//gb[4] = gb[4]; // offset lsb
	memcpy(gb+5,src+o,l); // data
	gb[2+gb[1]] = checksum(gb); // chk
	dbg_b(3,gb,-1);
	write_client_tty(gb,gb[1]+3);
}

/*
 * TPDD2 mem write
 * 
 * b[0] fmt
 * b[1] len
 *      b[2] area  0=sector_cache  1=cpu_memory
 *      b[3] addr msb - address or offset, 2 bytes
 *      b[4] addr lsb
 *      b[5+] data
 * b[#] chk
 */
void req_mem_write() {
	dbg(3,"%s()\n",__func__);
	if (model==1) return;
	uint8_t a = gb[2];
	uint16_t o = gb[3]*256+gb[4];
	uint8_t s = 5; // start of data
	uint8_t l = gb[1]-3; // length of data = length of packet - 3
	uint8_t e = ERR_SUCCESS;
	uint8_t* src = ram; // source of virtual ram data, ram[], rom[], etc
	switch (a) {
		case MEM_CACHE:
			dbg(2,"mem_write: cache  offset:0x%04X  len:0x%02X\n",o,l);
			if (o+l>SECTOR_DATA_LEN || l>PDD2_MEM_WRITE_MAX) e=ERR_PARAM;
			o+=PDD2_DATA_REL;
			break;
		case MEM_CPU:
			dbg(2,"mem_write: cpu  addr:0x%04X  len:0x%02X\n",o,l);
			if (o>=IOPORT_ADDR && o<IOPORT_ADDR+IOPORT_LEN) { src=ioport; o-=IOPORT_ADDR; break; }
			if (o>=CPURAM_ADDR && o<CPURAM_ADDR+CPURAM_LEN) { src=cpuram; o-=CPURAM_ADDR; break; }
			if (o>=GA_ADDR && o<GA_ADDR+GA_LEN) { src=ga; o-=GA_ADDR; break; }
			if (o>=RAM_ADDR && o<RAM_ADDR+RAM_LEN) { o-=RAM_ADDR; break; }
			//if (o>=ROM_ADDR && o<ROM_ADDR+ROM_LEN) { src=rom; o-=ROM_ADDR; break; }
			o-=RAM_ADDR;
			break;
		default: e=ERR_PARAM;
	}
	if (e) { dbg(1,"mem_write: ERROR: 0x%02X  area:0x%02X  offset:0x%04X  len:0x%02X\n",e,a,o,l); ret_cache(e); return; }

	// copy data from client over part of src[]
	memcpy(src+o,gb+s,l);
	dbg_b(3,src+o,l);
	ret_cache(ERR_SUCCESS);
}

/*
 * PDD2 get version
 *
 * Not including the ZZ or checksums:
 * Client sends  : 23 00
 * TPDD2 responds: 14 0F 41 10 01 00 50 05 00 02 00 28 00 E1 00 00 00
 * TPDD1 does not respond.
 *
 * Some versions of TS-DOS use this to detect TPDD2, matching the entire packet,
 * so we have to return this exact canned data if we want TS-DOS to know
 * that it can use TPDD2 features. (not a big deal really)
 *
 */
void ret_version() {
	dbg(3,"%s()\n",__func__);
	if (model==1) return;
	gb[0] = RET_VERSION[0];
	gb[1] = RET_VERSION[1];
	gb[2] = VERSION_MSB;
	gb[3] = VERSION_LSB;
	gb[4] = SIDES;
	gb[5] = TRACKS_MSB;
	gb[6] = TRACKS_LSB;
	gb[7] = SECTOR_SIZE_MSB;
	gb[8] = SECTOR_SIZE_LSB;
	gb[9] = SECTORS_PER_TRACK;
	gb[10] = DIRENTS_MSB;
	gb[11] = DIRENTS_LSB;
	gb[12] = MAX_FD;
	gb[13] = MODEL_CODE;
	gb[14] = VERSION_R0;
	gb[15] = VERSION_R1;
	gb[16] = VERSION_R2;
	gb[17] = checksum(gb);
	write_client_tty(gb,gb[1]+3);
}

/*
 * Similar to ret_version, except different data, and not used by TS-DOS.
 * Real drives also respond to request 0x11 exactly the same as 0x33, though only 0x33 is documented.
 * Not counting ZZ or checksums:
 * Client sends  : 33 00
 * TPDD2 responds: 3A 06 80 13 05 00 10 E1
 */
void ret_sysinfo() {
	dbg(3,"%s()\n",__func__);
	if (model==1) return;
	gb[0] = RET_SYSINFO[0];
	gb[1] = RET_SYSINFO[1];
	gb[2] = SECTOR_CACHE_START_MSB;
	gb[3] = SECTOR_CACHE_START_LSB;
	gb[4] = SECTOR_SIZE_MSB;
	gb[5] = SECTOR_SIZE_LSB;
	gb[6] = SYSINFO_CPU_CODE;
	gb[7] = MODEL_CODE;
	gb[8] = checksum(gb);
	write_client_tty(gb,gb[1]+3);
}

void req_rename() {
	dbg(3,"%s(%-*.*s)\n",__func__,TPDD_FILENAME_LEN,TPDD_FILENAME_LEN,gb+2);
	if (model==1) return;
	char *t = (char *)gb + 2;
	memcpy(t,collapse_padded_fname(t),TPDD_FILENAME_LEN);
	if (rename(cur_file->local_fname,t))
		ret_std(ERR_SECTOR_NUM);
	else {
		dbg(1,"Renamed: %s -> %s\n",cur_file->local_fname,t);
		ret_std(ERR_SUCCESS);
	}
}

void req_close() {
	dbg(2,"%s()\n",__func__);
	if (o_file_h>=0) close(o_file_h);
	o_file_h = -1;
	dbg(2,"Closed: \"%s\"\n",cur_file->local_fname);
	ret_std(ERR_SUCCESS);
}

void req_status() {
	dbg(2,"%s()\n",__func__);
	ret_std(ERR_SUCCESS);
}

// TPDD2 only
// response is 8 bit flags
// 7 unused  MSB
// 6 unused
// 5 unused
// 4 unused
// 3 disk changed
// 2 disk not inserted
// 1 write protected
// 0 low power
void ret_condition() {
	dbg(3,"%s()\n",__func__);
	gb[0] = RET_CONDITION[0];
	gb[1] = RET_CONDITION[1];
	gb[2] = pdd2_condition;
	gb[3] = checksum(gb);
	write_client_tty(gb,gb[1]+3);
}

void req_condition() {
	dbg(2,"%s()\n",__func__);
	if (model!=2) return;
	ret_condition();
}

// opr-format - this creates a disk that can load & save files
// the only difference from fdc-format is a single byte, the first byte of the SMT
// opr-format is just this:
//   start with: fdc-format 0    (0=64-byte logical sector size)
//   then: write 0x80 at sector 0 byte 1240 (aka physical:0 logical:20 byte:25 counting from 1)
void req_format() {
	dbg(2,"%s()\n",__func__);
	const int rc = model==1?(PDD1_TRACKS*PDD1_SECTORS):(PDD2_TRACKS*PDD2_SECTORS); // records count
	int rn = 0;          // record number

	dbg(0,"Operation-mode Format (make a filesystem)\n");

	uint8_t e = open_disk_image(0,O_WRONLY);
	if (e==ERR_READ_TIMEOUT) e=ERR_FMT_INTERRUPT;
	if (e) { ret_std(e); return; }

	// write the image
	// Real drive TPDD1 fresh Operation-mode format is strange.
	// Any sector with any data gets LSC 0, and all others get LSC 1.
	// Later, any sector that gets used by a file gets changed from LSC 1 to
	// LSC 0, and never changed back even when files are deleted.
	// A fresh format has one byte of data in sector 0 in the SMT,
	// so a fresh format sector 0 has LSC 0 and all other sectors have LSC 1.
	// We exactly mimick that here "just because", even though the LSC 1s
	// don't seem to actually matter and we could just make all LSC 0.
	for (rn=0;rn<rc;rn++) {
		memset(rb,0x00,SECTOR_LEN);
		switch (model) {
			case 1: if (rn==0) rb[SECTOR_HEADER_LEN+SMT_OFFSET]=PDD1_SMT; else rb[0]=1; break;
			default: rb[0]=0x16; if (rn<2) { rb[1]=0xFF; rb[SECTOR_HEADER_LEN+SMT_OFFSET]=PDD2_SMT; }
		}
		if (write(disk_img_fd,rb,SECTOR_LEN)<0) break;
	}

	if (rn<rc) {
		dbg(0,"%s\n",strerror(errno));
		e = ERR_FMT_INTERRUPT;
	}

	close(disk_img_fd);
	ret_std(e);
}

/*
 * req_exec() - execute program
 *
 * TPDD2 only
 *
 * Just a stub. Not likely to impliment any time soon,
 * but might as well put the stub in to document it.
 *
 * TPDD2 util disk bootstrap uses this
 */

/* response from req_exec()
 * returns the execution results from the cpu reisters A and X
 * b[0] fmt (0x3B)
 * b[1] len (0x03)
 *      b[2] reg A - 1 byte
 *      b[3] reg X msb - 2 bytes
 *      b[4] reg X lsb
 * b[5] chk
*/
void ret_exec(uint8_t reg_A, uint16_t reg_X) {
	dbg(3,"%s(%u,%u)\n",__func__,reg_A,reg_X);
	gb[0] = RET_EXEC[0];
	gb[1] = RET_EXEC[1];
	gb[2] = reg_A;
	gb[3] = (uint8_t)(reg_X >> 0x08); // msb
	gb[4] = (uint8_t)(reg_X & 0xFF);  // lsb
	gb[5] = checksum(gb);
	write_client_tty(gb,6);
}

/* Load cpu registers A and X with supplied values, then jump to supplied address.
 *
 * examples:
 * - jump to a rom routine
 * - req_cache() to load a sector from disk first, then jump to the sector cache
 * - req_mem_write() to write arbitrary code to cpu memory first, then jump to it
 *
 * b[0] fmt (0x34)
 * b[1] len (0x05)
 *      b[2] addr msb - execute address 2 bytes
 *      b[3] addr lsb
 *      b[4] reg A - 1 byte
 *      b[5] reg X msb - 2 bytes
 *      b[6] reg X lsb
 * b[7] chk
 */
void req_exec() {
	dbg(3,"%s() ***STUB***\n",__func__);
	if (model==1) return;
	uint16_t addr = gb[2]*256+gb[3];
	uint8_t reg_A = gb[4];
	uint16_t reg_X = gb[5]*256+gb[6];
	dbg(2,"exec:  addr:%u  A:%u  X:%u\n",addr,reg_A,reg_X);
	/*
	 * ...6301 emulator here...
	 * executed code leaves new values in reg_A and reg_X
	 */
	dbg(2,"(stub, exec() not implimented)");
	ret_exec(reg_A,reg_X);
}

void get_opr_cmd() {
	dbg(3,"%s()\n",__func__);
	uint16_t i = 0;
	memset(gb,0x00,TPDD_MSG_MAX);

	while (read_client_tty(&gb,1) == 1) {
		if (gb[0]==OPR_CMD_SYNC) i++; else { i=0; gb[0]=0x00; continue; }
		if (i<2) { gb[0]=0x00; continue; }
		if (read_client_tty(&gb,2) == 2) if (read_client_tty(&gb[2],gb[1]+1) == gb[1]+1) break;
		i=0; memset(gb,0x00,TPDD_MSG_MAX);
	}

	dbg_p(3,gb);

	if ((i=checksum(gb))!=gb[gb[1]+2]) {
		dbg(0,"Failed checksum: received: 0x%02X  calculated: 0x%02X\n",gb[gb[1]+2],i);
		return; // real drive does not return anything
	}

	uint8_t c = gb[0];

	// decode bit 6 in the FMT byte b[0] for bank0 vs bank1
	if (model==2) {
		//bank = 0; if (c&0x40) { bank = 1; c-=0x40; } // alternative method
		bank = (c >> 6) & 1; // read bit 6
		c &= ~(1 << 6);      // clear bit 6
	}

	// TODO double-check this
	// another note says sysinfo is 0x33 but the drive also responds to 0x11
	// so maybe this is backwards? maybe it's both?
	// For FMT 0x30-0x34 pdd2 accepts FMT or FMT+0x22. Unknown function.
	// For now just treat them as synonyms by subtracting 0x22 if present.
	//if ( c>0x51 && c<0x57 ) c-=0x22;

	// dispatch
	switch(c) {
		case REQ_DIRENT:        req_dirent();        break;
		case REQ_OPEN:          req_open();          break;
		case REQ_CLOSE:         req_close();         break;
		case REQ_READ:          req_read();          break;
		case REQ_WRITE:         req_write();         break;
		case REQ_DELETE:        req_delete();        break;
		case REQ_FORMAT:        req_format();        break;
		case REQ_STATUS:        req_status();        break;
		case REQ_FDC:           req_fdc();           break;
		case REQ_CONDITION:     req_condition();     break;
		case REQ_RENAME:        req_rename();        break;
		case REQ_VERSION:       ret_version();       break;
		case REQ_CACHE:         req_cache();         break;
		case REQ_MEM_READ:      req_mem_read();      break;
		case REQ_MEM_WRITE:     req_mem_write();     break;
		case REQ_SYSINFO:       ret_sysinfo();       break;
		case REQ_EXEC:          req_exec();          break;
		default: dbg(1,"OPR: unknown cmd \"0x%02X\"\n",gb[0]); dbg_p(1,gb);
		// local msg, nothing to client
	}
}

////////////////////////////////////////////////////////////////////////
//
//  BOOTSTRAP
//

void show_bootstrap_help() {
	dbg(0,"Available support files in %s\n\n",app_lib_dir);

	dbg(0,"Loader files for use with -b:\n"
	      "-----------------------------\n");
	dbg(0,  "TRS-80 Model 100/102 :"); lsx(app_lib_dir,"100"," %s");
	dbg(0,"\nTANDY Model 200      :"); lsx(app_lib_dir,"200"," %s");
	dbg(0,"\nNEC PC-8201/PC-8300  :"); lsx(app_lib_dir,"NEC"," %s");
	dbg(0,"\nKyotronic KC-85      :"); lsx(app_lib_dir,"K85"," %s");
	dbg(0,"\nOlivetti M-10        :"); lsx(app_lib_dir,"M10"," %s");

	dbg(0,"\n\nDisk image files for use with -i:\n"
	          "---------------------------------\n");
	lsx(app_lib_dir,"pdd1","%s\n");
	dbg(0,"\n");
	lsx(app_lib_dir,"pdd2","%s\n");

	dbg(0,
		"\n"
		"Filenames given without any path are searched from %2$s\n"
		"as well as the current directory.\n"
		"Examples:\n\n"
		"   %1$s -b TS-DOS.100\n"
		"   %1$s -b ~/Documents/LivingM100SIG/Lib-03-TELCOM/XMDPW5.100\n"
		"   %1$s -vb rxcini.DO && %1$s -vu\n"
		"   %1$s -vun -i Sardine_American_English.pdd1\n\n"
	,args[0],app_lib_dir);
}

void slowbyte(uint8_t b) {
	write_client_tty(&b,1);
	tcdrain(client_tty_fd);
	usleep(BASIC_byte_us);

	// line-endings - convert CR, LF, CRLF to local eol
	if (ch[0]==BASIC_EOL) {
		 ch[0]=0x00;
		 dbg(0,"%c",LOCAL_EOL);
		 if (b==LOCAL_EOL) return;
	}
	if (b==BASIC_EOL) { ch[0]=BASIC_EOL; return; }

#if defined(PRINT_8BIT)
	// display <32 and 127 as inverse ctrl char without ^
	// print everything else, requires disable 8-bit vt codes
	if (b<32) { dbg(0,SSO"%c"RSO,b+64); return; }
	if (b==127) { dbg(0,SSO"?"RSO); return; }
#else
	// display <32 >126 as inverse hex
	if (b<32||b>126) { dbg(0,SSO"%02X"RSO,b); return; }
#endif

	dbg(0,"%c",b);
}

int send_BASIC(char* f) {
	int fd;
	uint8_t b;

	if ((fd=open(f,O_RDONLY))<0) {
		dbg(0,"Could not open \"%s\" : %s\n",f,errno);
		return 9;
	}

#if defined(PRINT_8BIT)
	dbg(1,D8C); // disable 8-bit vt codes (0x80-0x9F) so we can print them
#endif
	dbg(0,"-- start --\n");
	ch[0]=0x00;
	while(read(fd,&b,1)==1) slowbyte(b);
	close(fd);
	if (dot_offset) { // if not in raw mode supply missing trailing EOF & EOL
		if (b!=LOCAL_EOL && b!=BASIC_EOL && b!=BASIC_EOF) slowbyte(BASIC_EOL);
		if (b!=BASIC_EOF) slowbyte(BASIC_EOF);
	}
	close(client_tty_fd);
	dbg(0,"\n-- end --\n\n");
	return 0;
}

int bootstrap(char* f) {
	dbg(0,"Bootstrap: Installing \"%s\"\n\n",f);
	if (access(f,F_OK)==-1) {
		dbg(0,"Not found.\n");
		return 1;
	}

	char t[PATH_MAX+1]={0x00};
	int sc = baud_to_stat_code(baud);
	if (!sc) {
		dbg(0,"Prepare the client to receive data."
		"\n"
		"Note: The current baud setting, %d, is not supported\n"
		"by the TRS-80 Model 100 or other KC-85-platform machines.\n"
		"There is no way for BASIC or TELCOM to use this baud rate.\n",baud);
	} else {
		strcpy(t,f);
		strcat(t,".pre-install.txt");
		if (!access(t,F_OK) && sc==9) dcat(t); // the text files all assume 19200
		else {
			dbg(0,"Prepare BASIC to receive:\n"
			"\n"
			"    RUN \"COM:%1$d8N1ENN\" [Enter]    <-- TANDY/Olivetti/Kyotronic\n"
			"    RUN \"COM:%1$dN81XN\"  [Enter]    <-- NEC\n",sc);
		}
	}

	dbg(0,"\nPress [Enter] when ready...");
	getchar();

	{ int r; if ((r=send_BASIC(f))!=0) return r; }

	strcpy(t,f);
	strcat(t,".post-install.txt");
	dcat(t);

	dbg(0,"\n\n\"%1$s -b\" will now exit.\n"
	      "Re-run \"%1$s\" (without -b this time) to run the TPDD server.\n\n",args[0]);

	return 0;
}

////////////////////////////////////////////////////////////////////////
//
//  MAIN
//

void show_config () {
#if !defined(_WIN)
	dbg(0,"getty_mode      : %s\n",getty_mode?"true":"false");
#endif
	dbg(0,"upcase          : %s\n",upcase?"true":"false");
	dbg(0,"rtscts          : %s\n",rtscts?"true":"false");
	dbg(0,"verbosity       : %d\n",debug);
	dbg(0,"model           : %d\n",model);
	dbg(0,"dot_offset      : %d\n",dot_offset);
	dbg(0,"BASIC_byte_ms   : %d\n",BASIC_byte_us/1000);
	dbg(0,"bootstrap_fname : \"%s\"\n",bootstrap_fname);
	dbg(0,"app_lib_dir     : \"%s\"\n",app_lib_dir);
	dbg(0,"client_tty_name : \"%s\"\n",client_tty_name);
	dbg(0,"disk_img_fname  : \"%s\"\n",disk_img_fname);
	dbg(0,"cwd             : \"%s\"\n",cwd);
	dbg(0,"share_path[0]   : \"%s\"\n",share_path[0]);
	dbg(0,"share_path[1]   : \"%s\"\n",share_path[1]);
	dbg(2,"opr_mode        : %d\n",opr_mode);
	dbg(2,"baud            : %d\n",baud);
	dbg(0,"dme_disabled    : %s\n",dme_disabled?"true":"false");
	dbg(2,"dme_root_label  : \"%-6.6s\"\n",dme_root_label);
	dbg(2,"dme_parent_label: \"%-6.6s\"\n",dme_parent_label);
	dbg(2,"dme_dir_label   : \"%-2.2s\"\n",dme_dir_label);
	dbg(0,"magic_files     : %s\n",enable_magic_files?"enabled":"disabled");
	dbg(2,"default_attr    : '%c'\n",default_attr);
}

void show_main_help() {
	dbg(0,"\nUsage: %1$s [options] [tty_device] [share_path]\n"
		"\n"
		"Options     Description (default setting)\n"
		"   -0       Raw mode - no filename munging, attr = ' '\n"
		"   -a c     Attribute - attribute byte used for all files (%2$c)\n"
		"   -b file  Bootstrap - send loader file to client\n"
		"   -d tty   Serial device connected to the client (%4$s*)\n"
		"   -n       Disable TS-DOS directories (enabled)\n"
#if !defined(_WIN)
		"   -g       Getty mode - run as daemon\n"
#endif
		"   -h       Print this help\n"
		"   -i file  Disk image filename for raw sector access\n"
		"   -l       List loader files and show bootstrap help\n"
		"   -m #     Model - 1 = FB-100/TPDD1, 2 = TPDD2 (%5$u)\n"
		"   -p dir   Path - /path/to/dir with files to be served (./)\n"
		"   -r       RTS/CTS hardware flow control (%7$s)\n"
		"   -s #     Speed - serial port baud rate (%6$d)\n"
		"   -u       Uppercase all filenames (%8$s)\n"
		"   -v       Verbosity - more v's = more verbose\n"
		"   -w       WP-2 mode - 8.2 filenames for TANDY WP-2\n"
		"   -z #     Milliseconds per byte for bootstrap (%3$d)\n"
		"\n"
		"The 1st non-option argument is another way to specify the tty device.\n"
		"The 2nd non-option argument is another way to specify the share path.\n"
		"TPDD2 mode accepts a 2nd share path for bank 1.\n"
		"TPDD2 mode does not support TS-DOS dfirectories.\n"
		"\n"
		"Examples:\n"
		"   $ %1$s\n"
		"   $ %1$s ttyUSB1\n"
		"   $ %1$s -vu -p ~/Downloads/REX\n"
		"   $ %1$s -w /dev/cu.usbserial-AB0MQNN1 ~/Documents/wp2\n"
		"   $ %1$s -m2 -p /tmp/bank0 -p /tmp/bank1\n"
		"\n"
		,args[0]
		,DEFAULT_ATTR
		,DEFAULT_BASIC_BYTE_MS
		,TTY_PREFIX
		,DEFAULT_MODEL
		,DEFAULT_BAUD
		,DEFAULT_RTSCTS?"true":"false"
		,DEFAULT_UPCASE?"true":"false"
	);
}

int main(int argc, char** argv) {
	dbg(0,APP_NAME " " APP_VERSION "\n");

	int i;
	bool x = false;
	args = argv;
	(void)!getcwd(iwd,PATH_MAX); // remember initial working directory

	// environment
	if (getenv("OPR_MODE")) opr_mode = atoi(getenv("OPR_MODE"));
	if (getenv("DISABLE_DME")) dme_disabled = true;
	if (getenv("DISABLE_MAGIC_FILES")) enable_magic_files = false;
	if (getenv("DOT_OFFSET")) dot_offset = atoi(getenv("DOT_OFFSET"));
	if (getenv("CLIENT_TTY")) strcpy(client_tty_name,getenv("CLIENT_TTY"));
	if (getenv("BAUD")) baud = atoi(getenv("BAUD"));
	if (getenv("ROOT_LABEL")) snprintf(dme_root_label,7,"%-6.6s",getenv("ROOT_LABEL"));
	if (getenv("PARENT_LABEL")) snprintf(dme_parent_label,7,"%-6.6s",getenv("PARENT_LABEL"));
	if (getenv("DIR_LABEL")) snprintf(dme_dir_label,3,"%-2.2s",getenv("DIR_LABEL"));
	if (getenv("ATTR")) default_attr = *getenv("ATTR");

	// commandline
#if defined(_WIN)
	while ((i = getopt (argc, argv, ":0a:b:d:nhi:lm:p:rs:uvwz:^")) >=0)
#else
	while ((i = getopt (argc, argv, ":0a:b:d:nghi:lm:p:rs:uvwz:^")) >=0)
#endif
		switch (i) {
			case '0': dot_offset=0; upcase=false; default_attr=RAW_ATTR;  break;
			case 'a': default_attr=*strndup(optarg,1);                    break;
			case 'b': strcpy(bootstrap_fname,optarg);                     break;
			case 'd': strcpy(client_tty_name,optarg);                     break;
			case 'n': dme_disabled = true;                                break;
#if !defined(_WIN)
			case 'g': getty_mode = true; debug = 0;                       break;
#endif
			case 'h': show_main_help(); exit(0);                          break;
			case 'i': strcpy(disk_img_fname,optarg);                      break;
			case 'l': show_bootstrap_help(); exit(0);                     break;
			case 'm': model = atoi(optarg);                               break;
			//case 'p': (void)!chdir(optarg);                               break;
			case 'p': add_share_path(optarg);                             break;
			case 'r': rtscts = true;                                      break;
			case 's': baud = atoi(optarg);                                break;
			case 'u': upcase = true;                                      break;
			case 'v': debug++;                                            break;
			case 'w': dot_offset = 8;                                     break;
			case 'z': BASIC_byte_us=atoi(optarg)*1000;                    break;
			case '^': x = true;                                           break;
			case ':': dbg(0,"\"-%c\" requires a value\n",optopt);         break;
			case '?':
				if (isprint(optopt)) dbg(0,"Unknown option \"-%c\"\n",optopt);
				else dbg(0,"Unknown option character \"0x%02X\"\n",optopt);
			default: show_main_help();                                 return 1;
		}

	// commandline non-option arguments
	for (i=0; optind < argc; optind++) {
		if (x) dbg(1,"non-option arg %u: \"%s\"\n",i,argv[optind]);
		switch (i++) {
			case 0: strcpy (client_tty_name,argv[optind]); break; // tty device
			//case 1: (void)!chdir(argv[optind]); break; // share path
			case 1:
			case 2: add_share_path(argv[optind]); break; // share path(s)
			default: dbg(0,"Unknown argument: \"%s\"\n",argv[optind]);
		}
	}

	// base setup that's always needed, whether tpdd or bootstrap
	if (model<1||model>2) {dbg(0,"Invalid model \"%u\"\n",model); return 1; }
	if (share_path[0][0]) cd_share_path();
	if (!cwd[0]) update_cwd();
	if (!share_path[0][0]) strcpy(share_path[0],cwd);
	resolve_client_tty_name();
	find_lib_file(bootstrap_fname);

	if (x) { show_config(); return 0; }

	dbg(0,    "Serial Device: %s\n",client_tty_name);
	if (model==2) {
		dbg(0,"Bank 0 Dir   : %s\n",share_path[0]);
		dbg(0,"Bank 1 Dir   : %s\n",share_path[1]);
	} else {
		dbg(0,"Working Dir  : %s\n",cwd);
	}

	if ((i=open_client_tty())) return i;

	// send loader and exit
	if (bootstrap_fname[0]) return (bootstrap(bootstrap_fname));

	// further setup that's only needed for tpdd
	if (check_disk_image()) return 1; // may change model based on disk image size or name
	dbg(2,"Emulating %s\n",(model==2)?"TANDY 26-3814 (TPDD2)":"Brother FB-100 (TPDD1)");
	if (model==2) {load_rom(TPDD2_ROM); dme_disabled=true; }
	dbg(2,"TS-DOS directories %s\n",(dme_disabled)?"disabled":"enabled");
	dbg(2,"TPDD2 banks %s\n",(model==2)?"enabled":"disabled");
	if (!dme_disabled) memcpy(dme_cwd,dme_root_label,6);
	if (dot_offset!=6) enable_magic_files=false; // only applies to UR2/TSLOAD
	dbg(2,"Magic files for UR-II/TSLOAD %s\n",(enable_magic_files)?"enabled":"disabled");

	// initialize the file list
	file_list_init();
	if (debug) update_file_list(NO_RET);

	// process commands forever
	while (1) if (opr_mode) get_opr_cmd(); else get_fdc_cmd();

	// file_list_cleanup()
	return 0;
}
