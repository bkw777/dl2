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
Extensions and enhancements Copyright (C) 2005 John R. Hogerhuis
20191226 Brian K. White - repackaging, reorganizing, bootstrap function
         Kurt McCullum - TS-DOS loaders

Copyright (c) 2022 Gabriele Gorla
Add support for subdirectories, general cleanup.

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

#define DEFAULT_BOOTSTRAP_BYTE_MSEC 6

#define BASIC_EOF 0x1A

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

#define FREE_BLOCKS 157

#define ST_OK                  0x00
#define ST_FILE_DOES_NOT_EXIST 0x10
#define ST_FILE_EXIST          0x11
#define ST_NO_FILENAME         0x30
#define ST_DIR_SEARCH_ERROR    0x31
#define ST_BANK_ERROR          0x35
#define ST_PARAMETER_ERROR     0x36
#define ST_OPEN_FRMT_MISMATCH  0x37
#define ST_EOF                 0x3f
#define ST_NO_START_MARK       0x40
#define ST_ID_CRC_ERROR        0x41
#define ST_SECTOR_LEN_ERROR    0x42
#define ST_FRMT_VERIFY_ERROR   0x44
#define ST_FRMT_INTERRUPTION   0x46
#define ST_ERASE_OFFSET_ERROR  0x47
#define ST_DATA_CRC_ERROR      0x49
#define ST_SECTOR_NUMBER_ERROR 0x4a
#define ST_READ_DATA_TIMEOUT   0x4b
#define ST_SECTOR_NUMBER_ERR2  0x4d // ???
#define ST_DISK_WRITE_PROTECT  0x50
#define ST_UNINITIALIZED_DISK  0x5e
#define ST_DIRECTORY_FULL      0x60
#define ST_DISK_FULL           0x61
#define ST_FILE_TOO_LONG       0x6e
#define ST_NO_DISK             0x70
#define ST_DISK_CHANGE_ERROR   0x71

// configuration
bool debug = false;
bool upcase = false;
bool rtscts = false;
unsigned dot_offset = 6; // 6 for 100/102/200/NEC/K85/M10 , 8 for WP-2
int client_baud = DEFAULT_CLIENT_BAUD;
int bootstrap_byte_msec = DEFAULT_BOOTSTRAP_BYTE_MSEC;

// state
bool getty_mode = false;
bool bootstrap_mode = false;

// globals
bool m1rec = false;

int file = -1;
int mode = 0;	/* 0=unopened, 1=Write, 3=Read, 2=Append */
int client_fd = -1; // client tty file handle

char **args;
struct termios origt;
struct termios ti;

unsigned char buf[131];

FILE_ENTRY *cur_file;
int dir_depth=0;

int be_disk(void);

void out_buf(unsigned char *bufp, unsigned len);

int bootstrap(char *f);

int send_installer(char *f);

