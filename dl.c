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
 * byte offsets foo[0], foo[1], foo[2], foo+2, foo[foo[1]+2], etc.
 * 
 * functions named req_*() receive a command in this format
 * functions named ret_*() generate a response in this format
 * 
 * There is also an FDC-mode that TPDD1/FB-100 drives have, which has
 * a completely different format, but to date this program only
 * implements Operation-mode. TPDD2 drives do not have FDC-mode, but
 * they do have extra Operation-mode commands that TPDD1 does not have,
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

#ifndef DEFAULT_CLIENT_MODEL
#define DEFAULT_CLIENT_MODEL 100
#endif

#ifndef DEFAULT_CLIENT_APP
#define DEFAULT_CLIENT_APP TEENY
#endif

#define DEFAULT_BASIC_BYTE_MSEC 6
#define DEFAULT_TPDD_FILE_ATTRIB 'F' // 0x46
#define DEFAULT_DME_ROOT_LABEL   "ROOT  "
#define DEFAULT_DME_PARENT_LABEL "PARENT"

////////////////////////////////////////////////////////////////////////
//
//  Experimental feature selections

// serial tty read() behavior
// VMIN blocking method not working. The idea was to set VTIME=0 VMIN=nbytes
// just before each read(), to make read() block until n bytes have been
// received, instead of polling. Seems to be only partially working. It
// does seem to produce larger contiguous reads, but still not full.
//
// Go into TS-DOS and select a large file and try to save it.
// F1-Save -> req_write() -> read_client_tty() -> "expected 129 bytes, got 64"
// Very consistent and repeatable, every single time not intermittent.
//
// And yet perror() says Success, and read() returnd 64 not 0 or -1.
// So read() is not blocking like every web page claims this combination
// of settings will do.
//
// However, adding retry around that works fine. See read_client_tty().
//
// 0 Is the safest fallback in case of problems. It's still better now 
//   than it used to be thanks to changing the loop to read in as big of
//   chunks as read() will deliver, instead of one byte per read(), and
//   because of the VTIME/VMIN defaults below, now even though it doesn't
//   block for the whole read like I want, it does at least block until at least
//   the first byte, which is enough to free the cpu 99%. No race.
//
// 1 Is described above, and exhibits the incomplete reads problem.
//
// 2 ... is "1" with retries instead of bailing. Best of both worlds?
//   64 out of 129 bytes is still 64x better than 1. I'm not sure if it's
//   expensive or abusive or otherwise not recommended to be calling
//   tcsetattr() so frequently. It may be that 0 is best.
#define READ_TTY_METHOD 2 // 0 normal, 1 VMIN blocking, 2 VMIN blocking plus retries.

// Two different forms of the ZZ scanner at the top of get_opr_cmd()
// Both seem solid.
#define ZZ_SCAN_METHOD 1

// These values are set to a dynamic value just before every read()
// and restored immediately after if TTY_READ_METHOD>0 .
// These are the all-the-time defaults for TTY_READ_METHOD=0,
// and in between reads in all cases. These are reasonable defaults.
// In particulare they make the read() loop block at least until the
// first/next byte is available, which is enough to free the cpu
// even without the full 128-byte blocking I was hoping for.
#define C_CC_VMIN 1
#define C_CC_VTIME 5

// This controls if the tty open() includes O_NONBLOCK. Man pages
// and web guides suggest this just sets if the tty will honor
// or ignore the DSR/DTR/DCD lines. Others say that VTIME/VMIN
// will *not* block if O_NONBLOCK and/or O_NDELAY or non-canonical
// mode in general are set.
// The original code had O_NONBLOCK in the open() call. (Same as
// 1 here). TS-DOS does use DSR/DTR to detect drive readiness
// with a real drive, and has no other form of flow conytrol.
// It can be confusing or annoying for users unfamiliar with
// serial cabling, but I suggest leaving this false / 0.
#define IGNORE_DSR 0

/*************************************************************/

// drive firmware/protocol constants

// TPDD request block formats
#define REQ_DIRENT        0x00
#define REQ_OPEN          0x01
#define REQ_CLOSE         0x02
#define REQ_READ          0x03
#define REQ_WRITE         0x04
#define REQ_DELETE        0x05
#define REQ_FORMAT        0x06
#define REQ_STATUS        0x07
#define REQ_FDC           0x08
#define REQ_SEEK          0x09
#define REQ_TELL          0x0A
#define REQ_SET_EXT       0x0B
#define REQ_CONDITION     0x0C // TPDD2
#define REQ_RENAME        0x0D
#define REQ_REQ_EXT_QUERY 0x0E
#define REQ_COND_LIST     0x0F
#define REQ_TSDOS_MYSTERY 0x23 // TS-DOS mystery - part of drive/emulator detection
#define REQ_CACHE_LOAD    0x30 // TPDD2 sector access
#define REQ_CACHE_WRITE   0x31 // TPDD2 sector access
#define REQ_CACHE_READ    0x32 // TPDD2 sector access

// TPDD return block formats
#define RET_READ          0x10
#define RET_DIRENT        0x11
#define RET_STD           0x12 // shared return format for: error open close delete status write
#define RET_TSDOS_MYSTERY 0x14
#define RET_CONDITION     0x15 // TPDD2
#define RET_CACHE_STD     0x38 // TPDD2 shared return format for: sector_cache write_cache
#define RET_READ_CACHE    0x39 // TPDD2

// directory entry request types
#define DIRENT_SET_NAME   0x00
#define DIRENT_GET_FIRST  0x01
#define DIRENT_GET_NEXT   0x02
#define DIRENT_GET_PREV   0x03 // TPDD2
#define DIRENT_CLOSE      0x04 // TPDD2

// file open access modes
#define F_OPEN_NONE       0x00  // used in here, not part of protocol
#define F_OPEN_WRITE      0x01
#define F_OPEN_APPEND     0x02
#define F_OPEN_READ       0x03

