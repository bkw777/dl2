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
2022                      disk image files and sector-access
2020     Kurt McCullum - TS-DOS loaders
2022     Gabriele Gorla - TS-DOS subdirectories

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

#ifndef APP_LIB_DIR
#define APP_LIB_DIR "."
#endif

#ifndef DEFAULT_CLIENT_TTY
#define DEFAULT_CLIENT_TTY "ttyS0"
#endif

#ifndef DEFAULT_CLIENT_BAUD
#define DEFAULT_CLIENT_BAUD B19200
#endif

// Most things get away with 5ms.
// REXCPM rxcini.do requires 6ms.
// TS-DOS.200 requires 7ms. (a "?" on line 3 gets dropped)
// TEENY.M10 requires 8-10
#define DEFAULT_BASIC_BYTE_MS 8

#define DEFAULT_TPDD_FILE_ATTR 0x46 // F

// To mimic the original DeskLink from Travelling Software:
//#define DEFAULT_DME_ROOT_LABEL   "ROOT  "
//#define DEFAULT_DME_PARENT_LABEL "PARENT" // environment variables:
#define DEFAULT_DME_ROOT_LABEL   "0:    "   // ROOT_LABEL='0:'   '-root-' 'C:\'
#define DEFAULT_DME_PARENT_LABEL "^     "   // PARENT_LABEL='^:' '-back-' 'UP:'
// this you can't change unless you also hack ts-dos
#define DEFAULT_DME_DIR_LABEL    "<>"       // DIR_LABEL='/'

/*
 * Support for Ultimate ROM II TS-DOS loader: see ref/ur2.txt
 * These filenames will always be loadable by "magic" in any cd path, even
 * if no such filename exists anywhere in the share tree. For any of these
 * filenames, search the following paths: cwd, share root, app_lib_dir.
 * TODO add $XDG_DATA_HOME (~/.local/share/myapp  mac: ~/Library/myapp/)
 */
char * magic_files [] = {
	"DOS100.CO",
	"DOS200.CO",
	"DOSNEC.CO",
	"SAR100.CO",
	"SAR200.CO",
	"SARNEC.CO", // This is known to have existed, but is currently lost.
	"DOSM10.CO", // The rest may have never existed,
	"DOSK85.CO", // and the filenames are just guesses.
	"SARM10.CO", //
	"SARK85.CO"  //
};

// termios VMIN & VTIME
#define C_CC_VMIN 1
#define C_CC_VTIME 5

/*************************************************************/

int debug = 0;
bool upcase = false;
bool rtscts = false;
unsigned dot_offset = 6; // 0 for raw, 6 for KC-85, 8 for WP-2
int client_baud = DEFAULT_CLIENT_BAUD;
int BASIC_byte_us = DEFAULT_BASIC_BYTE_MS*1000;
char client_tty_name[PATH_MAX] = DEFAULT_CLIENT_TTY;
char disk_img_fname[PATH_MAX] = {0x00};
char app_lib_dir[PATH_MAX] = APP_LIB_DIR;
char dme_root_label[7] = DEFAULT_DME_ROOT_LABEL;
char dme_parent_label[7] = DEFAULT_DME_PARENT_LABEL;
char dme_dir_label[3] = DEFAULT_DME_DIR_LABEL;
char default_attr = DEFAULT_TPDD_FILE_ATTR;
bool enable_magic_files = true;
bool getty_mode = false;
bool bootstrap_mode = false;
int model = 2;

char **args;
int f_open_mode = F_OPEN_NONE;
int client_tty_fd = -1;
int disk_img_fd = -1;
struct termios client_termios;
int o_file_h = -1;
unsigned char gb[TPDD_DATA_MAX];
char cwd[PATH_MAX] = {0x00};
char dme_cwd[7] = DEFAULT_DME_ROOT_LABEL;
char bootstrap_fname[PATH_MAX] = {0x00};
int opr_mode = 1;
bool dme_detected = false;
bool dme_fdc = false;
bool dme_disabled = false;
char ch[2] = {0xFF};
uint8_t mlen = PDD2_SECTOR_META_LEN;
const uint16_t dlen = SECTOR_DATA_LEN;
const int fdc_logical_size_codes[] = FDC_LOGICAL_SIZE_CODES;
const char fdc_cmds[] = FDC_CMDS;
unsigned char * rb = 0x00;

FILE_ENTRY *cur_file;
int dir_depth=0;

void show_main_help();

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
// if len<0, then assume the max tpdd buffer TPDD_DATA_MAX
void dbg_b(const int v, unsigned char *b, int n) {
	if (debug<v) return;
	unsigned i;
	if (n<0) n = TPDD_DATA_MAX;
	for (i=0;i<n;i++) fprintf (stderr,"%02X ",b[i]);
	fprintf (stderr, "\n");
	fflush(stderr);
}

// like dbg_b, except assume the buffer is a tpdd Operation-mode
// block and parse it to display cmd, len, payload, checksum.
void dbg_p(const int v, unsigned char *b) {
	dbg(v,"cmd: %1$02X\nlen: %2$02X (%2$u)\nchk: %3$02X\ndat: ",b[0],b[1],b[b[1]+2]);
	dbg_b(v,b+2,b[1]);
}

// Drive supposedly supports 76800, but on Linux (except Sparc) that would
// require termios2() and BOTHER. https://stackoverflow.com/a/39924923/5754855
// no idea about bsd or mac
/*
	struct termios2 t;
	ioctl(fd, TCGETS2, &t); // Read current settings
	t.c_cflag &= ~CBAUD;    // Remove current baud rate
	t.c_cflag |= BOTHER;    // Allow custom baud rate using int input
	t.c_ispeed = 76800;     // Set the input baud rate
	t.c_ospeed = 76800;     // Set the output baud rate
	ioctl(fd, TCSETS2, &t); // Apply new settings
*/
// clients really only ever use 9600 or 19200, but the
// real drive has dip switches for all these speeds so...
void set_baud (char * s) {
	int i=atoi(s);
	client_baud=
		i==150?B150:
		i==300?B300:
		i==600?B600:
		i==1200?B1200:
		i==2400?B2400:
		i==4800?B4800:
		i==9600?B9600:     // Brother FB-100, KnitKing FDD19, Purple Computing D103 default
		//i==19200?B19200: // TPDD1 & TPDD2 default
		i==38400?B38400:
#if defined(__sparc__)
		i==76800?B76800:
#endif
		B19200;
}