void print_usage()
{
	fprintf (stderr, "DeskLink++ " STRINGIFY(APP_VERSION) " usage:\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "%s [tty_device] [options]\n",args[0]);
	fprintf (stderr, "\n");
	fprintf (stderr, "tty_device:\n");
	fprintf (stderr, "    Serial device the client is connected to\n");
	fprintf (stderr, "    examples: ttyS0, ttyUSB0, /dev/pts/foo4, etc...\n");
	fprintf (stderr, "    default = " STRINGIFY(DEFAULT_CLIENT_TTY) "\n");
	fprintf (stderr, "    \"-\" = stdin/stdout (/dev/tty)\n"); // 20191227 bkw - could this be used with inetd to make a network tpdd server?
	fprintf (stderr, "\n");
	fprintf (stderr, "options:\n");
	fprintf (stderr, "   -h       Print this help\n");
	fprintf (stderr, "   -b=file  Bootstrap: Install <file> onto the portable\n");
	fprintf (stderr, "   -v       Verbose/debug mode\n");
	fprintf (stderr, "   -g       Getty mode. Run as daemon\n");
	fprintf (stderr, "   -p=dir   Path to files to be served, default is \".\"\n");
	fprintf (stderr, "   -w       WP-2 compatibility mode (8.2 filenames)\n");
	fprintf (stderr, "   -u       Uppercase all filenames\n");
	fprintf (stderr, "   -c       Hardware flow control (RTS/CTS)\n");
	fprintf (stderr, "   -z=#     Sleep # milliseconds between each byte while sending bootstrap file (default " STRINGIFY(DEFAULT_BOOTSTRAP_BYTE_MSEC) ")\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "available bootstrap files:\n");
	// blargh ...
	//(void)(system ("find " STRINGIFY(APP_LIB_DIR) " -regex \'.*/.+\\.\\(100\\|200\\|NEC\\|M10\\|K85\\)$\' -printf \'\%f\\n\' >&2")+1);
	// more blargh...  (using system("find...") just to get some filenames)
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
	fprintf (stderr, "\n\n");
	fprintf (stderr, "Bootstrap Examples:\n");
	fprintf (stderr, "   %s -b=TS-DOS.100\n",args[0]);
	fprintf (stderr, "   %s -b=~/Documents/TRS-80/M100SIG/Lib-03-TELCOM/XMDPW5.100\n",args[0]);
	fprintf (stderr, "   %s -b=./rxcini.DO\n",args[0]);
	fprintf (stderr, "\n");
	fprintf (stderr, "TPDD Server Examples:\n");
	fprintf (stderr, "   %s\n",args[0]);
	fprintf (stderr, "   %s ttyUSB1 -p=~/Documents/wp2files -w -v\n",args[0]);
	fprintf (stderr, "\n");
}

void cat(char *f)
{
	int h = -1;
	char b[4097];
	
	if((h=open(f,O_RDONLY))<0)
		return;
	while(read(h,&b,4096)>0)
		printf("%s",b);
	close(h);
}

int bootstrap(char *f)
{
	int r = 0;
	char installer_file[PATH_MAX]="";
	char pre_install_txt_file[PATH_MAX]="";
	char post_install_txt_file[PATH_MAX]="";
	
	if (f[0]=='~'&&f[1]=='/') {
		strcpy(installer_file,getenv("HOME"));
		strcat(installer_file,f+1);
	}
	
	if ((f[0]=='/')||(f[0]=='.'&&f[1]=='/'))
		strcpy(installer_file,f);
	
	if(installer_file[0]==0) {
		strcpy(installer_file,STRINGIFY(APP_LIB_DIR));
		strcat(installer_file,"/");
		strcat(installer_file,f);
	}
	
	strcpy(pre_install_txt_file,installer_file);
	strcat(pre_install_txt_file,".pre-install.txt");
	
	strcpy(post_install_txt_file,installer_file);
	strcat(post_install_txt_file,".post-install.txt");
	
	printf("Bootstrap: Installing %s\n", installer_file);
	
	if(access(installer_file,F_OK)==-1) {
		if(debug)
			fprintf(stderr, "Not found.\n");
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
	
	if ((r=send_installer(installer_file))!=0)
		return(r);
	
	cat(post_install_txt_file);
	
	printf("\n\n\"%s -b\" will now exit.\n",args[0]);
	printf("Re-run \"%s\" (without -b this time) to run the TPDD server.\n",args[0]);
	printf("\n");
	
	return(0);
}

int my_write (int fh, void *srcp, size_t len)
{
	if (debug) {
		fprintf (stderr, "SEND: ");
		out_buf (srcp, len);
	}
	return (write (fh, srcp, len));
}

unsigned char checksum(unsigned char *data)
{
	unsigned short sum=0;
	int len=data[1]+2;
	int i;
	
	for(i=0;i<len;i++)
		sum+=data[i];
	return((sum & 0xFF) ^ 0xFF);
}

void normal_return(unsigned char type)
{
	buf[0]=0x12;
	buf[1]=0x01;
	buf[2]=type;
	buf[3]=checksum(buf);
	my_write(client_fd,buf,4);
	if(debug)
		fprintf(stderr, "Response: %02X\n",type);
}

int main(int argc, char **argv)
{
	int off=0;
	unsigned char client_tty[PATH_MAX];
	char bootstrap_file[PATH_MAX];
	int arg;

	/* create the file list (for reverse order traversal) */
	file_list_init ();

	args = argv;

	strcpy ((char *)client_tty,STRINGIFY(DEFAULT_CLIENT_TTY));
	if (client_tty[0]!='/') {
		strcpy((char *)client_tty,"/dev/");
		strcat((char *)client_tty,(char *)STRINGIFY(DEFAULT_CLIENT_TTY));
	}
	
	for (arg = 1; arg < argc; arg++) {
		switch (argv[arg][0]) {
		case '/':
			strcpy ((char *)client_tty, (char *)(argv[arg]));
			break;
		case '-':
			switch (argv [arg][1]) {
			case 0:
				strcpy ((char *)client_tty,"/dev/tty");
				client_fd = 1;
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
				debug = true;
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
				if (argv[arg][2] == '=')
					strcpy (bootstrap_file,(char *)(argv[arg]+3));
				break;
			case 'z':
				if (argv[arg][2] == '=')
					bootstrap_byte_msec = atoi(argv[arg]+3);
				break;
			default:
				fprintf(stderr, "Unknown option %s\n",argv[arg]);
				print_usage();
				exit(1);
				break;
			}
			break;
		default:
			strcpy((char *)client_tty,"/dev/");
			strcat((char *)client_tty,(char *)(argv[arg]));
		}
	}
	
	if (getty_mode)
		debug = 0;
	
	if (debug) {
		fprintf (stderr, "DeskLink+ " STRINGIFY(APP_VERSION) "\n");
		fprintf (stderr, "Using Serial Device: %s\n", client_tty);
		if(!bootstrap_mode) {
			fprintf (stderr, "Working In Directory: ");
			fprintf (stderr, "--------------------------------------------------------------------------------\n");
			(void)(system ("pwd >&2;ls -l >&2")+1);
			fprintf (stderr, "--------------------------------------------------------------------------------\n");
		}
	}
	
	if(client_fd<0)
		client_fd=open((char *)client_tty,O_RDWR|O_NONBLOCK);
	if(client_fd<0) {
		fprintf (stderr,"Error: open(%s,...)=%d\n",client_tty,client_fd);
		return(1);
	}
	
	if(getty_mode) {
		if(login_tty(client_fd)==0)
			client_fd = STDIN_FILENO;
		else
			(void)(daemon(1,1)+1);
	}

	(void)(tcflush(client_fd, TCIOFLUSH)+1);	/* clear out the crap */
	ioctl(client_fd, FIONBIO, &off);	/* turn off non-blocking mode */
	ioctl(client_fd, FIOASYNC, &off);	/* ditto for async mode */
	
	if(tcgetattr(client_fd,&ti)==-1)
		return(1);
	cfmakeraw(&ti);
	ti.c_cflag |= CLOCAL|CS8;
	if(rtscts) {
		ti.c_cflag |= CRTSCTS;
	} else {
		ti.c_cflag &= ~CRTSCTS;
	}
	if(cfsetspeed(&ti,client_baud)==-1)
		return(1);
	if(tcsetattr(client_fd,TCSANOW,&ti)==-1)
		return(1);

	/* Set up terminal termios struct */
	origt.c_iflag=BRKINT | ICRNL | IMAXBEL | IXON | IXANY | IXOFF;
	origt.c_oflag=OPOST | ONLCR;
	origt.c_lflag=ECHO | ICANON | ISIG | IEXTEN | ECHOE | ECHOKE | ECHOCTL;
	origt.c_cflag=CREAD | CS8 | HUPCL;
	if(rtscts) {
		origt.c_cflag |=
#ifdef CRTS_OFLOW
			CRTS_OFLOW
#else
			CRTSCTS
#endif
			;
#ifdef CRTS_IFLOW
		origt.c_cflag |= CRTS_IFLOW ;
#endif
	} else {
		origt.c_cflag &= ~CRTSCTS;
	}
	
	cfsetspeed(&origt,client_baud);
	
	if(bootstrap_mode)
		exit(bootstrap(bootstrap_file));
	
	while(1)
		be_disk();

	file_list_cleanup ();

	return(0);
}

int out_dirent (FILE_ENTRY *ep)
{
	unsigned short size;
	int i;
	
	bzero(buf,31);
	buf[0]=0x11;
	buf[1]=0x1C;
	
	if (debug) fprintf (stderr, "out_dirent: %s\n", ep->tsname);
	
	/* format the filename */
	if (ep && ep->tsname) {
		
		buf[26] = 'F';
		size = htons (ep->len);
		memcpy (buf + 27, &size, 2);
		memset (buf + 2, ' ', 24);
		
		for(i=0;i<dot_offset+3;i++)
			buf[i+2]=(ep->tsname[i])?ep->tsname[i]:' ';
		//memcpy (buf + 2, ep->tsname, dot_offset+2);
		
		//		if (debug) fprintf (stderr, "str   : %24.24s\n", (char *)buf + 2);
	}
	
	buf[29]=FREE_BLOCKS;
	/* add checksum */
	buf[30] = checksum (buf);
	
	/* write packet */
	return (my_write (client_fd,buf,31) == 31);
}

FILE_ENTRY *make_file_entry(char *namep, u_int32_t len, u_int8_t flags)
{
	static FILE_ENTRY f;
	int i;
	
	/** fill the entry */
	strncpy (f.ufname, namep, sizeof (f.ufname) - 1);
	
	for(i=0;i<12;i++) f.tsname[i]=' ';
	
	f.len = len;
	
	// fix the filename
	for(i=strlen(namep);i>0;i--)
		if(namep[i]=='.') break;
	
	if(flags&DIR_FLAG) {
		// directory
		f.tsname[dot_offset+1]='<';
		f.tsname[dot_offset+2]='>';
		f.len=0;
	} else {
		if(i>0) {
			// found an extension
			// for the time being just copy
			f.tsname[dot_offset+1]=toupper(namep[i+1]);
			f.tsname[dot_offset+2]=toupper(namep[i+2]);
		} else {
			// no extension - default to .DO
			f.tsname[dot_offset+1]='D';
			f.tsname[dot_offset+2]='O';
		}
	}
	
	if(f.ufname[0]=='.' && f.ufname[1]=='.') {
		memcpy (f.tsname, "PARENT", 6);
	} else {
		for(i=0;i<dot_offset && i<strlen(namep) && namep[i]; i++) {
			if(namep[i]=='.') break;
			f.tsname[i]=namep[i];
		}
	}
	
	f.tsname[dot_offset]='.';
	f.tsname[dot_offset+3]=0;
	
	if(upcase) for(i=0;i<12;i++) f.tsname[i]=toupper(f.tsname[i]);
	
	f.flags=flags;
	
	fprintf(stderr,"unix file: %s ts-dos: %s  len:%d\n",f.ufname, f.tsname, f.len);
	
	return &f;
}

int read_next_dirent(DIR *dir)
{
	struct stat st;
	struct dirent *dire;
	int flags;
	
	
	if (dir == NULL) {
		printf ("%s:%u\n", __FUNCTION__, __LINE__);
		dire=NULL;
		normal_return (0x70);
		return(0);
	}
	
	while((dire=readdir(dir)) != NULL) {
		flags=0;
		
		if(stat(dire->d_name,&st)) {
			normal_return(0x31);
			return 0;
		}
		
		if (S_ISDIR(st.st_mode)) flags=DIR_FLAG;
		else if (!S_ISREG (st.st_mode))
		        continue;
		
		if(dire->d_name[0]=='.') continue; // skip "." ".." and hidden files
		if(dire->d_name[0]=='#') continue; // skip "#"
		
		if(strlen(dire->d_name)>FNAME_MAX) continue; // skip long filenames
		
		/* add file to list so we can traverse any order */
		add_file (make_file_entry(dire->d_name, st.st_size, flags));
		
		break;
	}
	
	if (dire == NULL)
		return 0;
	
	return 1;
}

char *ts2unix(char *fname)
{
	int i;
	for(i=dot_offset;i>1;i--)
		if(fname[i-1]!=' ') break;
	
	if(fname[dot_offset+1]=='<' && fname[dot_offset+2]=='>') {
		fname[i]=0;
	} else {
		fname[i]=fname[dot_offset];
		fname[i+1]=fname[dot_offset+1];
		fname[i+2]=fname[dot_offset+2];
		fname[i+3]=0;
	}
	return fname;
}


void list_dir()
{
	DIR * dir;
	
	//  if (dir!=NULL)
	//    closedir(dir);
	
	dir=opendir(".");
	/** rebuild the file list */
	file_list_clear_all();
	//  fname_ndx = 0;
	if(dir_depth) add_file (make_file_entry("..", 0, DIR_FLAG));
	while (read_next_dirent (dir));
	
	closedir(dir);
}


int ts_dir_ref(unsigned char *data)
{
	char *p;
	char filename[25];
	
	switch (data[29]) {
	case 0x00:	/* Pick file for open/delete */
		fprintf (stderr, "Directory req: %02x (pick file)\n", data[29]);
		strncpy(filename,(char *)data+4,24);
		filename[24]=0;
		//if (debug)
		fprintf (stderr, "Request: %s\n", filename);
		/* Remove trailing spaces */
		for(p = strrchr(filename,' '); p >= filename && *p == ' '; p--)
			*p = 0;
		cur_file=find_file(filename);
		if(cur_file) { 
			fprintf (stderr, "Found: %s  server: %s  len:%d\n", cur_file->tsname, cur_file->ufname, cur_file->len);
			out_dirent(cur_file);
			
			
		} else {
			//	  strncpy(cur_file->tsname, filename, 12);
			fprintf (stderr, "Can't find: %s\n", filename);
			out_dirent(NULL);
			//			  empty_dirent();
			if(filename[dot_offset+1]=='<' && filename[dot_offset+2]=='>') {
				cur_file=make_file_entry(ts2unix(filename), 0, DIR_FLAG);
			} else {
				cur_file=make_file_entry(ts2unix(filename), 0, 0);
			}
		}
		
		break;
	case 0x01:	/* "first" directory block */
		fprintf (stderr, "Directory req: %02x (first entry)\n", data[29]);
		list_dir();
		/** send the file name */
		out_dirent(get_first_file());
		break;
	case 0x02:	/* "next" directory block */
		fprintf (stderr, "Directory req: %02x (next entry)\n", data[29]);
		out_dirent(get_next_file());
		break;
	case 0x03:	/* "previous" directory block */
		fprintf (stderr, "Directory req: %02x (prev file)\n", data[29]);
		out_dirent(get_prev_file());
		break;
	case 0x04:	/* end directory reference */
		fprintf (stderr, "Directory req: %02x (close dir)\n", data[29]);
		//	closedir(dir);
		//	file_list_clear_all ();
		//			dir=NULL;
		break;
	}
	return 0;
}

static unsigned char dir_msg[14];
void update_dirname()
{
	if(dir_depth) {
		char dirbuf[1024];
		int i,j;
		
		if(getcwd(dirbuf, 1024) ) {
			memset(dir_msg,' ',sizeof(dir_msg));
			
			//      fprintf(stderr, "update dir = %s\n", dirbuf);
			for(i=strlen(dirbuf); i>=0 ; i--)
				if(dirbuf[i]=='/') break;
			
			//      fprintf(stderr, "update dir = %s\n", dirbuf+i);
			for(j=0; j<6 && dirbuf[i+j+1] && dirbuf[i+j+1]!='.'; j++)
				dir_msg[3+j]=dirbuf[i+j+1];
			
			
			dir_msg[0]=0x12;
			dir_msg[1]=0x0b;
			dir_msg[2]=0x00;
			dir_msg[9]='.';
			dir_msg[10]='<';
			dir_msg[11]='>';
			//    dir_msg[12]=' ';
			dir_msg[13]=checksum(dir_msg);
		}
	}
	
}

void send_current_path()
{
	static unsigned char root[] = {0x12, 0x0b, 0x00, 0x52, 0x4f, 0x4f, 0x54, 0x20, 0x20, 0x2e, 0x3c, 0x3e, 0x20, 0x96};
	
	//  fprintf(stderr, "dir depth = %d\n",dir_depth);
	if(dir_depth==0) 
		my_write (client_fd, root, sizeof (root));
	else
		my_write (client_fd, dir_msg, sizeof (dir_msg));
}



int ts_open(unsigned char *data)
{
	//if(debug) fprintf (stderr, "open_file() mode:%d filename:%s dire->d_name:%s\n", omode,filename,dire->d_name);
	unsigned char omode = data[4];
	
	switch(omode) {
	case 0x01:	/* New file for my_write */
		fprintf (stderr, "open mode: %02x (write)\n", omode);
		if (file >= 0) {
			close(file);
			file=-1;
		}
		if(cur_file->flags&DIR_FLAG) {
			if(mkdir(cur_file->ufname,0775)==0) {
				normal_return(ST_OK);
			} else {
				normal_return(ST_OPEN_FRMT_MISMATCH);
			}
		} else {
			file = open (cur_file->ufname,O_CREAT|O_TRUNC|O_WRONLY|O_EXCL,0666);
			if(file<0)
				normal_return(ST_OPEN_FRMT_MISMATCH);
			else {
				mode=omode;
				normal_return(ST_OK);
			}
		}
		break;
	case 0x02:	/* existing file for append */
		fprintf (stderr, "open mode: %02x (append)\n", omode);
		if (file >= 0) {
			close(file);
			file=-1;
		}
		if(cur_file==0) {
			normal_return(ST_OPEN_FRMT_MISMATCH);
			return -1;
		}
		file = open (cur_file->ufname, O_WRONLY | O_APPEND);
		if (file < 0)
			normal_return(ST_OPEN_FRMT_MISMATCH);
		else {
			mode=omode;
			normal_return (ST_OK);
		}
		break;
	case 0x03:	/* Existing file for read */
		fprintf (stderr, "open mode: %02x (read)\n", omode);
		if (file >= 0) {
			close (file);
			file=-1;
		}
		if(cur_file==0) {
			normal_return(ST_FILE_DOES_NOT_EXIST);
			return -1;
		}
		
		if(cur_file->flags&DIR_FLAG) {
			int err=0;
			// directory
			if(cur_file->ufname[0]=='.' && cur_file->ufname[1]=='.') {
				// parent dir
				if(dir_depth>0) {
					err=chdir(cur_file->ufname);
					if(!err) dir_depth--;
				}
			} else {
				// enter dir
				err=chdir(cur_file->ufname);
				dir_depth++;
			}
			update_dirname();
			if(err) normal_return(0x37);
			else normal_return (ST_OK);
		} else {
			// regular file
			file = open (cur_file->ufname, O_RDONLY);
			if(file<0)
				normal_return(ST_FILE_DOES_NOT_EXIST);
			else {
				mode = omode;
				normal_return (ST_OK);
			}
		}
		break;
	}
	return (file);
}

void ts_read(void)
{
	int in;

	buf[0]=0x10;
	if(file<0) {
		normal_return(ST_NO_FILENAME);
		return;
	}
	if(mode!=3) {
		normal_return(ST_OPEN_FRMT_MISMATCH);
		return;
	}
	in = read (file, buf+2, 128);
	buf[1] = (unsigned char) in;
	buf[2+in] = checksum(buf);
	my_write (client_fd, buf, 3+in);
}

#if 0
void respond_mystery()
{
	static unsigned char canned[] = {0x38, 0x01, 0x00};

	memcpy (buf, canned, sizeof (canned));
	buf[sizeof(canned)] = calc_sum (canned[0], canned[1], canned + 2);
	my_write (client_fd, buf, sizeof (canned) + 1);
}

void respond_mystery2()
{
	static unsigned char canned[] = {0x14, 0x0F, 0x41, 0x10, 0x01, 0x00, 0x50, 0x05, 0x00, 0x02, 0x00, 0x28, 0x00, 0xE1, 0x00, 0x00, 0x00};

	memcpy (buf, canned, sizeof (canned));
	buf[sizeof(canned)] = calc_sum (canned[0], canned[1], canned + 2);
	my_write (client_fd, buf, sizeof (canned) + 1);
}
#endif

void ts_rename(unsigned char *data)
{
	char *new_name = (char *)data + 4;
	
	new_name[24]=0;
	
	if(rename (cur_file->ufname, ts2unix(new_name)))
		normal_return(0x4A);
	else
		normal_return(ST_OK);
}

int readbytes(int handle, void *buf, int max)
{
	int r = 0;
	int rval;
	
	while (r < max) {
		rval = read (client_fd, buf + r, 1);
		if (rval < 0)
			continue;
		r += rval;
	}
	
	return (r);
}

int send_installer(char *f)
{
	int w=0;
	int i=0;
	int fd;
	int byte_usleep = bootstrap_byte_msec*1000;
	unsigned char b;

	if((fd=open(f,O_RDONLY))<0) {
		if(debug)
			fprintf(stderr, "Failed to open %s for read.\n",f);
		return(9);
	}

	if(debug) {
		fprintf(stderr, "Sending %s\n",f);
		fflush(stdout);
	}

	while(read(fd,&b,1)==1) {
		while((i=my_write(client_fd,&b,1))!=1);
		w+=i;
		usleep(byte_usleep);
		if(debug)
			fprintf(stderr, "Sent: %d bytes\n",w);
		else
			fprintf(stderr, ".");
		fflush(stdout);
	}
	fprintf(stderr, "\n");
	b = BASIC_EOF;
	my_write(client_fd,&b,1);
	close(fd);
	close(client_fd);

	if(debug) {
		fprintf(stderr, "Sent %s\n",f);
		fflush(stdout);
	}
	else
		fprintf(stderr, "\n");

	return(0);
}

void out_buf(unsigned char *bufp, unsigned len)
{
	unsigned i,j,k;
	
	if (!debug)
		return;
	
	for (i = 0; i < len;) {
		for (j = 0; j < 2; j++) {
			for (k = 0; i < len && k < 8; k++, i++)
				fprintf (stderr, "%02X ", bufp[i]);
			fprintf (stderr, "   ");
		}
	}
	fprintf (stderr, "\n");
}

void process_Z_cmd(unsigned char *data)
{
	if(checksum(data+2)!=data[data[3]+4]) {
		if(debug) {
			fprintf(stderr, "BAD CHECKSUM!\n");
			fprintf(stderr, "Packet checksum: %02X  My checksum: %02X\n", data[data[3]+4], checksum(data+2));
		}
		normal_return(ST_PARAMETER_ERROR); //  should this be CRC error?
		return;
	}
	
	switch(data[2]) {
	case 0x00:	/* Directory ref */
		ts_dir_ref(data);
		break;
	case 0x01:	/* Open file */
		fprintf(stderr,"open()\n");
		ts_open(data);
		break;
	case 0x02:	/* Close file */
		fprintf(stderr,"close()\n");
		if(file>=0)
			close(file);
		file = -1;
		normal_return(ST_OK);
		break;
	case 0x03:	/* Read */
		fprintf(stderr,"read()\n");
		ts_read();
		//			fprintf(stderr,"read_file end\n");
		break;
	case 0x04:	/* Write */
		fprintf(stderr,"write()\n");
		if(file<0) {
			normal_return(ST_NO_FILENAME);
			break;
		}
		if(mode!=1 && mode !=2) {
			normal_return(ST_OPEN_FRMT_MISMATCH);
			break;
		}
		//				if(my_write(file,data+4,data[3])!=data[3])
		if(write(file,data+4,data[3])!=data[3])
			normal_return(0x4a);
		else
			normal_return(ST_OK);
		break;
	case 0x05:	/* Delete */
		fprintf(stderr,"delete()\n");
		if(cur_file->flags&DIR_FLAG)
			rmdir(cur_file->ufname);
		else 
			unlink (cur_file->ufname);
		list_dir();
		normal_return(0x00);
		break;
	case 0x06:	/* Format disk */
		normal_return(ST_OK);
		break;
	case 0x07:	/* Drive Status */
		normal_return(ST_OK);
		break;
	case 0x08:	/* TS-DOS DME Request */
		fprintf(stderr,"DME()\n");
		// chage to FDC mode (?)
		send_current_path();
		break;
	case 0x0C:	/* Condition */
		normal_return(ST_OK);
		break;
	case 0x0D:	/* Rename File */
		fprintf(stderr,"rename()\n");
		ts_rename(data);
		list_dir();
		break;
#if 0
	case 0x23:  /* TS-DOS mystery command 2 */
		respond_mystery2();
		break;
	case 0x31:  /* TS-DOS mystery command 1 */
		respond_mystery();
		break;
#endif
	default:
		return;
		break;
	}
	if(data[2]!=0x07 && data[2]!=0x08)
		m1rec=0;
	
	return;
}

int be_disk(void)
{
	unsigned char read_buf[131];
	unsigned len;
	unsigned char recv;
	
	unsigned cmd_len;
	unsigned pos;
	
	fprintf(stderr,"be_disk\n");
	pos=0;
	cmd_len=0;
	
	while(1) {
    
		do {
			len=read (client_fd, &recv, 1);
		} while (len!=1);
		
		//	fprintf(stderr,"pos:%d %02x:%c\n",pos,recv,recv);
		
		if(pos==0) {
			switch(recv) {
			case 'Z':
				cmd_len=4;
				read_buf[pos++]=recv;
				break;
			case 'R':
				cmd_len=7;
				read_buf[pos++]=recv;
				break;
			case 'M':
				cmd_len=2;
				read_buf[pos++]=recv;
				break;
			case '\r':
				// send feedback?
				//	      	normal_return(0x00);
				break;
			default:
				break;
			}
		} else { // not first char
			read_buf[pos++]=recv;
			if(pos>=cmd_len) {
				if(read_buf[0]=='Z' && cmd_len ==4) {
					cmd_len=read_buf[3]+5;
				} else {
					if (debug) {
						fprintf (stderr, "RECV: ");
						out_buf (read_buf, cmd_len);
					}
					
					// cmd end
					switch(read_buf[0]) {
					case 'Z':
						process_Z_cmd(read_buf);
						break;
					case 'M':
						m1rec=true;
						break;
					case 'R':
						break;
					}
					// send response
					cmd_len = 0;
					pos = 0;
				}
			}
		}
	}
	return 0;
}