// TPDD Operation-mode error codes
#define ERR_SUCCESS       0x00 // 'Operation Complete'
#define ERR_NO_FILE       0x10 // 'File Not Found'
#define ERR_EXISTS        0x11 // 'File Exists'
#define ERR_CMDSEQ        0x30 // 'Command Parameter or Sequence Error'
#define ERR_DIR_SEARCH    0x31 // 'Directory Search Error'
#define ERR_BANK          0x35 // 'Bank Error'
#define ERR_PARAM         0x36 // 'Parameter Error'
#define ERR_FMT_MISMATCH  0x37 // 'Open Format Mismatch'
#define ERR_EOF           0x3F // 'End of File'
#define ERR_NO_START      0x40 // 'No Start Mark'
#define ERR_ID_CRC        0x41 // 'ID CRC Check Error'
#define ERR_SECTOR_LEN    0x42 // 'Sector Length Error'
#define ERR_FMT_VERIFY    0x44 // 'Format Verify Error'
#define ERR_NOT_FORMATTED 0x45 // 'Disk Not Formatted'
#define ERR_FMT_INTERRUPT 0x46 // 'Format Interruption'
#define ERR_ERASE_OFFSET  0x47 // 'Erase Offset Error'
#define ERR_DATA_CRC      0x49 // 'DATA CRC Check Error'
#define ERR_SECTOR_NUM    0x4A // 'Sector Number Error'
#define ERR_READ_TIMEOUT  0x4B // 'Read Data Timeout'
#define ERR_SECTOR_NUM2   0x4D // 'Sector Number Error'
#define ERR_WRITE_PROTECT 0x50 // 'Write-Protected Disk'
#define ERR_DISK_NOINIT   0x5E // 'Disk Not Formatted'
#define ERR_DIR_FULL      0x60 // 'Disk Full or Max File Size Exceeded or Directory Full' / TPDD2 'Directory Full'
#define ERR_DISK_FULL     0x61 // 'Disk Full'
#define ERR_FILE_LEN      0x6E // 'File Too Long' (real drive limits to 65534, we exceed for REXCPM)
#define ERR_NO_DISK       0x70 // 'No Disk'
#define ERR_DISK_CHG      0x71 // 'Disk Not Inserted or Disk Change Error' / TPDD2 'Disk Change Error'
#define ERR_DEFECTIVE     0x83 // 'Defective Disk'  (real drive needs a power-cycle to clear this error)

// TPDD1 FDC-mode commands
#define FDC_SET_MODE        'M' // set Operation-mode or FDC-mode
#define FDC_CONDITION       'D' // drive condition
#define FDC_FORMAT          'F' // format disk
#define FDC_FORMAT_NV       'G' // format disk without verify
#define FDC_READ_ID         'A' // read sector ID
#define FDC_READ_SECTOR     'R' // read sector data
#define FDC_SEARCH_ID       'S' // search sector ID
#define FDC_WRITE_ID        'B' // write sector ID
#define FDC_WRITE_ID_NV     'C' // write sector ID without verify
#define FDC_WRITE_SECTOR    'W' // write sector data
#define FDC_WRITE_SECTOR_NV 'X' // write sector data without verify

// TPDD1 FDC-mode error codes
// There is no documentation for FDC error codes.
// These are guesses from experimenting.
// These appear in the first hex pair of an 8-byte FDC-mode response.
#define ERR_FDC_SUCCESS 0         // 'OK'
#define ERR_FDC_LSN_LO 17         // 'Logical Sector Number Below Range'
#define ERR_FDC_LSN_HI 18         // 'Logical Sector Number Above Range'
#define ERR_FDC_PSN HI 19         // 'Physical Sector Number Above Range'
#define ERR_FDC_PARAM 33          // 'Parameter Invalid, Wrong Type'
#define ERR_FDC_LSSC_LO 50        // 'Invalid Logical Sector Size Code'
#define ERR_FDC_LSSC_HI 51        // 'Logical Sector Size Code Above Range'
#define ERR_FDC_NOT_FORMATTED 160 // 'Disk Not Formatted'
#define ERR_FDC_READ 161          // 'Read Error'
#define ERR_FDC_WRITE_PROTECT 176 // 'Write-Protected Disk'
#define ERR_FDC_COMMAND 193       // 'Invalid Command'
#define ERR_FDC_NO_DISK 209       // 'Disk Not Inserted'

// fixed lengths
#define TPDD_DATA_MAX 0x80
#define TPDD_FREE_SECTORS 0x50 // max valid 80 sectors
#define LEN_RET_STD 0x01
#define LEN_RET_DME 0x0B
#define LEN_RET_DIRENT 0x1C

// KC-85 platform BASIC interpreter EOF byte for bootstrap()
#define BASIC_EOF 0x1A

// configuration
int debug = 0;
bool upcase = false;
bool rtscts = false;
unsigned dot_offset = 6; // 6 for KC-85 platform, 8 for WP-2
int client_baud = DEFAULT_CLIENT_BAUD;
int BASIC_byte_msec = DEFAULT_BASIC_BYTE_MSEC;
const char dme_root_label[6] = DEFAULT_DME_ROOT_LABEL;

// state
bool getty_mode = false;
bool bootstrap_mode = false;

// globals
char **args;
int f_open_mode = F_OPEN_NONE;
int client_tty_fd = -1;
struct termios client_termios;
bool m1rec = false;
int o_file_h = -1;
unsigned char buf[TPDD_DATA_MAX+3];
char dme_cwd[6] = DEFAULT_DME_ROOT_LABEL;

FILE_ENTRY *cur_file;
int dir_depth=0;

// blarghamagargle
void ret_std(unsigned char err);

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