int get_baud () {
	return
		client_baud==B150?150:
		client_baud==B300?300:
		client_baud==B600?600:
		client_baud==B1200?1200:
		client_baud==B2400?2400:
		client_baud==B4800?4800:
		client_baud==B9600?9600:
		client_baud==B19200?19200:
		client_baud==B38400?38400:
#if defined(__sparc__)
		client_baud==B76800?76800:
#endif
		0;
}

void find_lib_file (char *f) {
	if (f[0]==0x00) return;

	char t[PATH_MAX]={0x00};

	if (f[0]=='~' && f[1]=='/') {
		strcpy(t,f);
		memset(f,0x00,PATH_MAX);
		strcpy(f,getenv("HOME"));
		strcat(f,t+1);
	}

	if (f[0]!='/' && f[0]!='.' && f[1]!='/' && access(f,F_OK)) {
		memset(t,0x00,PATH_MAX);
		strcpy(t,app_lib_dir);
		strcat(t,"/");
		strcat(t,f);
		if (!access(t,F_OK)) {
			memset(f,0x00,PATH_MAX);
			strcpy(f,t);
		}
	}

	dbg(0,"Loading: \"%s\"\n",f);
}

void resolve_client_tty_name () {
	dbg(3,"%s()\n",__func__);
	switch (client_tty_name[0]) {
		case 0x00: break;
		case '-':
			debug = 0;
			strcpy (client_tty_name,"/dev/tty");
			client_tty_fd=1;
			break;
		default:
			if (!access(client_tty_name,F_OK)) break;
			char t[PATH_MAX]={0x00};
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

	if (!strcmp(client_tty_name,"")) { show_main_help() ;dbg(0,"Error: No serial device specified\n"); return 1; }

	dbg(0,"Opening \"%s\" ... ",client_tty_name);
	// open with O_NONBLOCK to avoid hang from client not ready, then unset later.
	if (client_tty_fd<0) client_tty_fd=open((char *)client_tty_name,O_RDWR|O_NOCTTY|O_NONBLOCK);
	if (client_tty_fd<0) { dbg(0,"%s\n",strerror(errno)); return 1; }
	dbg(0,"OK\n");

	ioctl(client_tty_fd,TIOCEXCL);

	if (getty_mode) {
		debug = 0;
		if (login_tty(client_tty_fd)==0) client_tty_fd = STDIN_FILENO;
		else (void)(daemon(1,1)+1);
	}

	(void)(tcflush(client_tty_fd, TCIOFLUSH)+1);

	// unset O_NONBLOCK
	fcntl(client_tty_fd, F_SETFL, fcntl(client_tty_fd, F_GETFL, NULL) & ~O_NONBLOCK);

	if (tcgetattr(client_tty_fd,&client_termios)==-1) return 21;

	cfmakeraw(&client_termios);
	client_termios.c_cflag |= CLOCAL|CS8;

	if (rtscts) client_termios.c_cflag |= CRTSCTS;
	else client_termios.c_cflag &= ~CRTSCTS;

	if (cfsetspeed(&client_termios,client_baud)==-1) return 22;

	if (tcsetattr(client_tty_fd,TCSANOW,&client_termios)==-1) return 23;

	client_tty_vmt(-2,-2);

	return 0;
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
void dcat(char *f) {
	char b[4097]={0x00}; int h=open(f,O_RDONLY);
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
unsigned char checksum(unsigned char *b) {
	unsigned short s=0; unsigned char i; unsigned char l=2+b[1];
	for (i=0;i<l;i++) s+=b[i];
	return ~(s&0xFF);
}

char *collapse_padded_fname(char *fname) {
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
		if (!strcmp(files->d_name+i+1,match)) dbg(0," %s",files->d_name);
	}
	closedir(dir);
}

int check_magic_file(char *b) {
	dbg(3,"%s(\"%s\")\n",__func__,b);
	if (!enable_magic_files) return 1;
	if (dot_offset!=6) return 1; // UR2 only exists on the KC-85 clones
	int l = sizeof(magic_files)/sizeof(magic_files[0]);
	for (int i=0;i<l;++i) if (!strcmp(magic_files[i],b)) return 0;
	return 1;
}

////////////////////////////////////////////////////////////////////////
//
//  FDC MODE
//

/*
sectors: 0-79
sector: 1293 bytes
| ID 13 bytes | DATA 1280 bytes |
---
1    logical sector length code
12   data
---
1280 data
---
*/

// return the length in bytes for a given logical size code
int lsc_to_len(int l) {
	if (l<0||l>6) l=3;
	return fdc_logical_size_codes[l];
}

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

/*
int seek_disk_image (int p, int l, int r) {
	int s = (p*(mlen+dlen));
	if (l) s+=mlen+l*llen-l;
	return lseek(disk_img_fd,s,SEEK_SET);
}
*/

// p   : physical sector to seek to
// m   : read-only / write-only / read-write
// r   : send or don't send error response to client from here
int open_disk_image (int p, int m, int r) {
	dbg(2,"%s(%d,%d,%d)\n",__func__,p,m,r);

	if (!strcmp(disk_img_fname,"")) return ERR_FDC_NO_DISK;
	int of; int e=ERR_FDC_SUCCESS;

	switch (m) {
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
		int s = (p*(mlen+dlen)); // initial seek position to start of physical sector
		if (lseek(disk_img_fd,s,SEEK_SET)!=s) e=ERR_FDC_READ;
	}

	if (r && e) ret_fdc_std(e,0,0);
	return e;
}

void req_fdc_set_mode(int m) {
	dbg(2,"%s(%d)\n",__func__,m);
	dbg(1,"FDC: Switching to \"%s\" mode\n",m==0?"FDC":m==1?"Operation":"-invalid-");
	opr_mode=m; // no response, just switch modes
}

// disk not-ready conditions
// ret_fdc_std(e,s,l)
// e = ERR_FDC_SUCCESS
// s = bit flags:
//   7: 1 = disk not inserted     FDC_COND_NOTINS
//   6: 1 = disk changed          FDC_COND_CHANGED
//   5: 1 = disk write-protected  FDC_COND_WPROT
// l = 0
// examples
// ret_fdc_std(ERR_FDC_SUCCESS,FDC_COND_WPROT,0)
// ret_fdc_std(ERR_FDC_SUCCESS,FDC_COND_NOTINS|FDC_COND_CHANGED,0)
void req_fdc_condition() {
	dbg(2,"%s()\n",__func__);
	int s=FDC_COND_NONE;
	if (access(disk_img_fname,F_OK)) s=FDC_COND_NOTINS;
	else if (access(disk_img_fname,W_OK)) s=FDC_COND_WPROT;
	ret_fdc_std(ERR_FDC_SUCCESS,s,0);
}

// lc = logical sector size code
void req_fdc_format(int lc) {
	dbg(2,"%s(%d)\n",__func__,lc);
	int ll = lsc_to_len(lc);
	int rn = 0;     // physical sector number
	int rl = mlen+dlen; // total length
	int rc = (PDD1_TRACKS*PDD1_SECTORS);

	dbg(0,"Format: Logical sector size: %d = %d\n",lc,ll);

	if (open_disk_image(0,O_RDWR,ALLOW_RET)) return;

	memset(rb,0x00,rl);
	rb[0]=lc;            // logical sector size code
	for (rn=0;rn<rc;rn++) {
		if (write(disk_img_fd,rb,rl)<0) {
			dbg(0,"%s\n",strerror(errno));
			(void)(close(disk_img_fd)+1);
			ret_fdc_std(ERR_FDC_READ,rn,0);
			return;
		}
	}

	(void)(close(disk_img_fd)+1);
	ret_fdc_std(ERR_FDC_SUCCESS,0,0);
}

// p = physical sector number
void req_fdc_read_id(int p) {
	dbg(2,"%s(%d)\n",__func__,p);
	if (open_disk_image(p,O_RDONLY,ALLOW_RET)) return; // open and seek
	int r = read(disk_img_fd,rb,mlen);  // read ID section
	dbg_b(2,rb,mlen);
	int l = lsc_to_len(rb[0]);          // get logical size from header
	ret_fdc_std(ERR_FDC_SUCCESS,p,l);   // send OK
	char t=0x00; read_client_tty(&t,1); // read 1 byte from client
	if (t!=FDC_CMD_EOL) return; // if it's anything but CR, silently abort
	write_client_tty(rb+1,r-1); // send data
	(void)(close(disk_img_fd)+1);
}

// tp = target physical sector
// tl = target logical sector
void req_fdc_read_sector(int tp,int tl) {
	dbg(2,"%s(%d,%d)\n",__func__,tp,tl);
	if (open_disk_image(tp,O_RDONLY,ALLOW_RET)) return; // open & seek to tp
	if (read(disk_img_fd,rb,mlen)!=mlen) { // read ID section
		dbg(1,"failed read ID\n");
		(void)(close(disk_img_fd)+1);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}
	dbg_b(3,rb,mlen);

	int l = lsc_to_len(rb[0]); // get logical size from header

	// seek to target_physical*(id_len+physical_len) + id_len + (target_logical-1)*logical_len
	int s = (tp*(mlen+dlen))+mlen+((tl-1)*l);
	if (lseek(disk_img_fd,s,SEEK_SET)!=s) {
		dbg(1,"failed seek %d : %s\n",s,strerror(errno));
		(void)(close(disk_img_fd)+1);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}
	memset(rb,0x00,l);
	if (read(disk_img_fd,rb,l)!=l) { // read one logical sector of DATA
		dbg(1,"failed logical sector read\n");
		(void)(close(disk_img_fd)+1);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}
	ret_fdc_std(ERR_FDC_SUCCESS,tp,l); // 1st stage response
	char t=0x00;
	read_client_tty(&t,1);  // read 1 byte from client
	if (t!=0x0D) return;    // if it's anything but CR, silently abort
	write_client_tty(rb,l); // send data
	(void)(close(disk_img_fd)+1);
}

void req_fdc_search_id() {
	dbg(2,"%s()\n",__func__);
	// not sure how this is supposed to work yet, this is a guess:
	//   send OK to client
	//   read 12 bytes from client
	//   search all ID's for match
	//   return, what? Send another OK?
	//   manual says it's the same as write_sector, which
	//   sends an OK to tell client to send, and then
	//   another OK to ack. So perhaps we return
	//   just a status return that indicates if a match was found
	//   and probably the first matching sector number in the len/addr field.
	//   Probably the err field is always success.
	ret_fdc_std(ERR_FDC_SUCCESS,0,0);
}

void req_fdc_write_id(uint16_t tp) {
	dbg(2,"%s(%d)\n",__func__,tp);
	int s = (tp*(mlen+dlen));
	if (open_disk_image(s,O_RDWR,ALLOW_RET)) return; // we need both read & write
	uint8_t r = read(disk_img_fd,rb,mlen); // read ID
	dbg_b(2,rb,r);
	int l = lsc_to_len(rb[0]); // get logical size from header

	// seek file back to tp+hdr (skip the ID header to the start of the 12 bytes of user ID data)
	if (lseek(disk_img_fd,s+1,SEEK_SET)!=s+1) {
		dbg(1,"failed seek %d : %s\n",s+1,strerror(errno));
		(void)(close(disk_img_fd)+1);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}

	ret_fdc_std(ERR_FDC_SUCCESS,tp,l); // tell client to send data

	read_client_tty(rb,PDD1_SECTOR_ID_LEN); // read 12 bytes from client

	// write those to the file
	if (write(disk_img_fd,rb,PDD1_SECTOR_ID_LEN)<0) {
		dbg(0,"%s\n",strerror(errno));
		(void)(close(disk_img_fd)+1);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}

	// close file
	(void)(close(disk_img_fd)+1);

	// send a final success response to the client
	ret_fdc_std(ERR_FDC_SUCCESS,tp,l);
}

void req_fdc_write_sector(int tp,int tl) {
	dbg(2,"%s(%d,%d)\n",__func__,tp,tl);
	if (open_disk_image(tp,O_RDWR,ALLOW_RET)) return; // open & seek to tp
	if (read(disk_img_fd,rb,mlen)!=mlen) { // read ID section
		dbg(0,"failed read ID\n");
		(void)(close(disk_img_fd)+1);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}

	int l = lsc_to_len(rb[0]); // get logical size from header

	// seek to target_physical*full_sectors+ID+target_logical*logical_size
	int s = (tp*(mlen+dlen))+mlen+((tl-1)*l);
	if (lseek(disk_img_fd,s,SEEK_SET)!=s) {
		dbg(0,"failed seek %d : %s\n",s,strerror(errno));
		(void)(close(disk_img_fd)+1);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}

	ret_fdc_std(ERR_FDC_SUCCESS,tp,l); // tell client to send data

	read_client_tty(rb,l); // read logical_size bytes from client

	// write them to the file
	if (write(disk_img_fd,rb,l)<0) {
		dbg(0,"%s\n",strerror(errno));
		(void)(close(disk_img_fd)+1);
		ret_fdc_std(ERR_FDC_READ,tp,0);
		return;
	}

	// close file
	(void)(close(disk_img_fd)+1);

	// send a final success response to the client
	ret_fdc_std(ERR_FDC_SUCCESS,tp,l);
}

// ref/fdc.txt
void get_fdc_cmd(void) {
	dbg(3,"%s()\n",__func__);
	char b[6] = {0x00};
	unsigned i = 0;
	bool eol = false;
	char c = 0x00;
	int p = -1;
	int l = -1;

	// see if the command byte was collected already by req_fdc()
	if (gb[0]>0x00 && gb[0]!=FDC_CMD_EOL && gb[1]==0x00) c=gb[0];
	memset(gb,0x00,TPDD_DATA_MAX);

	// scan for a valid command byte first
	while (!c) {
		read_client_tty(&c,1);
		if (c==FDC_CMD_EOL) { eol=true; c=0x20; break; } // fall through to ERR_FDC_COMMAND, important for Sardine
		if (!strchr(fdc_cmds,c)) c=0x20 ; // eat bytes until valid cmd or eol
	}

	// read params
	while (i<6 && !eol) { // if we get out of sync (i>5), just fall through to restart
		if (read_client_tty(&b[i],1)==1) {
			dbg(3,"i:%d b:\"%s\"\n",i,b);
			switch (b[i]) {
				case FDC_CMD_EOL: eol=true;
				case 0x20: b[i]=0x00; break;
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
	// (format & condition have different meanings but the rule still holds)
	p=0; // real drive uses physical sector 0 when omitted
	l=1; // real drive uses logical sector 1 when omitted
	char* t;
	if ((t=strtok(b,","))!=NULL) p=atoi(t); // target physical sector number
	if ((t=strtok(NULL,","))!=NULL) l=atoi(t); // target logical sector number
	if (p<0 || p>79 || l<1 || l>20) {ret_fdc_std(ERR_FDC_PARAM,0,0); return;}

	// debug
	dbg(3,"\"c:%c p:%d l:%d\"\n",c,p,l);

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
		default: dbg(3,"FDC: invalid cmd \"%s\"\n",b);
			ret_fdc_std(ERR_FDC_COMMAND,0,0); // required for model detection
	}
}

////////////////////////////////////////////////////////////////////////
//
//  OPERATION MODE
//

FILE_ENTRY *make_file_entry(char *namep, uint16_t len, char flags) {
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
		snprintf(f.client_fname,25,"%-24.24s",namep);
	}

	dbg(1,"\"%s\"\t%s%s\n",f.client_fname,f.local_fname,f.flags&FE_FLAGS_DIR?"/":"");
	return &f;
}

// standard return - return for: error open close delete status write
void ret_std(unsigned char err) {
	dbg(3,"%s()\n",__func__);
	gb[0]=RET_STD;
	gb[1]=1;
	gb[2]=err;
	gb[3]=checksum(gb);
	dbg(3,"Response: %02X\n",err);
	write_client_tty(gb,4);
	if (gb[2]!=ERR_SUCCESS) dbg(2,"ERROR RESPONSE TO CLIENT\n");
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
		flags=FE_FLAGS_NONE;

		if (stat(dire->d_name,&st)) {
			if (m) ret_std(ERR_NO_FILE);
			return 0;
		}

		if (S_ISDIR(st.st_mode)) flags=FE_FLAGS_DIR;
		else if (!S_ISREG (st.st_mode)) continue;

		if (flags==FE_FLAGS_DIR && !dme_detected) continue;

		if (dot_offset) {
			if (dire->d_name[0]=='.') continue; // skip "." ".." and hidden files
			if (strlen(dire->d_name)>LOCAL_FILENAME_MAX) continue; // skip long filenames
		}

		// If filesize is too large for the tpdd 16bit filesize field, still
		// allow the file to be accessed, because REXCPM (cpmupd.CO) violates
		// the protocol to load a CP/M disk image. But declare the size 0
		// rather than give a random value from taking only 16 of 32 bits.
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
	DIR * dir;

	dir=opendir(".");
	file_list_clear_all();
	dbg(1,"-------------------------------------------------------------------------------\n");
	if (dir_depth) add_file(make_file_entry("..", 0, FE_FLAGS_DIR));
	while (read_next_dirent(dir,m));
	dbg(1,"-------------------------------------------------------------------------------\n");
	closedir(dir);
}

// return for dirent
int ret_dirent(FILE_ENTRY *ep) {
	dbg(2,"%s(\"%s\")\n",__func__,ep->client_fname);
	int i;

	memset(gb,0x00,TPDD_DATA_MAX);
	gb[0]=RET_DIRENT;
	gb[1]=LEN_RET_DIRENT;

	if (ep) {
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

	// free sectors
	gb[29] = model==1?(PDD1_TRACKS*PDD1_SECTORS):(PDD2_TRACKS*PDD2_SECTORS);

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
	} else if (check_magic_file(filename)==0) {
		// let UR2 load <root>/DOSxxx.CO from anywhere
		cur_file=make_file_entry(filename,0,0);
		char t[LOCAL_FILENAME_MAX+1] = {0x00};
		// try share root
		for (int i=dir_depth;i>0;i--) strcat(t,"../");
		strncat(t,cur_file->local_fname,LOCAL_FILENAME_MAX-dir_depth*3);
		struct stat st; int e=stat(t,&st);
		if (e) { // try loaders dir
			strcpy(t,app_lib_dir);
			strcat(t,"/");
			strcat(t,cur_file->local_fname);
			e=stat(t,&st);
		}
		if (e) ret_dirent(NULL); else {
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
// If you don't send all 6 bytes, TS-DOS doesn't clear the previous
// contents from the display
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
	gb[9]=0x00;   // gb[9]='.';  // contents don't matter but length does
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
		if (gb[0]==FDC_CMD_EOL) dme_fdc = true;
	}
	if (dme_fdc) {
		dme_detected=true;
		dbg(3,"dme detected\n");
		ret_dme_cwd();
	} else {
		//if (model==2) { ret_std(ERR_PARAM); return; } // real tpdd2 returns
		opr_mode = 0;
		dbg(1,"Switching to \"FDC\" mode\n"); // no response to client, just switch modes
	}
}

// b[0] = fmt  0x01
// b[1] = len  0x01
// b[2] = mode 0x01 write new
//             0x02 write append
//             0x03 read
// b[3] = chk
int req_open(unsigned char *b) {
	dbg(2,"%s(\"%s\")\n",__func__,cur_file->client_fname);
	dbg(5,"b[]\n"); dbg_b(5,b,-1);
	dbg_p(4,b);

	unsigned char omode = b[2];

	switch(omode) {
	case F_OPEN_WRITE:
		dbg(3,"mode: write\n");
		if (o_file_h >= 0) {
			close(o_file_h);
			o_file_h=-1;
		}
		if (cur_file->flags&FE_FLAGS_DIR) {
			if (mkdir(cur_file->local_fname,0755)==0) {
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
		dbg(3,"mode: append\n");
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
		dbg(3,"mode: read\n");
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
	}
	return o_file_h;
}

void req_read(void) {
	if (ch[1]!=REQ_READ || debug>2) dbg(2,"%s()\n",__func__);
	int i;

	if (o_file_h<0) {
		ret_std(ERR_CMDSEQ);
		return;
	}
	if (f_open_mode!=F_OPEN_READ) {
		ret_std(ERR_FMT_MISMATCH);
		return;
	}

	i = read(o_file_h, gb+2, REQ_RW_DATA_MAX);

	gb[0]=RET_READ;
	gb[1] = (unsigned char) i;
	gb[2+i] = checksum(gb);

	if (debug<4) {
		dbg(1,".");
		if (i<REQ_RW_DATA_MAX) dbg(1,"\n");
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

	if (debug<4) {
		dbg(1,".",b[1]);
		if (b[1]<REQ_RW_DATA_MAX) dbg(1,"\n");
	}

	if (write (o_file_h,b+2,b[1]) != b[1]) ret_std (ERR_SECTOR_NUM);
	else ret_std (ERR_SUCCESS);
}

void req_delete(void) {
	dbg(2,"%s()\n",__func__);
	if (cur_file->flags&FE_FLAGS_DIR) rmdir(cur_file->local_fname);
	else unlink (cur_file->local_fname);
	dbg(1,"Deleted: %s\n",cur_file->local_fname);
	ret_std (ERR_SUCCESS);
}

void ret_cache_std(int e) {
	dbg(3,"%s()\n",__func__);
	gb[0]=RET_CACHE_STD;
	gb[1]=0x01;
	gb[2]=e;
	gb[3]=checksum(gb);
	write_client_tty(gb,4);
}


/*
 * Load a sector from disk into rb[], or unload rb[] to a sector on the disk,
 * and like a real drive, flushing the cache to disk does not clear the cache.
 *
 * rb[] (record buffer) is the drive cache
 *
 * Load/Unload Cache
 * b[0] fmt 0x30
 * b[1] len 0x05
 *   b[2] action 0=load (cache<disk) 2=unload (cache>disk)
 *   b[3] -
 *   b[4] track 0-79
 *   b[5] -
 *   b[6] sector 0-1
 */
void req_cache_load(unsigned char *b) {
	dbg(3,"%s(action=%u track=%u sector=%u)\n",__func__,b[2],b[4],b[6]);
	if (model==1) return;
	int a=b[2];
	int t=b[4];
	int s=b[6];
	if ((a!=0 && a!=2) || t>79 || s>1) { ret_cache_std(ERR_PARAM); return; }
	int rn = t*2 + s; // convert track#:sector# to linear record#
	int rl = mlen+dlen;
	int e;

	switch (a) {
		case CACHE_LOAD:
			dbg(2,"cache load: track:%u  sector:%u\n",t,s);
			e = open_disk_image (rn, O_RDONLY, NO_RET );
			switch (e) { // convert the FDC error codes to equivalent OPR error codes
				case ERR_FDC_NO_DISK: e=ERR_NO_DISK; break;
				case ERR_FDC_WRITE_PROTECT: e=ERR_WRITE_PROTECT; break;
				case ERR_FDC_READ: e=ERR_FMT_INTERRUPT; break;
				case ERR_FDC_SUCCESS: e=ERR_SUCCESS;
			}
			if (e) { ret_cache_std(e); return; }
			memset(rb,0x00,rl);
			if (read(disk_img_fd,rb,rl)!=rl) {
				dbg(2,"failed cache load\n");
				(void)(close(disk_img_fd)+1);
				ret_cache_std(ERR_DEFECTIVE);
				return;
			}
			break;
		case CACHE_UNLOAD:
			dbg(2,"cache unload: track:%u  sector:%u\n",t,s);
			e = open_disk_image (rn, O_WRONLY, NO_RET );
			switch (e) { // convert the FDC error codes to equivalent OPR error codes
				case ERR_FDC_NO_DISK: e=ERR_NO_DISK; break;
				case ERR_FDC_WRITE_PROTECT: e=ERR_WRITE_PROTECT; break;
				case ERR_FDC_READ: e=ERR_FMT_INTERRUPT; break;
				case ERR_FDC_SUCCESS: e=ERR_SUCCESS;
			}
			if (e) { ret_cache_std(e); return; }
			if (write(disk_img_fd,rb,rl)!=rl) {
				dbg(2,"failed cache unload\n");
				(void)(close(disk_img_fd)+1);
				ret_cache_std(ERR_DEFECTIVE);
				return;
			}
	}
	(void)(close(disk_img_fd)+1);
	dbg_b(3,rb,rl);
	ret_cache_std(ERR_SUCCESS);
}

/* Emulating access to the main data area is pretty straightforward, but the
 * metadata area (or maybe it should be called a mode?) is mostly a mystery.
 * So we aren't really implementing the full whatever a real drive does for
 * metadata read/write, because we don't know what a real drive does. We just
 * recognize one magic value for "offset" (which is probably not actually an
 * offset while in metadata mode, but just 2 bytes with some other meaning),
 * and treat that as an otherwise ordinary access but to the 4-byte metadata
 * field instead of the main data field, and just return "success" for all
 * other access to the metadata area without actually doing anything.
 * Since all other observed accesses to the metadata area are always just writes
 * without matching reads, and the data is always the same, it suggests these
 * other writes are not storing data but issuing commands to control the drive.
 *
 * Some metadata accesses from common clients, not including ZZ or checksum:
 * "metadatat access" = command 0x31 or 0x32, area/mode 0x01
 *
 * BACKUP.BA  len    area   offset      data
 * 0x31,      0x04,  0x01,  0x00,0x83,  0x00,
 * 0x31,      0x04,  0x01,  0x00,0x96,  0x00,
 * 0x31,      0x07,  0x01,  0x80,0x04,  0x16,0x00,0x00,0x00    (data varies) this is the only one we actually do anything
 *
 * TS-DOS
 * 0x31,      0x04,  0x01,  0x00,0x84,  0xFF,
 * 0x31,      0x04,  0x01,  0x00,0x96,  0x0F,
 * 0x31,      0x04,  0x01,  0x00,0x94,  0x0F,
 */

/*
 * req:
 * b[0] fmt
 * b[1] len 4
 *      b[2] area        0=data 1=meta
 *      b[3] offset msb  0000-0500
 *      b[4] offset lsb
 *      b[5] dlen        00-FC
 * b[6] chk
 *
 * ret:
 * b[0] fmt ret_cache_read
 * b[1] len
 *      b[2] area        0=data 1=meta
 *      b[3] offset msb
 *      b[4] offset lsb
 *      b[5+] data       dlen bytes
 * b[#] chk
 */
void req_cache_read(unsigned char *b) {
	dbg(3,"%s()\n",__func__);
	if (model==1) return;
	int a = b[2];
	int o = b[3]*256+b[4];
	int l = b[5];
	int e = -1;
	dbg(2,"cache_read: area:%u  offset:%u  len:%u\n",a,o,l);
	switch (a) {
		case CACHE_AREA_DATA:
			if (o+l>SECTOR_DATA_LEN || l>PDD2_CACHE_READ_MAX) e=ERR_PARAM;
			o+=PDD2_SECTOR_META_LEN; // shift offset past metadata field
			break;
		case CACHE_AREA_META:
			if (o==PDD2_META_ADDR) { o=0; if (l>PDD2_SECTOR_META_LEN) e=ERR_PARAM; } // set offset to start of metadata field
			else e=ERR_PARAM; // this is wrong, real drive returns all kinds of data
			break;
		default: e=ERR_PARAM;
	}
	if (e!=-1) { ret_cache_std(e); return; }
	dbg(3,"offset:%u  len:%u\n",o,l);

	// copy some data from rb[] and return to client
	gb[0]=RET_CACHE_READ;
	gb[1]=3+l;  // len = area + omsb + olsb + data
	gb[2]=b[2]; // area
	gb[3]=b[3]; // offset msb
	gb[4]=b[4]; // offset lsb
	memcpy(gb+5,rb+o,l); // data
	gb[2+gb[1]]=checksum(gb); // chk
	dbg_b(3,gb,-1);
	write_client_tty(gb,2+gb[1]+1);
}

/*
 * TPDD2 sector cache write
 * Previously called "TS-DOS mystery command 1" and had a canned response,
 * now actually implements the function. Aside from being a normal PDD2 sector
 * access command, TS-DOS uses it just to detect TPDD2
 * 
 * b[0] fmt
 * b[1] len
 *      b[2] area
 *      b[3] offset msb
 *      b[4] offset lsb
 *      b[5+] data
 * b[#] chk
 */
void req_cache_write(unsigned char *b) {
	dbg(3,"%s()\n",__func__);
	if (model==1) return;
	int a = b[2];
	int o = b[3]*256+b[4];
	int s = 5; // start of data
	int l = b[1]-3; // length of data = length of packet - area - omsb - olsb
	int e = -1;
	dbg(2,"cache_write: area:%u  offset:%u  len:%u\n",a,o,l);
	switch (a) {
		case CACHE_AREA_DATA:
			if (o+l>SECTOR_DATA_LEN || l>PDD2_CACHE_WRITE_MAX) e=ERR_PARAM;
			o+=PDD2_SECTOR_META_LEN; // shift offset past metadata field
			break;
		case CACHE_AREA_META:
			if (o==PDD2_META_ADDR) { o=0 ;if (l>PDD2_SECTOR_META_LEN) e=ERR_PARAM; } // set offset to start of metadata field
			else e=ERR_SUCCESS; // thumbs-up but don't actually do anything
			break;
		default: e=ERR_PARAM;
	}
	if (e!=-1) { ret_cache_std(e); return; }
	dbg(3,"offset:%u  len:%u\n",o,l);

	// copy data from client over part of rb[]
	dbg_b(3,b+s,l);
	dbg_b(3,rb,mlen+dlen);
	memcpy(rb+o,b+s,l);
	dbg_b(3,rb,mlen+dlen);
	ret_cache_std(ERR_SUCCESS);
}

/*
 * Another part of TS-DOS's drive/server capabilities detection scheme.
 * Previously called "TS-DOS mystery command 2"
 * The function of the command in a real drive is unknown.
 * The meaning of the response is unkmnown.
 * But the command apparently takes no parameters, and a real TPDD2 always
 * responds with the same string of bytes, and TPDD1 ignores it.
 * not counting ZZ or checksums:
 * Client sends  : 23 00
 * TPDD2 responds: 14 0F 41 10 01 00 50 05 00 02 00 28 00 E1 00 00 00
 * TPDD1 does not respond.
 */
void ret_pdd2_unk23() {
	dbg(3,"%s()\n",__func__);
	if (model==1) return;
	static unsigned char canned[] = UNK23_RET_DAT;
	memcpy(gb, canned, canned[1]+2);
	gb[canned[1]+2] = checksum(gb);
	write_client_tty(gb, gb[1]+3);
}

/*
 * Similar to unk23, except the response is different, and not used by TS-DOS.
 * Nothing is known to use this command. It was just found by feeding arbitrary
 * commands to a real drive with github/bkw777/pdd.sh
 * 0x11 and 0x33 both produce the same response. Possibly 0x11 and 0x33 are
 * just different versions of the same function, like how 0x4# commands are
 * really just 0x0# commands for bank 1 instead of bank 0? Just a guess.
 * not counting ZZ or checksums:
 * Client sends  : 11 00
 *     or sends  : 33 00
 * TPDD2 responds: 3A 06 80 13 05 00 10 E1
 */
void ret_pdd2_unk11() {
	dbg(3,"%s()\n",__func__);
	if (model==1) return;
	static unsigned char canned[] = UNK11_RET_DAT;
	memcpy(gb, canned, canned[1]+2);
	gb[canned[1]+2] = checksum(gb);
	write_client_tty(gb, gb[1]+3);
}

void req_rename(unsigned char *b) {
	dbg(3,"%s(%-24.24s)\n",__func__,b+2);
	if (model==1) return;
	char *t = (char *)b + 2;
	memcpy(t,collapse_padded_fname(t),TPDD_FILENAME_LEN);
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

// opr-format - this creates a disk that can load & save files
// the only difference from fdc-format is a single byte, the first byte of the SMT
// opr-format is just this:
//   start with: fdc-format 0    (0=64-byte logical sector size)
//   then: sector 0, byte 1240, write 0x80 (or physical:0 logical:20 byte:25 counting from 1
void req_format() {
	dbg(2,"%s()\n",__func__);
	const int rl = mlen+dlen; // img file record length
	const int rc = model==1?(PDD1_TRACKS*PDD1_SECTORS):(PDD2_TRACKS*PDD2_SECTORS); // records count
	int rn = 0;          // record number

	dbg(0,"Operation-mode Format (make a filesystem)\n");

	int e = open_disk_image(0,O_WRONLY,NO_RET);
	switch (e) { // convert the FDC error codes to equivalent OPR error codes
		case ERR_FDC_NO_DISK: e=ERR_NO_DISK; break;
		case ERR_FDC_WRITE_PROTECT: e=ERR_WRITE_PROTECT; break;
		case ERR_FDC_READ: e=ERR_FMT_INTERRUPT; break;
		case ERR_FDC_SUCCESS: e=0;
	}
	if (e) { ret_std(e); return; }

	// write the image
	// TPDD1 fresh OPR-mode format is strange.
	// Sector 0 gets logical size code 0, and all other sectors get lsc 1.
	// We exactly mimick that here "just because", even though the lsc 1's
	// don't seem to serve any purpose or have any effect.
	for (rn=0;rn<rc;rn++) {
		memset(rb,0x00,rl);
		switch (model) {
			case 1: if (rn==0) rb[PDD1_SECTOR_META_LEN+SMT_OFFSET]=PDD1_SMT; else rb[0]=1; break;
			default: rb[0]=0x16; if (rn<2) { rb[1]=0xFF; rb[PDD2_SECTOR_META_LEN+SMT_OFFSET]=PDD2_SMT; }
		}
		if (write(disk_img_fd,rb,rl)<0) break;
	}

	if (rn<rc) {
		dbg(0,"%s\n",strerror(errno));
		(void)(close(disk_img_fd)+1);
		ret_std(ERR_FMT_INTERRUPT);
		return;
	}

	(void)(close(disk_img_fd)+1);

	ret_std(ERR_SUCCESS);
}

void get_opr_cmd(void) {
	dbg(3,"%s()\n",__func__);
	unsigned char b[TPDD_DATA_MAX] = {0x00};
	unsigned i = 0;
	memset(gb,0x00,TPDD_DATA_MAX);

	while (read_client_tty(&b,1) == 1) {
		if (b[0]==OPR_CMD_SYNC) i++; else { i=0; b[0]=0x00; continue; }
		if (i<2) { b[0]=0x00; continue; }
		if (read_client_tty(&b,2) == 2) if (read_client_tty(&b[2],b[1]+1) == b[1]+1) break;
		i=0; memset(b,0x00,TPDD_DATA_MAX	);
	}

	dbg_p(3,b);

	if ((i=checksum(b))!=b[b[1]+2]) {
		dbg(0,"Failed checksum: received: %02X  calculated: %02X\n",b[b[1]+2],i);
		return; // real drive does not return anything
	}

	ch[1]=ch[0]; ch[0]=b[0]; // command history

	// dispatch
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
		case REQ_CACHE_LOAD:    req_cache_load(b);   break;
		case REQ_CACHE_READ:    req_cache_read(b);   break;
		case REQ_CACHE_WRITE:   req_cache_write(b);  break;
		case REQ_PDD2_UNK11:    ret_pdd2_unk11();    break;
		case REQ_PDD2_UNK33:    ret_pdd2_unk11();    break;
		default: dbg(1,"OPR: unknown cmd \"%02X\"\n",b[0]); if (debug<3) dbg_p(2,b);
		// local msg, nothing to client
	}
}

////////////////////////////////////////////////////////////////////////
//
//  BOOTSTRAP
//

void show_bootstrap_help() {
	dbg(0,"%1$s - DeskLink+ " APP_VERSION " - \"bootstrap\" help\n\n"
		"Available loader files (in %2$s):\n\n",args[0],app_lib_dir);

	dbg(0,  "TRS-80 Model 100/102 :"); lsx(app_lib_dir,"100");
	dbg(0,"\nTANDY Model 200      :"); lsx(app_lib_dir,"200");
	dbg(0,"\nNEC PC-8201/PC-8300  :"); lsx(app_lib_dir,"NEC");
	dbg(0,"\nKyotronic KC-85      :"); lsx(app_lib_dir,"K85");
	dbg(0,"\nOlivetti M-10        :"); lsx(app_lib_dir,"M10");

	dbg(0,
		"\n\n"
		"Filenames given without any path are searched from above\n"
		"as well as the current dir.\n"
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
	if (debug) { // local display nicely regardless if CR, LF, or CRLF
		if (b!=LOCAL_EOL && ch[0]==LOCAL_EOL) {ch[0]=0x00; dbg(0,"%c%c",LOCAL_EOL,b);}
		else if (b==LOCAL_EOL || b==BASIC_EOL) ch[0]=LOCAL_EOL;
		else if (b<32 || b>126) dbg(0,"\033[7m^%02X\033[m",b); // hardcoded ansi/vt codes, sorry
		else dbg(0,"%c",b);
	}
}

int send_BASIC(char *f) {
	int fd;
	char b;

	if ((fd=open(f,O_RDONLY))<0) {
		dbg(1,"Could not open \"%s\" : %s\n",f,errno);
		return 9;
	}

	dbg(0,"Sending \"%s\" ... ",f);
	dbg(1,"\n");
	while(read(fd,&b,1)==1) slowbyte(b);
	close(fd);
	if (dot_offset) { // if not in raw mode supply missing trailing EOF & EOL
		if (b!=LOCAL_EOL && b!=BASIC_EOL && b!=BASIC_EOF) slowbyte(BASIC_EOL);
		if (b!=BASIC_EOF) slowbyte(BASIC_EOF);
	}
	close(client_tty_fd);
	dbg(1,"\n");
	dbg(0,"DONE\n\n");
	return 0;
}

int bootstrap(char *f) {
	dbg(0,"Bootstrap: Installing \"%s\"\n\n",f);
	if (access(f,F_OK)==-1) {
		dbg(0,"Not found.\n");
		return 1;
	}

	char t[PATH_MAX]={0x00};

	strcpy(t,f);
	strcat(t,".pre-install.txt");
	if (!access(t,F_OK)) dcat(t);
	else dbg(0,"Prepare BASIC to receive:\n"
		"\n"
		"    RUN \"COM:98N1ENN\" [Enter]    <-- TANDY/Olivetti/Kyotronic\n"
		"    RUN \"COM:9N81XN\"  [Enter]    <-- NEC\n");

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
	dbg(0,"getty_mode      : %s\n",getty_mode?"true":"false");
	dbg(0,"upcase          : %s\n",upcase?"true":"false");
	dbg(0,"rtscts          : %s\n",rtscts?"true":"false");
	dbg(0,"verbosity       : %d\n",debug);
	dbg(0,"model           : %d\n",model);
	dbg(0,"dot_offset      : %d\n",dot_offset);
	dbg(0,"BASIC_byte_ms   : %d\n",BASIC_byte_us/1000);
	dbg(0,"bootstrap_mode  : %s\n",bootstrap_mode?"true":"false");
	dbg(0,"bootstrap_fname : \"%s\"\n",bootstrap_fname);
	dbg(0,"app_lib_dir     : \"%s\"\n",app_lib_dir);
	dbg(0,"client_tty_name : \"%s\"\n",client_tty_name);
	dbg(0,"disk_img_fname  : \"%s\"\n",disk_img_fname);
	dbg(0,"share_path      : \"%s\"\n",cwd);
	dbg(2,"opr_mode        : %d\n",opr_mode);
	dbg(2,"baud            : %d\n",get_baud());
	dbg(0,"dme_disabled    : %s\n",dme_disabled?"true":"false");
	dbg(2,"dme_root_label  : \"%-6.6s\"\n",dme_root_label);
	dbg(2,"dme_parent_label: \"%-6.6s\"\n",dme_parent_label);
	dbg(2,"dme_dir_label   : \"%-2.2s\"\n",dme_dir_label);
	dbg(0,"magic_files     : %s\n",enable_magic_files?"enabled":"disabled");
	dbg(2,"default_attr    : '%c'\n",default_attr);
}

void show_main_help() {
	dbg(0,
		"%1$s - DeskLink+ " APP_VERSION " - main help\n\n"
		"usage: %1$s [options] [tty_device] [share_path]\n"
		"\n"
		"options:\n"
		"   -0       Raw mode - no filename munging, attr = ' '\n"
		"   -a c     Attr - attribute used for all files (%2$c)\n"
		"   -b file  Bootstrap - send loader file to client\n"
		"   -d tty   Serial device connected to client (" DEFAULT_CLIENT_TTY ")\n"
		"   -e       Disable TS-DOS directory extension (enabled)\n"
		"   -g       Getty mode - run as daemon\n"
		"   -h       Print this help\n"
		"   -i file  Disk image file for raw sector access, TPDD1 only\n"
		"   -l       List loader files and show bootstrap help\n"
		"   -m model Model: 1 for TPDD1, 2 for TPDD2 (2)\n"
		"   -p dir   Share path - directory with files to be served (.)\n"
		"   -r       RTS/CTS hardware flow control\n"
		"   -s #     Speed - serial port baud rate 9600 or 19200 (19200)\n"
		"   -u       Uppercase all filenames\n"
		"   -v       Verbose/debug mode - more v's = more verbose\n"
		"   -w       WP-2 mode - 8.2 filenames\n"
		"   -z #     Milliseconds per byte for bootstrap (%3$d)\n"
		"\n"
		"The 1st non-option argument is another way to specify the tty device.\n"
		"The 2nd non-option argument is another way to specify the share path.\n"
		"\n"
		"   %1$s\n"
		"   %1$s -vvu -p ~Downloads/REX/ROMS\n"
		"   %1$s -v -w ttyUSB1 ~/Documents/wp2files\n\n"
	,args[0],DEFAULT_TPDD_FILE_ATTR,DEFAULT_BASIC_BYTE_MS);
}

int main(int argc, char **argv) {
	dbg(0,"DeskLink+ " APP_VERSION "\n");

	int i;
	bool x = false;
	args = argv;

	// environment
	if (getenv("OPR_MODE")) opr_mode = atoi(getenv("OPR_MODE"));
	if (getenv("DISABLE_DME")) dme_disabled = true;
	if (getenv("DISABLE_MAGIC_FILES")) enable_magic_files = false;
	if (getenv("DOT_OFFSET")) dot_offset = atoi(getenv("DOT_OFFSET"));
	if (getenv("BAUD")) set_baud(getenv("BAUD"));
	if (getenv("ROOT_LABEL")) {snprintf(dme_root_label,7,"%-6.6s",getenv("ROOT_LABEL"));
		memcpy(dme_cwd,dme_root_label,6);}
	if (getenv("PARENT_LABEL")) snprintf(dme_parent_label,7,"%-6.6s",getenv("PARENT_LABEL"));
	if (getenv("DIR_LABEL")) snprintf(dme_dir_label,3,"%-2.2s",getenv("DIR_LABEL"));
	if (getenv("ATTR")) default_attr = *getenv("ATTR");

	// commandline
	while ((i = getopt (argc, argv, ":0a:b:d:eghi:lm:p:rs:uvwz:^")) >=0)
		switch (i) {
			case '0': dot_offset=0; upcase=false; default_attr=0x20;      break;
			case 'a': default_attr=*strndup(optarg,1);                    break;
			case 'b': bootstrap_mode=true; strcpy(bootstrap_fname,optarg);break;
			case 'd': strcpy(client_tty_name,optarg);                     break;
			case 'e': dme_disabled = true;                                break;
			case 'g': getty_mode = true; debug = 0;                       break;
			case 'h': show_main_help(); exit(0);                          break;
			case 'i': strcpy(disk_img_fname,optarg);                      break;
			case 'l': show_bootstrap_help(); exit(0);                     break;
			case 'm': model=atoi(optarg);                                 break;
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
				if (isprint(optopt)) dbg(0,"Unknown option \"-%c\"\n",optopt);
				else dbg(0,"Unknown option character \"0x%02X\"\n",optopt);
			default: show_main_help();                                 return 1;
		}
	// commandline non-option arguments

	for (i=0; optind < argc; optind++) {
		if (x) dbg(1,"non-option arg %u: \"%s\"\n",i,argv[optind]);
		switch (i++) {
			case 0: strcpy (client_tty_name,argv[optind]); break; // tty device
			case 1: (void)(chdir(argv[optind])+1); break; // share path
			default: dbg(0,"Unknown argument: \"%s\"\n",argv[optind]);
		}
	}

	// auto-completes & fixups
	if (model<1||model>2) model=2;
	if (model==1) mlen = PDD1_SECTOR_META_LEN;
	resolve_client_tty_name();
	find_lib_file(disk_img_fname);
	if (disk_img_fname[0]) rb = malloc(mlen+dlen);
	find_lib_file(bootstrap_fname);
	(void)(getcwd(cwd,PATH_MAX-1)+1);

	if (x) { show_config(); return 0; }

	dbg(0,"Serial Device: %s\n"
		  "Working Dir  : %s\n",client_tty_name,cwd);

	if ((i=open_client_tty())) return i;

	// send loader and exit
	if (bootstrap_mode) return (bootstrap(bootstrap_fname));

	// initialize the file list
	file_list_init();
	if (debug) update_file_list(NO_RET);

	// process commands forever
	while (1) if (opr_mode) get_opr_cmd(); else get_fdc_cmd();

	// file_list_cleanup()
	return 0;
}
