# dl2
DeskLink2 is a [Tandy Portable Disk Drive](http://tandy.wiki/TPDD) emulator or "[TPDD server](http://tandy.wiki/TPDD_server)" written in C.  

## Install
```
$ make clean all && sudo make install
```

## Uninstall
```
$ sudo make uninstall
```

## Manual
```
$ dl -h
DeskLink2 v2.1.001-2-gabcc469

usage: dl [options] [tty_device] [share_path]

options:
   -0       Raw mode - no filename munging, attr = ' '
   -a c     Attribute - attribute byte used for all files (F)
   -b file  Bootstrap - send loader file to client
   -d tty   Serial device connected to the client (ttyUSB*)
   -n       Disable support for TS-DOS directories (enabled)
   -g       Getty mode - run as daemon
   -h       Print this help
   -i file  Disk image filename for raw sector access
   -l       List loader files and show bootstrap help
   -m #     Model - 1 = FB-100/TPDD1, 2 = TPDD2 (2)
   -p dir   Path - /path/to/dir with files to be served (./)
   -r       RTS/CTS hardware flow control
   -s #     Speed - serial port baud rate (19200)
   -u       Uppercase all filenames
   -v       Verbosity - more v's = more verbose
   -w       WP-2 mode - 8.2 filenames
   -z #     Milliseconds per byte for bootstrap (8)

The 1st non-option argument is another way to specify the tty device.
The 2nd non-option argument is another way to specify the share path.

   dl
   dl -vv -p ~/Downloads/REX/ROMS
   dl -v -w ttyUSB1 ~/Documents/wp2files

$ 
```
```
$ dl -l
DeskLink2 v2.1.001-2-gabcc469
Available support files in /usr/local/lib/dl

Loader files for use with -b:
-----------------------------
TRS-80 Model 100/102 : PAKDOS.100 TINY.100 D.100 TEENY.100 DSKMGR.100 TSLOAD.100 TS-DOS.100
TANDY Model 200      : PAKDOS.200 TEENY.200 DSKMGR.200 TSLOAD.200 TS-DOS.200
NEC PC-8201/PC-8300  : TS-DOS.NEC TEENY.NEC
Kyotronic KC-85      : Disk_Power.K85 DSKMGR.K85
Olivetti M-10        : TEENY.M10 DSKMGR.M10

Disk image files for use with -i:
---------------------------------
Sardine_American_English.pdd1
Disk_Power.K85.pdd1


Filenames given without any path are searched from /usr/local/lib/dl
as well as the current directory.
Examples:

   dl -b TS-DOS.100
   dl -b ~/Documents/LivingM100SIG/Lib-03-TELCOM/XMDPW5.100
   dl -vb rxcini.DO && dl -vu
   dl -vun -i Sardine_American_English.pdd1

$
```

Several of the above settings can alternatively be supplied via environment variables, as well as a few other [hacky extra options](ref/advanced_options.txt)

Docs from the past versions of this program. They don't exactly match this version any more.   
[README.txt](README.txt) from [dlplus](http://bitchin100.com/files/linux/dlplus.zip) by John R. Hogerhuis  
[dl.do](dl.do) from [dl 1.0-1.3](http://m100.bbsdev.net/) the original "DeskLink for \*nix" by Steven Hurd

## Hardware
[KC-85 to PC Serial Cable](http://tandy.wiki/Model_T_Serial_Cable)

## Examples:

### Run the TPDD server, verbose, upcase, serving files from the current directory
`$ dl -vu`

### List all available TPDD client installers, and then bootstrap one of them
```
$ dl -l
$ dl -vb TS-DOS.100
```

### Bootstrap a [REXCPM](http://bitchin100.com/wiki/index.php?title=REXCPM)
`$ dl -vb rxcini.DO && dl -vu`  
([Full directions for REXCPM](ref/REXCPM.md))

### Update a [REX#](http://bitchin100.com/wiki/index.php?title=REXsharp)
`$ dl -vb 'rx#u1.do' && dl -vu`

## "Magic Files" / Ultimate ROM II / TSLOAD
There is a short list of filenames that are specially recognized:  
DOS100.CO, DOS200.CO, DOSNEC.CO, DOSM10.CO, DOSK85.CO  
SAR100.CO, SAR200.CO, SARNEC.CO, SARM10.CO, SARK85.CO

When a client requests any of these filenames, dl2 first looks in the current directory (the current directory that the client is CD'd into within the share path). If a file matching the requested filename is there, it is used, the same as for any other file.

Failing that, it looks in the root of the share path

Failing that, it looks in /usr/local/lib/dl

And some of those files are bundled with dl2 and installed in /usr/local/lib/dl:  
DOS100.CO, DOS200.CO, DOSNEC.CO  
SAR100.CO, SAR200.CO

SARNEC.CO is known to have existed, but is currently lost.  
The others probably never existed, but dl2 will recognize and serve them up if available just for completeness.

This allows TSLOAD and the TS-DOS and SARDIN features in Ultimate ROM 2 to work "by magic" at all times without you having to actually place copies of these files in every directory and subdirectory in the share path.

You can override the bundled versions of these files without touching the /usr/local/lib/dl files by placing say a different version of DOS100.CO in the root of a share path, and it will be in effect for all directories in that share path.

[More details](ref/ur2.txt)

## Sector Access / Disk Images
`$ dl -i disk_image.pdd1`  
or  
`$ dl -i disk_image.pdd2`

This is support for disk image files that allow use of raw sector access commands on a virtual disk image file.

Limitations: Only supports using the disk image for sector access. It doesn't provide access to the files in a disk image as files, just as raw sectors.

If the file exists, it's size is used to set the emulation mode to tpdd1 vs tpdd2.  
If the file doesn't exist or is zero bytes, then the last 5 characters in the filename are used, ".pdd1" or ".pdd2", case insensitive.

One example usage is the [Sardine](ref/Sardine.md) spell checker.  
Another is [installing Disk Power for Kyotronic KC-85](clients/disk_power/Disk_Power.txt)

There are 2 ways to create disk image files so far:  
* One way is to use [pdd.sh](https://github.com/bkw777/pdd.sh) to read a real disk from a real drive, and output a disk image file.  
* Another way is to run `dl -i filename`, where the file either doesn't exist or is zero bytes, and then use a client (like TS-DOS or pdd.sh) to format the "disk". When dl2 gets the format command, it will create the disk image.

More details about the disk image format [disk_image_files.txt](ref/disk_image_files.txt)

## ROOT & PARENT labels
The `ROOT  ` and `PARENT` labels are not hard coded in TS-DOS. You can set them to other things.  
In both cases the length is limited to 6 characters.

The ROOT label is `ROOT  ` in the original Travelling Software Desk-Link.  
This is what is shown for the current directory name in the top-right corner of TS-DOS when the current working directory is at the top level directory of the share path, like the root directory of a disk.  
Almost anything may be used for the `ROOT  ` label.

The PARENT label is `PARENT` in the original Travelling Software Desk-Link.  
This is shown as a virtual filename in the top-left filename slot when not in the root directory, and you press Enter on it in order to move up out of the current subdirectory to it's parent directory.  
This is is limited to things that TS-DOS thinks is a valid filename.  
Sadly, `..` can not be used, but here are a few examples that do work.  

`$ ROOT_LABEL=/ PARENT_LABEL=^ dl`  
`$ ROOT_LABEL='-root-' PARENT_LABEL='-back-' dl`  
`$ ROOT_LABEL='-top-' PARENT_LABEL='-up-' dl`  
`$ ROOT_LABEL='0:' PARENT_LABEL='^:' dl`  
or you can confuse someone...  
`$ ROOT_LABEL='C:\' PARENT_LABEL='UP:' dl`

## co2ba.sh
Also included is a bash script to read a binary .CO file and output an ascii BASIC loader .DO file,  
which may then be used with the **-b** bootstrap function to re-create the original binary .CO file on the portable.  
All KC-85 platform machines are supported including TRS-80 Model 100, TANDY 102 & 200, Kyotronic KC-85, Olivetti M10, NEC PC-8201 & PC-8300.  
It's simple and doesn't handle all situations or do anything fancy like relocating, but it handles the common case and serves as a reference and starting point for making a custom loader.  
See [co2ba](co2ba.md)

## OS Compatibility
Tested on Linux, [Mac](ref/mac.md), [FreeBSD](ref/freebsd.md), and [Windows](ref/windows.md).

## TODO - not all necessarily serious
* Store the actual attr byte given by clients when they save a file, rather than faking with hardwired DEFAULT_ATTR=0x46. Store in xattr so that it stays part of the file when it is copied/moved/renamed.
* File/filesystem access on disk images - Currently can only use disk images for sector access.
* Verify if the code works on a big-endian platform - There are a lot of 2-byte values and a lot of direct byte manipulations because the protocol & drive uses MSB-first everywhere while most platforms today do not.
* Figure out and emulate more of the special memory addresses accessible in tpdd2 mode. We already do some.
* Fake sector 0 based on the files in the current share path so that if a client tries to read the FCB table directly it works.
* Fake entire disk image in ram based on current share path files. Option to save the image as long as we're there.

## History / Credits
[DeskLink for ms-dos](https://ftp.whtech.com/club100/com/dl-arc.exe.gz) 1987 Travelling Software  
1.0-1.3  [DeskLink for *nix](http://m100.bbsdev.net/) 2004 Stephen Hurd  
1.4      [DeskLink+](https://www.bitchin100.com/files/linux/dlplus.zip) 2005 John R. Hogerhuis  
1.5      2019 Brian K. White  
2.0      DeskLink2 2023 Brian K. White  