void print_usage()
{
	fprintf (stderr,
		"%1$s - DeskLink+ " STRINGIFY(APP_VERSION) "\nusage:\n\n"
		"%1$s [tty_device] [options]\n"
		"\n"
		"tty_device:\n"
		"    Serial device the client is connected to\n"
		"    examples: ttyS0, ttyUSB0, /dev/pts/foo4, etc...\n"
		"    default = " STRINGIFY(DEFAULT_CLIENT_TTY) "\n"
		"    \"-\" = stdin/stdout (/dev/tty)\n"
		"\n"
		"options:\n"
		"   -h       Print this help\n"
		"   -b=file  Bootstrap: Install <file> onto the portable\n"
		"   -v       Verbose/debug mode (more -v's = more verbose)\n"
		"   -g       Getty mode. Run as daemon\n"
		"   -p=dir   Path to files to be served, default is \".\"\n"
		"   -w       WP-2 compatibility mode (8.2 filenames)\n"
		"   -u       Uppercase all filenames\n"
		"   -c       Hardware flow control (RTS/CTS)\n"
		"   -z=#     Sleep # milliseconds between each byte while sending bootstrap file (default " STRINGIFY(DEFAULT_BASIC_BYTE_MSEC) ")\n"
		"\n"
		"available bootstrap files (in "STRINGIFY(APP_LIB_DIR)"):\n"
	,args[0]);

	// FIXME - This is crap using system(), and relying on an external,
	// just to get some filenames, but I don't want to write /bin/find. - bkw
	// works but blargh ...
	//(void)(system ("find " STRINGIFY(APP_LIB_DIR) " -regex \'.*/.+\\.\\(100\\|200\\|NEC\\|M10\\|K85\\)$\' -printf \'\%f\\n\' >&2")+1);
	// even more blargh...
	fprintf (stderr,   "   TRS-80 Model 100 / Tandy 102 : ");
	(void)(system ("find " STRINGIFY(APP_LIB_DIR) " -regex \'.*/.+\\.100$\' -printf \'\%f \' >&2")+1);
	fprintf (stderr, "\n   Tandy 200                    : ");
	(void)(system ("find " STRINGIFY(APP_LIB_DIR) " -regex \'.*/.+\\.200$\' -printf \'\%f \' >&2")+1);
	fprintf (stderr, "\n   NEC PC-8201/PC-8201a/PC-8300 : ");
	(void)(system ("find " STRINGIFY(APP_LIB_DIR) " -regex \'.*/.+\\.NEC$\' -printf \'\%f \' >&2")+1);
	fprintf (stderr, "\n   Kyotronic KC-85              : ");
	(void)(system ("find " STRINGIFY(APP_LIB_DIR) " -regex \'.*/.+\\.K85$\' -printf \'\%f \' >&2")+1);
	fprintf (stderr, "\n   Olivetti M-10                : ");
	(void)(system ("find " STRINGIFY(APP_LIB_DIR) " -regex \'.*/.+\\.M10$\' -printf \'\%f \' >&2")+1);

	fprintf (stderr,
		"\n\n"
		"Bootstrap Examples:\n"
		"   %1$s -b=TS-DOS.100    (no leading / or ./ takes from above)\n"
		"   %1$s -b=~/Documents/TRS-80/M100SIG/Lib-03-TELCOM/XMDPW5.100\n"
		"   %1$s -b=./rxcini.DO\n"
		"\n"
		"TPDD Server Examples:\n"
		"   %1$s\n"
		"   %1$s ttyUSB1 -p=~/Documents/wp2files -w -v\n"
		"\n"
	,args[0]);
}

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

void bz (void) {
	//dbg(6,"%s()\n",__func__);
	memset(buf,0x00,TPDD_DATA_MAX+3);
}

// make read(client_tty_fd,,) block until n bytes received
// >-1 = set new values if not already
// <0 = set default values if not already
// <-1 = refresh tcgetattr() to be more certain, then set default values if not already
void client_tty_vmin(int n) {
	if (n<-1) tcgetattr(client_tty_fd,&client_termios);
	if (n<0) {
		if (client_termios.c_cc[VTIME] == C_CC_VTIME && client_termios.c_cc[VMIN] == C_CC_VMIN) return;
		//dbg(4,"setting default vtime vmin\n");
		client_termios.c_cc[VTIME] = C_CC_VTIME;
		client_termios.c_cc[VMIN] = C_CC_VMIN;
	} else {
		if (client_termios.c_cc[VTIME] == 0 && client_termios.c_cc[VMIN] == n) return;
		//dbg(4,"setting blocking vtime vmin\n");
		client_termios.c_cc[VTIME] = 0;
		client_termios.c_cc[VMIN] = n;
	}
	tcsetattr(client_tty_fd,TCSANOW,&client_termios);
	//if (debug>3) {
	//	tcgetattr(client_tty_fd,&client_termios);
	//	dbg(4,"client_termios.c_cc[VTIME]=%u\nclient_termios.c_cc[VMIN]=%u\n",client_termios.c_cc[VTIME],client_termios.c_cc[VMIN]);
	//}
}

int write_client_tty(void *b, size_t n) {
	dbg(3,"%s()\n",__func__);
	dbg(2,"SEND: "); dbg_b(2,b,n);
	return (write(client_tty_fd,b,n));
}

