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

#ifndef DEFAULT_BOOTSTRAP_BYTE_MSEC
#define DEFAULT_BOOTSTRAP_BYTE_MSEC 6
#endif

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

unsigned prev_dir_type = 0;
unsigned fname_ndx = 0;
unsigned file_len;
LocalID cur_id;

int file = -1;
int mode = 0;	/* 0=unopened, 1=Write, 3=Read, 2=Append */
int client_fd = -1; // client tty file handle
unsigned char filename[25];
struct dirent *dire;
DIR *dir = NULL;
char *dirname;
char **args;
struct termios origt;
struct termios ti;
int getty_mode = 0;
int bootstrap_mode = 0;
int bootstrap_byte_msec = DEFAULT_BOOTSTRAP_BYTE_MSEC;
int debug = 0;
int upcase = 0;
unsigned dot_offset = 6; // 6 for 100/102/200/NEC/K85/M10 , 8 for WP-2
unsigned char buf[131];
int client_baud = DEFAULT_CLIENT_BAUD;

int be_disk(void);

void out_buf(unsigned char *bufp, unsigned len);

int bootstrap(char *f);

int send_installer(char *f);

void print_usage() {
	fprintf (stderr, "DeskLink+ usage:\n");
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
	fprintf (stderr, "   -z=#     Sleep # milliseconds between each byte while sending bootstrap file (default " STRINGIFY(DEFAULT_BOOTSTRAP_BYTE_MSEC) ")\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "available bootstrap files:\n");
	// blargh ...
	//(void)(system ("find " STRINGIFY(APP_LIB_DIR) " -regex \'.*/.+\\.\\(100\\|200\\|NEC\\|M10\\|K85\\)$\' -printf \'\%f\\n\' >&2")+1);
	// more blargh...
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

void cat(char *f) {
	int h = -1;
	char b[4097];

	if((h=open(f,O_RDONLY))<0)
		return;
	while(read(h,&b,4096)>0)
		printf("%s",b);
	close(h);
}

int bootstrap(char *f) {
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

	cat(pre_install_txt_file);

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

int my_write (int fh, void *srcp, size_t len) {
	if (debug) {
		fprintf (stderr, "Writing: ");
		out_buf (srcp, len);
	}
	return (write (fh, srcp, len));
}

unsigned char calc_sum(unsigned char type, unsigned char length, unsigned char *data) {
	unsigned short sum=0;
	int i;

	sum+=type;
	sum+=length;
	for(i=0;i<length;i++)
		sum+=data[i];
	return((sum & 0xFF) ^ 255);
}

void normal_return(unsigned char type) {
	buf[0]=0x12;
	buf[1]=0x01;
	buf[2]=type;
	buf[3]=calc_sum(0x12,0x01,buf+2);
	my_write(client_fd,buf,4);
	if(debug)
		fprintf(stderr, "Response: %02X\n",type);
}

int main(int argc, char **argv) {
	int off=0;
	unsigned char client_tty[PATH_MAX];
	char bootstrap_file[PATH_MAX];
	int arg;

	/* create the file list (for reverse order traversal) */
	file_list_init ();

	args = argv;

// ugly redundant with default: below...
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
						getty_mode = 1;
						break;
					case 'u':
						upcase = 1;
						break;
					case 'v':
						debug = 1;
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
						bootstrap_mode = 1;
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
	ti.c_cflag |= CLOCAL|CRTSCTS|CS8;
	if(cfsetspeed(&ti,client_baud)==-1)
		return(1);
	if(tcsetattr(client_fd,TCSANOW,&ti)==-1)
		return(1);

	/* Set up terminal termios struct */
	origt.c_iflag=BRKINT | ICRNL | IMAXBEL | IXON | IXANY | IXOFF;
	origt.c_oflag=OPOST | ONLCR;
	origt.c_lflag=ECHO | ICANON | ISIG | IEXTEN | ECHOE | ECHOKE | ECHOCTL;
	origt.c_cflag=CREAD | CS8 | HUPCL
#ifdef CRTS_OFLOW
		| CRTS_OFLOW
#else
		| CRTSCTS
#endif
#ifdef CRTS_IFLOW
		| CRTS_IFLOW
#endif
		;

	cfsetspeed(&origt,client_baud);

	if(bootstrap_mode)
		exit(bootstrap(bootstrap_file));

	while(1)
		be_disk();

	file_list_cleanup ();

	return(0);
}

int out_dirent (unsigned char *fnamep, unsigned len) {
	unsigned short size;
	unsigned char *dotp;
	unsigned char *p;
	unsigned i;

	if (debug)
		fprintf (stderr, "out_dirent: %s\n", fnamep);

	/* format the filename */
	if (fnamep) {

		if (debug)
			fprintf (stderr, "no fmt: %s\n", fnamep);

		buf[26] = 'F';
		size = htons (len);
		memcpy (buf + 27, &size, 2);
		memset (buf + 2, ' ', 24);

		if (upcase)
			for (i = 0; i < MIN (strlen ((char *)fnamep), 24); i++)
				buf[2+i] = toupper (fnamep[i]);
		else
			memcpy (buf + 2, fnamep, MIN (strlen ((char *)fnamep), 24));

		dotp = memchr (buf + 2, '.', 24);

		if (dotp != NULL) {
			memmove (buf + dot_offset + 2, dotp, 3);
			for (p = dotp; p < buf + dot_offset + 2; p++)
				*p = ' ';
		}

		if (debug)
			fprintf (stderr, "str: %24.24s\n", (char *)buf + 2);
	}

	/* add checksum */
	buf[30] = calc_sum (0x11, 0x1C, buf + 2);

	/* write packet */
	return (my_write (client_fd,buf,31) == 31);
}

int send_dirent (unsigned char *buf, struct stat *st) {
	unsigned short size;
	unsigned char *dot;
	unsigned char *p;
	unsigned i;

	if(dire!=NULL) {
		buf[26]='F';
		size=htons(st->st_size);
		memcpy(buf+27,&size,2);
		memset(buf+2,' ',24);

		if (upcase)
			for (i = 0; i < strlen (dire->d_name); i++)
				buf[2+i] = toupper (dire->d_name[i]);
		else
			memcpy(buf+2,dire->d_name,strlen((char *)dire->d_name));

		dot=memchr(buf+2,'.',24);

		if(dot!=NULL) {
			memcpy(buf + dot_offset + 2, dot, 3);
			for( p = dot; p < buf + dot_offset + 2; p++)
				*p=' ';
		}
	}
	buf[30] = calc_sum (0x11, 0x1C, buf+2);
	return (my_write (client_fd, buf, 31) == 31);
}

int read_next_dirent(struct stat *st) {

	if (dir == NULL) {
		printf ("%s:%u\n", __FUNCTION__, __LINE__);
		dire=NULL;
		normal_return (0x70);
		return(0);
	}
	while((dire=readdir(dir)) != NULL) {
		if (debug)
			fprintf (stderr, "%s (%u)", dire->d_name, (unsigned) st->st_size);

		if(stat(dire->d_name,st)) {
			normal_return(0x31);
			return 0;
		}

		if (!S_ISREG (st->st_mode))
			continue;

		if(st->st_size > 65535)
			continue;

		/* add file to list so we can traverse any order */
		add_file ((unsigned char *) dire->d_name, st->st_size, ++fname_ndx);

		if (debug)
			fprintf (stderr, "added file %s len %ld\n", dire->d_name, st->st_size);

		break;
		}

	if (dire == NULL)
		return 0;

	return 1;
}

int directory(unsigned char length, unsigned char *data) {
	unsigned char search_form;
	unsigned char *p,*dot;
	struct stat st;

	bzero(buf,31);
	buf[29]=40;
	buf[0]=0x11;
	buf[1]=0x1C;
	search_form=data[length-1];
	switch (search_form) {
		case 0x00:	/* Pick file for open/delete */
			strncpy((char *)filename,(char *)data,24);
			filename[24]=0;
			/* Remove trailing spaces */
			for(p = (unsigned char *) strrchr((char *)filename,' '); p >= filename && *p == ' '; p--)
				*p = 0;
			/* Remove spaces between base and dot */
			dot = (unsigned char *) strchr((char *)filename,'.');
			if(dot!=NULL) {
				for(p=dot-1;*p==' ';p--) ;
				memmove(p+1,dot,strlen((char *)dot)+1);
			}
			if(dir!=NULL)
				closedir(dir);
			dir=opendir(".");
			if (upcase)
				for(;read_next_dirent(&st) && dire!=NULL && strcasecmp((char *)dire->d_name, (char *)filename););
			else
				for(;read_next_dirent(&st) && dire!=NULL && strcmp((char *)dire->d_name, (char *)filename););
			send_dirent(buf,&st);
			break;
		case 0x01:	/* "first" directory block */
			if (debug)
				fprintf (stderr, "get first directory entry command\n");
			if(dir!=NULL)
				closedir(dir);
			dir=opendir(".");
			/** rebuild the file list */
			file_list_clear_all();
			while (read_next_dirent (&st));
			/** send the file name */
			if (get_first_file (filename, &file_len, &cur_id) == 0) {
				if (debug)
					fprintf (stderr, "get_first_file -> %s len = %d\n", filename, file_len);
				out_dirent (filename, file_len);
			}
			else
				out_dirent (NULL, 0);
			break;
		case 0x02:	/* "next" directory block */
			if (get_next_file (filename, &file_len, &cur_id) == 0)
				out_dirent (filename, file_len);
			else
				out_dirent (NULL, 0);
			break;
		case 0x03:	/* "previous" directory block */
			if (get_prev_file (filename, &file_len, &cur_id) == 0)
				out_dirent (filename, file_len);
			else
				out_dirent (NULL, 0);
			break;
		case 0x04:	/* end directory reference */
			closedir(dir);
			file_list_clear_all ();
			fname_ndx = 0;
			break;
	}
	return 0;
}

int open_file(unsigned char omode) {
	struct stat st;

	if(debug)
		fprintf (stderr, "Open Mode: %d\n", omode);

	switch(omode) {
		case 0x01:	/* New file for my_write */
			if (file >= 0) {
				close(file);
				file=-1;
			}
			file = open ((char *) filename,O_CREAT|O_TRUNC|O_WRONLY|O_EXCL,0666);
			if(file<0)
				normal_return(0x37);
			else {
				mode=omode;
				normal_return(0x00);
			}
			break;
		case 0x02:	/* existing file for append */
			if (file >= 0) {
				close(file);
				file=-1;
			}
			stat ((char *) filename, &st);
			if(st.st_size>65535) {
				normal_return (0x6E);
				break;
			}
			file = open ((char *) filename, O_WRONLY | O_APPEND);
			if (file < 0)
				normal_return (0x37);
			else {
				mode=omode;
				normal_return (0x00);
			}
			break;
		case 0x03:	/* Existing file for read */
			if (file >= 0) {
				close (file);
				file=-1;
			}
			stat ((char *)filename, &st);
			if (st.st_size > 65535) {
				normal_return (0x6E);
				break;
			}
			file = open ((char *)filename, O_RDONLY);
			if(file<0)
				normal_return(0x37);
			else {
				mode = omode;
				normal_return (0x00);
			}
			break;
	}
	return (file);
}

void read_file(void) {
	int in;

	buf[0]=0x10;
	if(file<0) {
		normal_return(0x30);
		return;
	}
	if(mode!=3) {
		normal_return(0x37);
		return;
	}
	in = read (file, buf+2, 128);
	buf[1] = (unsigned char) in;
	buf[2+in] = calc_sum(0x10, (unsigned char)in, buf+2);
	my_write (client_fd, buf, 3+in);
}

void respond_mystery() {
	static unsigned char canned[] = {0x38, 0x01, 0x00};

	memcpy (buf, canned, sizeof (canned));
	buf[sizeof(canned)] = calc_sum (canned[0], canned[1], canned + 2);
	my_write (client_fd, buf, sizeof (canned) + 1);
}

void respond_mystery2() {
	static unsigned char canned[] = {0x14, 0x0F, 0x41, 0x10, 0x01, 0x00, 0x50, 0x05, 0x00, 0x02, 0x00, 0x28, 0x00, 0xE1, 0x00, 0x00, 0x00};

	memcpy (buf, canned, sizeof (canned));
	buf[sizeof(canned)] = calc_sum (canned[0], canned[1], canned + 2);
	my_write (client_fd, buf, sizeof (canned) + 1);
}

void respond_place_path() {
	static unsigned char canned[] = {0x12, 0x0b, 0x00, 0x52, 0x4f, 0x4f, 0x54, 0x20, 0x20, 0x2e, 0x3c, 0x3e, 0x20, 0x96};

	my_write (client_fd, canned, sizeof (canned));
}

void renamefile(unsigned char *name) {
	unsigned char *p;

	name[24]=0;
	for(p = (unsigned char *) strrchr((char *)name,' ');*p==' ';p--)
		*p=0;
	if(rename ((char *) filename, (char *)name))
		normal_return(0x4A);
	else
		normal_return(0x00);
}

int readbytes(int handle, void *buf, int max) {
	int r = 0;
	int rval;
	unsigned i;

	if (debug)
		fprintf (stderr, "Read... ");

	while (r < max) {
		rval = read (client_fd, buf + r, 1);
		if (rval < 0)
			continue;
		r += rval;
	}

	if (debug) {
		for (i = 0; r >=0 && i < r; i++)
			fprintf (stderr, "%02X ", ((unsigned char *)buf)[i]);
		fprintf (stderr, "\n");
	}

	return (r);
}

int send_installer(char *f) {
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
	b = 0x1A;
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

void out_buf(unsigned char *bufp, unsigned len) {
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

int be_disk(void) {
	unsigned char preamble[3];
	unsigned char type;
	unsigned char length;
	unsigned char data[0x81];
	unsigned char checksum;
	unsigned precnt = 0;
	unsigned len;

	preamble[2]=0;

	for (precnt = 0; precnt < 2; precnt++) {
		len = readbytes(client_fd, preamble + precnt, 1);
		if (len == 0) {
			fprintf (stderr, "Zero Length - 1\n");
			continue;
		}
		if (preamble[precnt] != 'Z') {
			if (debug)
				fprintf(stderr, "Bad preamble: '%s' (%02X %02X)\n",preamble,preamble[0],preamble[1]);
			return (-1); // no error message, TS-DOS doesn't like that
		}
	}

	len = readbytes(client_fd,&type,1);
	if (!len) {
		fprintf (stderr, "zero len - 2\n");
		return (0);
	}

	len = readbytes(client_fd,&length,1);
	if (!len) {
		fprintf (stderr, "zero len - 3\n");
		return (0);
	}

	len = readbytes(client_fd,data,length);
	if (length && !len) {
		fprintf (stderr, "zero len - 4\n");
		return (0);
	}

	data[length]=0;
	len = readbytes(client_fd,&checksum,1);
	if (!len) {
		fprintf (stderr, "zero len - 5\n");
		return (0);
	}

	if (debug)
		fprintf (stderr, "\nProcessing command type = %02X length = %02X csum = %02X\n", type, length, checksum);

	if(calc_sum(type,length,data)!=checksum) {
		if(debug) {
			fprintf(stderr, "BAD CHECKSUM! Preamble: %s Type: %02X Length: %02X Data: %s\n",preamble,type,length,data);
			fprintf(stderr, "Packet checksum: %02X  My checksum: %02X\n",checksum,calc_sum(type,length,data));
		}
		normal_return(0x36);
		return(7);
	}

	switch(type) {
		case 0x00:	/* Directory ref */
			directory(length,data);
			break;
		case 0x01:	/* Open file */
			open_file(data[0]);
			break;
		case 0x02:	/* Close file */
			if(file>=0)
				close(file);
			normal_return(0x00);
			break;
		case 0x03:	/* Read */
			read_file();
			break;
		case 0x04:	/* Write */
			if(file<0) {
				normal_return(0x30);
				break;
			}
			if(mode!=1 && mode !=2) {
				normal_return(0x37);
				break;
			}
			if(my_write(file,data,length)!=length)
				normal_return(0x4a);
			else
				normal_return(0x00);
			break;
		case 0x05:	/* Delete */
			unlink ((char *) filename);
			normal_return(0x00);
			break;
		case 0x06:	/* Format disk */
			normal_return(0x00);
			break;
		case 0x07:	/* Drive Status */
			normal_return(0x00);
			break;
		case 0x08:	/* TS-DOS DME Request */
			respond_place_path();
			break;
		case 0x0C:	/* Condition */
			normal_return(0x00);
			break;
		case 0x0D:	/* Rename File */
			renamefile(data);
			break;
		case 0x23:  /* TS-DOS mystery command 2 */
			respond_mystery2();
			break;
		case 0x31:  /* TS-DOS mystery command 1 */
			respond_mystery();
			break;
		default:
			return(8);
			break;
	}

	return(0);
}
