// the TPDD drive has a dip switch setting for 76800 baud

/* set weird baud rates on linux
 *
 *   cc -o baud baud.c
 *   baud /dev/ttyUSB0 74880
 *   picocom --noinit /dev/ttyUSB0
 *
 * http://cholla.mmto.org/esp8266/weird_baud/
 */

#define DEFAULT_DEVICE "/dev/ttyUSB0"
#define DEFAULT_BAUD 76800

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <asm/termios.h>
#include <stdio.h>
#include <stdlib.h>

int ioctl (int,int,struct termios2 *);

void set_bother (int fd,int baud) {
    struct termios2 tio;
    int x;

    x = ioctl(fd,TCGETS2,&tio);
    // printf ("x = %d\n",x);

    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;
    tio.c_ispeed = baud;
    tio.c_ospeed = baud;
    x = ioctl(fd,TCSETS2,&tio);
    // printf ("x = %d\n",x);
}

int main (int argc,char **argv)
{
    char *device = DEFAULT_DEVICE;
    int baud = DEFAULT_BAUD;
    int fd;

    if (argc==2) device = argv[1];
    if (argc>2) baud = atoi(argv[2]);

    fd = open(device,O_RDWR);
    if (fd<0) {
	printf("Sorry, cannot open %s\n",device);
	return 1;
    }

    set_bother(fd,baud);
    printf("Baud rate for %s set to %d\n",device,baud);
}