// TODO - retry sanity check counter - don't rety forever
int read_client_tty(void *b, const unsigned int n) {
	unsigned t = 0;

	dbg(4,"read_client_tty(%u): ",n);

#if (READ_TTY_METHOD == 2)
	// try to force read() to block until n bytes, but also retry
	// "expected 129, got 64" still better than before.
	//dbg(3,"new method\n");
	int i;
	client_tty_vmin(n);
	while (t<n) if ((i = read(client_tty_fd, b+t, n-t))) t+=i;
	client_tty_vmin(-1);
#elif (READ_TTY_METHOD == 1)
	// force read() to block until n bytes, single read(), no polling.
	// NOT WORKING - it doesn't block completely. "expected 129, got 64"
	// perror() says "Success" :/
	//dbg(3,"new method\n");
	client_tty_vmin(n);
	if ((t = read(client_tty_fd, b, n)) != n) perror("read()");
	client_tty_vmin(-1);
#else
	// Poll, but at least read as many bytes as available each time and
	// subtract from total, instead of one byte at a time.
	// This is the reliable fallback "old" method, but even this
	// is better than it used to be thanks to the default VTIME & VMIN
	// that now makes it at least block until the first byte, so no cpu.
	//dbg(3,"old method\n");
	int i;
	while (t<n) if ((i = read(client_tty_fd, b+t, n-t))) t+=i;
#endif

	dbg_b(4,b,t);

	if (t!=n) {
		dbg(0,"\aread error, expected %u bytes, got %u\n",n,t);
		exit(1);
	}
	return (t);
}

// cat a file to terminal, for bootstrap directions
void cat(char *f) {
	char b[4097];
	int h;
	if ((h=open(f,O_RDONLY))<0) return;
	while (read(h,&b,4096)>0) printf("%s",b);
	close(h);
}

// b[] = TPDD Operation-mode return block
// b[0] = cmd
// b[1] = len (how many more bytes to read after this one, 0-128)
// b[2] to b[1+len] = 0 to 128 bytes of payload
// contents after b[1+len] are ignored
unsigned char checksum(unsigned char *b)
{
	unsigned short s=0;
	int i;

	for(i=0;i<2+b[1];i++) s+=b[i];
	return((s&0xFF)^0xFF);
}

char *pdd_to_local_fn(char *fname)
{
	dbg(3,"%s(\"%s\")\n",__func__,fname);
	int i;
	for(i=dot_offset;i>1;i--) if(fname[i-1]!=' ') break;

	if(fname[dot_offset+1]=='<' && fname[dot_offset+2]=='>') {
		fname[i]=0x00;
	} else {
		fname[i]=fname[dot_offset];
		fname[i+1]=fname[dot_offset+1];
		fname[i+2]=fname[dot_offset+2];
		fname[i+3]=0x00;
	}
	return fname;
}


// FIXME - don't do half of this stuff if (!dme_enable)
// FIXME - option not to munge the client filenames at all other than
//         to truncate to 24 bytes. No dot-offset/extension assumptions,
//         no toupper, no hiding dot-files, etc.
FILE_ENTRY *make_file_entry(char *namep, u_int32_t len, u_int8_t flags)
{
	dbg(3,"%s(\"%s\")\n",__func__,namep);
	static FILE_ENTRY f;
	int i;

	/** fill the entry */
	strncpy (f.local_fname, namep, sizeof (f.local_fname) - 1);
	dbg_b(3,(unsigned char*)f.client_fname,TPDD_FILENAME_LEN+1);
	f.len = len;


	// construct the client filename

	// 24 spaces
	memset(f.client_fname,0x20,TPDD_FILENAME_LEN);

	// find the last dot in the local filename
	for(i=strlen(namep);i>0;i--) if(namep[i]=='.') break;

	// write client extension
	if(flags&DIR_FLAG) {
		// directory - put TS-DOS DME ext on client fname
		f.client_fname[dot_offset+1]='<';
		f.client_fname[dot_offset+2]='>';
		f.len=0;
	} else {
		// file - put first 2 bytes of ext on client fname
		f.client_fname[dot_offset+1]=namep[i+1];
		f.client_fname[dot_offset+2]=namep[i+2];
	}

	dbg(5,"\"%s\"\n",f.client_fname);

	// replace ".." with "PARENT" (or whatever dme root label)
	// TODO - make this configurable, allow ".." to show through,
	// allow ordinary file or directory named "PARENT" etc.
	if(f.local_fname[0]=='.' && f.local_fname[1]=='.') {
		memcpy (f.client_fname, DEFAULT_DME_PARENT_LABEL, 6);
	} else {
		for(i=0;i<dot_offset && i<strlen(namep) && namep[i]; i++) {
			if(namep[i]=='.') break;
			f.client_fname[i]=namep[i];
		}
	}

	dbg(5,"\"%s\"\n",f.client_fname);

	f.client_fname[dot_offset]='.';

	dbg(5,"\"%s\"\n",f.client_fname);

	f.client_fname[dot_offset+3]=0;

	dbg(5,"\"%s\"\n",f.client_fname);

	if(upcase) for(i=0;i<TPDD_FILENAME_LEN;i++) f.client_fname[i]=toupper(f.client_fname[i]);

	dbg(5,"\"%s\"\n",f.client_fname);

	f.flags=flags;

	dbg(4," local: \"%s\"\n",f.local_fname);
	dbg(4,"client: \"%s\"\n",f.local_fname);
	dbg(4,"   len: %d\n",f.len);

	return &f;
}

int read_next_dirent(DIR *dir)
{
	dbg(3,"%s()\n",__func__);
	struct stat st;
	struct dirent *dire;
	int flags;

	if (dir == NULL) {
		printf ("%s:%u\n", __FUNCTION__, __LINE__);
		dire=NULL;
		ret_std (ERR_NO_DISK);
		return(0);
	}

	while ((dire=readdir(dir)) != NULL) {
		flags=0;

		if (stat(dire->d_name,&st)) {
			ret_std(DIRENT_GET_FIRST);
			return 0;
		}

		if (S_ISDIR(st.st_mode)) flags=DIR_FLAG;
		else if (!S_ISREG (st.st_mode)) continue;

		if (dire->d_name[0]=='.') continue; // skip "." ".." and hidden files
		//if (dire->d_name[0]=='#') continue; // skip "#"

		if (strlen(dire->d_name)>LOCAL_FILENAME_MAX) continue; // skip long filenames

		/* add file to list so we can traverse any order */
		add_file (make_file_entry(dire->d_name, st.st_size, flags));

		break;
	}

	if (dire == NULL) return 0;

	return 1;
}

void update_file_list()
{
	dbg(3,"%s()\n",__func__);
	DIR * dir;

	dir=opendir(".");
	/** rebuild the file list */
	file_list_clear_all();
	if(dir_depth) add_file (make_file_entry("..", 0, DIR_FLAG));
	while (read_next_dirent (dir));

	closedir(dir);
}

/* TPDD operations */

// standard return - return for: error open close delete status write
void ret_std(unsigned char err)
{
	dbg(2,"%s()\n",__func__);
	buf[0]=RET_STD;
	buf[1]=0x01;
	buf[2]=err;
	buf[3]=checksum(buf);
	dbg(3,"Response: %02X\n",err);
	write_client_tty(buf,4);
	if (buf[2]!=ERR_SUCCESS) dbg(2,"ERROR RESPONSE TO CLIENT");
}

// return for dirent
int ret_dirent(FILE_ENTRY *ep)
{
	dbg(2,"%s(\"%s\")\n",__func__,ep->client_fname);
	unsigned short size;
	int i;

	bz();
	buf[0]=RET_DIRENT;
	buf[1]=LEN_RET_DIRENT;

	if (ep && ep->client_fname) {

		// name
		memset (buf + 2, ' ', TPDD_FILENAME_LEN);
		for(i=0;i<dot_offset+3;i++)
			buf[i+2]=(ep->client_fname[i])?ep->client_fname[i]:' ';
		//memcpy (buf + 2, ep->client_fname, dot_offset+2);

		// attrib
		buf[26] = DEFAULT_TPDD_FILE_ATTRIB;

		// size
		size = htons (ep->len);
		memcpy (buf + 27, &size, 2);
	}

	dbg(3,"\"%24.24s\"\n",buf+2);

	buf[29] = TPDD_FREE_SECTORS;
	buf[30] = checksum (buf);

	return (write_client_tty(buf,31) == 31);
}

// REQ_DIRENT 
// b[0]-b[23] = filename
// b[24] = attrib
// b[25] = search
/*
 * heads-up
 * TS-DOS sometimes submits request with junk in the filename & attrib fields
 * in some cases where a real drive would ignore them (get-first/get-next).
 * So only look at those fields for the set-name case.
*/ 
int req_dirent(unsigned char *data)
{
	dbg(2,"%s()\n",__func__);
	dbg(5,"data[]\n"); dbg_b(5,data,-1);
	dbg_p(4,data);

	char *p;
	char filename[TPDD_FILENAME_LEN+1] = { 0x00 };

	switch (data[27]) {
	case DIRENT_SET_NAME:	/* set filename for subsequent actions */
		dbg(3,"DIRENT_SET_NAME\n");
		if (data[2]) {
			dbg(3,"filename: \"%24.24s\"\n",data+2);
			dbg(3,"  attrib: \"%c\" (%1$02X)\n",data[26]);
		}
		strncpy(filename,(char *)data+2,TPDD_FILENAME_LEN);
		filename[TPDD_FILENAME_LEN]=0;
		/* Remove trailing spaces */
		for (p = strrchr(filename,' '); p >= filename && *p == ' '; p--) *p = 0;
		cur_file=find_file(filename);
		if (cur_file) { 
			fprintf (stderr, "Found: \"%s\"  %u\n", cur_file->local_fname, cur_file->len);
			ret_dirent(cur_file);

		} else {
			//	  strncpy(cur_file->client_fname, filename, TPDD_FILENAME_LEN);
			fprintf (stderr, "Not found\n");
			ret_dirent(NULL);
			//			  empty_dirent();
			if (filename[dot_offset+1]=='<' && filename[dot_offset+2]=='>') {
				cur_file=make_file_entry(pdd_to_local_fn(filename), 0, DIR_FLAG);
			} else {
				cur_file=make_file_entry(pdd_to_local_fn(filename), 0, 0);
			}
		}
		break;
	case DIRENT_GET_FIRST:
		dbg(3,"DIRENT_GET_FIRST\n");
		if(debug==1) dbg(1,"directory listing\n");
		update_file_list();
		ret_dirent(get_first_file());
		break;
	case DIRENT_GET_NEXT:
		dbg(3,"DIRENT_GET_NEXT\n");
		ret_dirent(get_next_file());
		break;
	case DIRENT_GET_PREV:
		dbg(3,"DIRENT_GET_PREV\n");
		ret_dirent(get_prev_file());
		break;
	case DIRENT_CLOSE:
		dbg(3,"DIRENT_CLOSE\n");
		// file_list_clear_all ();
		break;
	}
	return 0;
}

// update dme_cwd with a 6-byte truncated / space-padded working dir
void update_dme_cwd()
{
	dbg(2,"%s()\n",__func__);
	int i;
	if(dir_depth) {
		char dirbuf[1024];
		int j;

		if(getcwd(dirbuf, 1024) ) {
			memset(dme_cwd,0x20,6);
			for(i=strlen(dirbuf); i>=0 ; i--) if(dirbuf[i]=='/') break;
			for(j=0; j<6 && dirbuf[i+j+1] && dirbuf[i+j+1]!='.'; j++) dme_cwd[j]=dirbuf[i+j+1];
		}
	} else {
		memcpy(dme_cwd,dme_root_label,6);
	}
}

// TS-DOS DME return
// Construct a DME packet around dme_cwd and send it to the client
void ret_dme_cwd()
{
	dbg(2,"%s(\"%s\")\n",__func__,dme_cwd);
	buf[0]=RET_STD;
	buf[1]=LEN_RET_DME;
	buf[2]=0x00;
	memcpy(buf+3,dme_cwd,6);
	buf[9]='.';
	buf[10]='<';
	buf[11]='>';
	buf[12]=0x20;
	buf[13]=checksum(buf);
	dbg(3,"Setting TS-DOS CWD: \"%6.6s\"\n",buf+3);
	write_client_tty(buf,14);
}

// b[0] = fmt  0x01
// b[1] = len  0x01
// b[2] = mode 0x01 write new
//             0x02 write append
//             0x03 read
// b[3] = ck
int req_open(unsigned char *data)
{
	dbg(2,"%s(\"%s\")\n",__func__,cur_file->local_fname);
	dbg(5,"data[]\n"); dbg_b(5,data,-1);
	dbg_p(4,data);

	unsigned char omode = data[2];

	switch(omode) {
	case F_OPEN_WRITE:
		dbg(3,"mode: write\n");
		if (o_file_h >= 0) {
			close (o_file_h);
			o_file_h=-1;
		}
		if(cur_file->flags&DIR_FLAG) {
			if(mkdir(cur_file->local_fname,0775)==0) {
				ret_std(ERR_SUCCESS);
			} else {
				ret_std(ERR_FMT_MISMATCH);
			}
		} else {
			o_file_h = open (cur_file->local_fname,O_CREAT|O_TRUNC|O_WRONLY|O_EXCL,0666);
			if(o_file_h<0)
				ret_std(ERR_FMT_MISMATCH);
			else {
				f_open_mode=omode;
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
		if(cur_file==0) {
			ret_std(ERR_FMT_MISMATCH);
			return -1;
		}
		o_file_h = open (cur_file->local_fname, O_WRONLY | O_APPEND);
		if (o_file_h < 0)
			ret_std(ERR_FMT_MISMATCH);
		else {
			f_open_mode=omode;
			ret_std (ERR_SUCCESS);
		}
		break;
	case F_OPEN_READ:
		dbg(3,"mode: read\n");
		if (o_file_h >= 0) {
			close (o_file_h);
			o_file_h=-1;
		}
		if(cur_file==0) {
			ret_std (ERR_NO_FILE);
			return -1;
		}
		
		if(cur_file->flags&DIR_FLAG) {
			int err=0;
			// directory
			if(cur_file->local_fname[0]=='.' && cur_file->local_fname[1]=='.') {
				// parent dir
				if(dir_depth>0) {
					err=chdir (cur_file->local_fname);
					if(!err) dir_depth--;
				}
			} else {
				// enter dir
				err=chdir(cur_file->local_fname);
				dir_depth++;
			}
			update_dme_cwd();
			if(err) ret_std (ERR_FMT_MISMATCH);
			else ret_std (ERR_SUCCESS);
		} else {
			// regular file
			o_file_h = open (cur_file->local_fname, O_RDONLY);
			if(o_file_h<0)
				ret_std (ERR_NO_FILE);
			else {
				f_open_mode = omode;
				ret_std (ERR_SUCCESS);
			}
		}
		break;
	}
	return (o_file_h);
}

// b[0] = 0x03
// b[1] = 0x00
// b[2] = ck
void req_read(void)
{
	dbg(2,"%s()\n",__func__);
	int i;

	buf[0]=RET_READ;
	if(o_file_h<0) {
		ret_std(ERR_CMDSEQ);
		return;
	}
	if(f_open_mode!=F_OPEN_READ) {
		ret_std(ERR_FMT_MISMATCH);
		return;
	}

	i = read (o_file_h, buf+2, TPDD_DATA_MAX);

	buf[1] = (unsigned char) i;
	buf[2+i] = checksum(buf);

	dbg(4,"...OUT going packet TO client...\n");
	dbg(5,"buf[]\n"); dbg_b(5,buf,-1);
	dbg_p(4,buf);
	dbg(4,"................................\n");

	write_client_tty(buf, 3+i);
}

// b[0] = 0x04
// b[1] = 0x01 - 0x80
// b[2] = b[1] bytes
// b[2+len] = ck
void req_write(unsigned char *data)
{
	dbg(2,"%s()\n",__func__);
	dbg(4,"...IN coming packet FROM client...\n");
	dbg(5,"data[]\n"); dbg_b(5,data,-1);
	dbg_p(4,data);
	dbg(4,"..................................\n");

	if(o_file_h<0) {
		ret_std(ERR_CMDSEQ);
		return;
	}
	if(f_open_mode!=F_OPEN_WRITE && f_open_mode !=F_OPEN_APPEND) {
		ret_std(ERR_FMT_MISMATCH);
		return;
	}
	if(write (o_file_h,data+2,data[1]) != data[1])
		ret_std (ERR_SECTOR_NUM);
	else
		ret_std (ERR_SUCCESS);
}

void req_delete(void)
{
	dbg(2,"%s()\n",__func__);
	if(cur_file->flags&DIR_FLAG)
		rmdir(cur_file->local_fname);
	else 
		unlink (cur_file->local_fname);
	update_file_list();
	ret_std (ERR_SUCCESS);
}

// TPDD2 sector cache write - but not really doing that
// This is just something TS-DOS does to detect TPDD2, and we do implement
// other TPDD2 features, so we respond to this just enough to satisfy TS-DOS.
// We just blindly return a packet that means "cache write suceeded".
// http://bitchin100.com/wiki/index.php?title=TPDD-2_Sector_Access_Protocol
// https://github.com/bkw777/pdd.sh/blob/41053c21f6f2ee349db2abf51547117de0a51b59/pdd.sh#L1637
void ret_cache_write() {
	dbg(3,"%s()\n",__func__);
	buf[0]=RET_CACHE_STD;
	buf[1]=0x01;
	buf[2]=ERR_SUCCESS;
	buf[3]=checksum(buf);
	write_client_tty(buf,4);
}

// Another part of TS-DOS's drive/server capabilities detection scheme.
// Used to be called "TS-DOS mystery command 2", but now it's the only one.
// ("mystery command 1" was the TPDD2 sector cache command above)
void ret_tsdos_mystery() {
	dbg(3,"%s()\n",__func__);
	static unsigned char canned[] = {RET_TSDOS_MYSTERY, 0x0F, 0x41, 0x10, 0x01, 0x00, 0x50, 0x05, 0x00, 0x02, 0x00, 0x28, 0x00, 0xE1, 0x00, 0x00, 0x00};
	memcpy(buf, canned, canned[1]+2);
	buf[canned[1]+2] = checksum(buf);
	write_client_tty(buf, buf[1]+3);
}

void req_rename(unsigned char *data)
{
	dbg(3,"%s()\n",__func__);
	char *new_name = (char *)data + 4;

	new_name[TPDD_FILENAME_LEN]=0;

	if (rename (cur_file->local_fname, pdd_to_local_fn(new_name)))
		ret_std(ERR_SECTOR_NUM);
	else
		ret_std(ERR_SUCCESS);
}

void dispatch_opr_cmd(unsigned char *data)
{
	dbg(3,"%s()\n",__func__);
	dbg_p(3,data);
	dbg(5,"data[]\n"); dbg_b(5,data,-1);

	switch(data[0]) {
	case REQ_DIRENT:
		req_dirent(data);
		break;
	case REQ_OPEN:
		req_open(data);
		break;
	case REQ_CLOSE:
		if(o_file_h>=0) close(o_file_h);
		o_file_h = -1;
		ret_std(ERR_SUCCESS);
		break;
	case REQ_READ:
		req_read();
		break;
	case REQ_WRITE:
		req_write(data);
		break;
	case REQ_DELETE:
		req_delete();
		break;
	case REQ_FORMAT:
		ret_std(ERR_SUCCESS);
		break;
	case REQ_STATUS:
		ret_std(ERR_SUCCESS);
		break;
	case REQ_FDC:	/* switch to FDC-mode - part of TS-DOS DME */
		ret_dme_cwd();
		break;
	case REQ_CONDITION: // TPDD2
		ret_std(ERR_SUCCESS);
		break;
	case REQ_RENAME: // TPDD2
		req_rename(data);
		update_file_list ();
		break;
	case REQ_TSDOS_MYSTERY:  /* TS-DOS mystery command 2 */
		ret_tsdos_mystery(); /* part of TS-DOS drive/server detection */
		break;
	case REQ_CACHE_WRITE:  /* formerly TS-DOS "mystery command 1" */
		ret_cache_write(); /* part of TS-DOS detection of TPDD2 */
		break;
	default:
		return;
		break;
	}

	if(data[0]!=REQ_STATUS && data[0]!=REQ_FDC) m1rec=0;

	return;
}

int get_opr_cmd(void)
{
	dbg(3,"%s()\n",__func__);
	unsigned char b[TPDD_DATA_MAX+3] = { 0x00 };
	unsigned i = 0;
	bz();

// both of these work
#if (ZZ_SCAN_METHOD == 1)
	// collect command
	while (read_client_tty(&b,1) == 1) {
		if (b[0]==0x5A) i++; else { i=0; b[0]=0x00; continue; }
		if (i<2) { b[0]=0x00; continue; }
		if ((read_client_tty(&b,2) == 2) && (read_client_tty(&b[2],b[1]+1) == b[1]+1)) break;
		i=0; memset(b,0x00,TPDD_DATA_MAX+3);
	}
#else
	// collect command
	while (read_client_tty(&b,1) == 1) {
		if (i==2) { // have 2 Z's else skip
			i=0; // ensure if any of the following fail, start over
			if (read_client_tty(&b[1],1) != 1) continue; // read len
			if (b[1]>TPDD_DATA_MAX) continue; // len is sane
			if (read_client_tty(&b[2],b[1]+1) == b[1]+1) break; // read payload+checksum & done
		}
		if (b[0]==0x5A) i++; else i=0; // current byte is Z else start over
	}
#endif

	// debug
	dbg_p(3,b);

	// checksum else abort
	i = checksum(b);
	if (b[b[1]+2]!=i) {
		dbg(0,"Failed checksum: received: %02X  calculated: %02X\n",b[b[1]+2],i);
		ret_std(ERR_PARAM);
		return(7);
	}

	// dispatch
	dispatch_opr_cmd(b);
	return 0;
}

int send_BASIC(char *f)
{
	int w=0;
	int i=0;
	int fd;
	int byte_usleep = BASIC_byte_msec*1000;
	unsigned char b;

	if ((fd=open(f,O_RDONLY))<0) {
		if (debug) fprintf(stderr, "Failed to open %s for read.\n",f);
		return(9);
	}

	if(debug) {
		fprintf(stderr, "Sending %s\n",f);
		fflush(stdout);
	}

	while(read(fd,&b,1)==1) {
		while((i=write_client_tty(&b,1))!=1);
		w+=i;
		usleep(byte_usleep);
		if (debug) fprintf(stderr, "Sent: %d bytes\n",w);
		else fprintf(stderr, ".");
		fflush(stdout);
	}
	fprintf(stderr, "\n");
	b = BASIC_EOF;
	write_client_tty(&b,1);
	close(fd);
	close(client_tty_fd);

	if (debug) {
		fprintf(stderr, "Sent %s\n",f);
		fflush(stdout);
	}
	else
		fprintf(stderr, "\n");

	return(0);
}

int bootstrap(char *f)
{
	int r = 0;
	char loader_file[PATH_MAX]="";
	char pre_install_txt_file[PATH_MAX]="";
	char post_install_txt_file[PATH_MAX]="";

	if (f[0]=='~'&&f[1]=='/') {
		strcpy(loader_file,getenv("HOME"));
		strcat(loader_file,f+1);
	}

	if ((f[0]=='/')||(f[0]=='.'&&f[1]=='/'))
		strcpy(loader_file,f);

	if(loader_file[0]==0) {
		strcpy(loader_file,STRINGIFY(APP_LIB_DIR));
		strcat(loader_file,"/");
		strcat(loader_file,f);
	}

	strcpy(pre_install_txt_file,loader_file);
	strcat(pre_install_txt_file,".pre-install.txt");

	strcpy(post_install_txt_file,loader_file);
	strcat(post_install_txt_file,".post-install.txt");

	printf("Bootstrap: Installing %s\n", loader_file);

	if(access(loader_file,F_OK)==-1) {
		if(debug) fprintf(stderr, "Not found.\n");
		return(1);
	}

	if(access(pre_install_txt_file,F_OK)>=0) {
		cat(pre_install_txt_file);
	} else {
		printf("Prepare the portable to receive. Hints:\n");
		printf("\tRUN \"COM:98N1ENN\"\t(for TANDY, Kyotronic, Olivetti)\n");
		printf("\tRUN \"COM:9N81XN\"\t(for NEC)\n");
		printf("\n");
	}

	printf("Press [Enter] when ready...");
	getchar();

	if ((r=send_BASIC(loader_file))!=0)
		return(r);

	cat(post_install_txt_file);

	printf("\n\n\"%s -b\" will now exit.\n",args[0]);
	printf("Re-run \"%s\" (without -b this time) to run the TPDD server.\n",args[0]);
	printf("\n");

	return(0);
}

int main(int argc, char **argv)
{
	int off=0;
	unsigned char client_tty_name[PATH_MAX];
	char bootstrap_file[PATH_MAX];
	int arg;

	/* create the file list (for reverse order traversal) */
	file_list_init ();

	args = argv;

	strcpy ((char *)client_tty_name,STRINGIFY(DEFAULT_CLIENT_TTY));
	if (client_tty_name[0]!='/') {
		strcpy((char *)client_tty_name,"/dev/");
		strcat((char *)client_tty_name,(char *)STRINGIFY(DEFAULT_CLIENT_TTY));
	}

	for (arg = 1; arg < argc; arg++) {
		switch (argv[arg][0]) {
		case '/':
			strcpy ((char *)client_tty_name, (char *)(argv[arg]));
			break;
		case '-':
			switch (argv [arg][1]) {
			case 0:
				strcpy ((char *)client_tty_name,"/dev/tty");
				client_tty_fd = 1;
				break;
			case 'g':
				getty_mode = true;
				break;
			case 'u':
				upcase = true;
				break;
			case 'c':
				rtscts = true;
				break;
			case 'v':
				debug++;
				break;
			case 'p':
				if (argv[arg][2] == '=')
					(void)(chdir (argv[arg] + 3)+1);
				break;
			case 'w':
				dot_offset = 8;
				break;
			case 'h':
				print_usage();
				exit(0);
				break;
			case 'b':
				bootstrap_mode = true;
				strcpy (bootstrap_file,STRINGIFY(DEFAULT_CLIENT_APP) "." STRINGIFY(DEFAULT_CLIENT_MODEL));
				if (argv[arg][2] == '=') strcpy (bootstrap_file,(char *)(argv[arg]+3));
				break;
			case 'z':
				if (argv[arg][2] == '=') BASIC_byte_msec = atoi(argv[arg]+3);
				break;
			default:
				fprintf(stderr, "Unknown option %s\n",argv[arg]);
				print_usage();
				exit(1);
				break;
			}
			break;
		default:
			strcpy((char *)client_tty_name,"/dev/");
			strcat((char *)client_tty_name,(char *)(argv[arg]));
		}
	}

	if (getty_mode)
		debug = 0;

	if (debug) {
		fprintf (stderr, "DeskLink+ " STRINGIFY(APP_VERSION) "\n");
		fprintf (stderr, "Using Serial Device: %s\n", client_tty_name);
	}

// Also, what about O_NOCTTY ?
	if(client_tty_fd<0)
		client_tty_fd=open((char *)client_tty_name,O_RDWR
#if (IGNORE_DSR == 1)
			,O_NONBLOCK
#endif
		);

	if(client_tty_fd<0) {
		fprintf (stderr,"Can't open \"%s\"\n",client_tty_name);
		return(1);
	}


	if (debug) {
		if(!bootstrap_mode) {
			fprintf (stderr, "Working In Directory: ");
			fprintf (stderr, "--------------------------------------------------------------------------------\n");
			(void)(system ("pwd >&2;ls -l >&2")+1);
			fprintf (stderr, "--------------------------------------------------------------------------------\n");
		}
	}

	// getty mode
	if(getty_mode) {
		if(login_tty(client_tty_fd)==0) client_tty_fd = STDIN_FILENO;
		else (void)(daemon(1,1)+1);
	}

	// serial line setup
	(void)(tcflush(client_tty_fd, TCIOFLUSH)+1);
	ioctl(client_tty_fd, FIONBIO, &off);
	ioctl(client_tty_fd, FIOASYNC, &off);
	if(tcgetattr(client_tty_fd,&client_termios)==-1) return(21);
	cfmakeraw(&client_termios);
	client_termios.c_cflag |= CLOCAL|CS8;
	if(rtscts) client_termios.c_cflag |= CRTSCTS;
	else client_termios.c_cflag &= ~CRTSCTS;
	if(cfsetspeed(&client_termios,client_baud)==-1) return(22);
	if(tcsetattr(client_tty_fd,TCSANOW,&client_termios)==-1) return(23);
	client_tty_vmin(-2);

	// send loader and exit
	if(bootstrap_mode) return(bootstrap(bootstrap_file));

	// process commands forever
	while(1) get_opr_cmd();

	return(0);
}
