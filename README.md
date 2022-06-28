# dlplus
DeskLink+ is a [Tandy Portable Disk Drive](http://tandy.wiki/TPDD) emulator or "[TPDD server](http://tandy.wiki/TPDD_server)" implimented in C.  
2022 [GGLabs](https://gglabs.us/) has added support for TS-DOS subdirectories!  
[hacky extra options](ref/advanced_options.txt)  
[Serial Cable](http://tandy.wiki/Model_T_Serial_Cable)

Docs from the past versions of this program. They don't exactly match this version any more.  
[README.txt](README.txt) from dlplus by John R. Hogerhuis  
[dl.do](dl.do) from dl 1.0-1.3 the original "DeskLink for \*nix" by Steven Hurd
<!-- [Original source](http://bitchin100.com/files/linux/dlplus.zip) -->

## install
```
$ make clean all && sudo make install
```

## uninstall
```
$ sudo make uninstall
```

## manual
```
$ dl -h
```

```
$ dl -h
dl - DeskLink+ v1.5.010-47-g93f3db4 - help

usage: dl [options] [tty_device] [share_path]

options:
   -0       Raw mode. Do not munge filenames in any way.
            Disables 6.2 or 8.2 filename trucating & padding
            Changes the attribute byte to ' ' instead of 'F'
            Disables adding the TS-DOS ".<>" extension for directories
            The entire 24 bytes of the filename field on a real drive is used.
   -a c     Attr - attribute used for all files (F)
   -b file  Bootstrap: Send loader file to client
   -d tty   Serial device to client (ttyUSB0)
   -g       Getty mode - run as daemon
   -h       Print this help
   -i file  Disk image file for raw sector access, TPDD1 only
   -l       List available loader files and bootstrap help
   -p dir   Share path - directory with files to be served (.)
   -r       RTS/CTS hardware flow control
   -s #     Speed - serial port baud rate 9600 or 19200 (19200)
   -u       Uppercase all filenames
   -v       Verbose/debug mode - more v's = more verbose
   -w       WP-2 mode - 8.2 filenames
   -z #     Milliseconds per byte for bootstrap (7)

Alternative to the -d and -p options,
The 1st non-option argument is another way to specify the tty device.
The 2nd non-option argument is another way to specify the share path.

   dl
   dl -vv /dev/ttyS0
   dl ttyUSB1 -v -w ~/Documents/wp2files

```
```
$ dl -l
dl - DeskLink+ v1.5.010-47-g93f3db4 - "bootstrap" help

Available loader files (in /usr/local/lib/dl):

TRS-80 Model 100 & 102 : TEENY.100 TINY.100 TS-DOS.100 DSKMGR.100
TANDY Model 200        : TEENY.200 TS-DOS.200 DSKMGR.200
NEC PC-8201(a)/PC-8300 : TEENY.NEC TS-DOS.NEC
Kyotronic KC-85        : DSKMGR.K85
Olivetti M-10          : DSKMGR.M10 TEENY.M10

Filenames without any leading path are searched from above
if not found in the current directory.
Examples:

   dl -b TS-DOS.100
   dl -b ~/Documents/LivingM100SIG/Lib-03-TELCOM/XMDPW5.100
   dl -b ./rxcini.DO

```

## run the TPDD server, verbose, upcase, serving files from the current directory
```
$ dl -vu
```

## list all available TPDD client installers, and then bootstrap one of them
```
$ dl -l
$ dl -vb TS-DOS.100
```

## bootstrap a [REXCPM](http://bitchin100.com/wiki/index.php?title=REXCPM)
```
$ unzip REXCPMV21_b19.ZIP
$ dl -vb ./rxcini.DO ;dl -vu
```

## Disk image files and FDC-mode sector access
```
$ dl -vi tpdd1_disk_image.pdd1
```
Initial support for [disk image files](ref/disk_image_files.txt) that allow use of FDC-mode sector access commands on a virtual disk image file.

The feature does not support normal "Operation-mode" file access to the filesystem on a disk, just raw sector access. If you issue normal file access commands from a client, like load or save in TS-DOS, they operate on regular files in the share directory the same as if you had not included the -i option.

The feature does not yet support TPDD2 disks or the TPDD2 version of sector access commands.

What it does do is allow applications that use sector access commands like databases to work on a virtual disk image file.  

The most well known possible example would be [Sardine](https://ftp.whtech.com/club100/doc/sardine.pdf) and it's [dictionary disk](http://www.club100.org/memfiles/index.php?&direction=0&order=&directory=Kurt%20McCullum/Sardine), but that has not been tested yet. (the sector log needs to be converted to the .pdd1 format, and needs either a [Sardine ROM](http://www.club100.org/memfiles/index.php?&direction=0&order=&directory=Kurt%20McCullum/SARDOS) or RAM version of Sardine   
It has been tested with a [Disk Power for KC-85](clients/disk_power/Disk_Power.txt) install disk.

Disk images are currently only created by specifying a new image filename with `dl -i` ,and then issuing FDC-mode format and sector-write commands from a tpdd client.

Example:  
 using pdd.sh as the client,  
 (connected with a 2nd usb-serial adapter and a null-modem cable)  
 to create a disk image of the TPDD1 Utility Disk from pdd.sh's existing image of that disk,  
 using pdd.sh's "restore_disk" command to read the .p1h disk image and write it's sectors into dlplus,  
 which writes them into a .pdd1 file.  
```
$ dl -vi ./new_disk.pdd1 ttyUSB0
```

```
$ pdd1 ttyUSB1
PDD(opr:6.2,F)> rd TPDD1_26-3808_Utility_Disk.p1h
Formatting Disk, TPDD1 "FDC" mode, 1280-Byte Logical Sectors
[########################################] 100%                                
Restoring Disk from TPDD1_26-3808_Utility_Disk.p1h
[########################################] 100% (80/80 P:79 L:-/-)
PDD(fdc:6.2,F)>q
$ ls -l *.pdd1
-rw-rw-r-- 1 bkw bkw 103440 Jun 24 19:05 new_disk.pdd1
```

## trivia
The "ROOT  " and "PARENT" labels are not hard coded in TS-DOS. You can set them to other things. Sadly, this does not extend as far as being able to use ".." for "PARENT". TS-DOS thinks it's an invalid filename (even though it DISPLAYS it in the file list just fine. If it would just go ahead and send the command to "open" it, it would work.) However, plenty of other things that are all better than "ROOT  " and "PARENT" do work.
```
$ ROOT_LABEL=/ PARENT_LABEL=^ dl
$ ROOT_LABEL='-root-' PARENT_LABEL='-back-' dl
$ ROOT_LABEL='0:' PARENT_LABEL='^:' dl
or you can confuse someone...  
$ ROOT_LABEL='C:\' PARENT_LABEL='UP:' dl
```
## UR-II
Ultimate ROM II ([docs](http://www.club100.org/library/libdoc.html)) ([roms](https://bitchin100.com/wiki/index.php?title=REXsharp#Option_ROM_Images_for_Download)) has a feature where it can load a RAM version of TS-DOS from disk on-the-fly.  
This allows you to keep the TS-DOS executable on the disk instead of in ram, and it is loaded and then discarded on-demand by selecting the TS-DOS menu entry from inside UR2.

That normally requires that there be a copy of DOS100.CO on the "disk" so that UR-II can load it. And since this "disk" is actually a server that can cd into other directories, you would normally need a copy of the file in every single directory.  
This version of dlplus has special support for that so that the TS-DOS button always works, even if the file doesn't exist in the current directory, or even if the file doesn't exist anywhere within the share tree.

There are copies of [DOS100.CO](clients/ts-dos/DOS100.CO), [DOS200.CO](clients/ts-dos/DOS200.CO), and [DOSNEC.CO](clients/ts-dos/DOSNEC.CO) installed to ```/usr/local/lib/dl``` by ```sudo make install```.

When the client machine requests any of these files, dlplus first looks in the current directory like normal. If it's there, that is what is used.  
Failing that, then it looks in the root share dir. Failing that, finally it gets the file from /usr/local/lib/dl. This way the TS-DOS button in Ultimate ROM II just always works by magic.

[More details](ref/ur2.txt)

## OS Compatibility
Tested on Linux, Macos, FreeBSD

Notes for [FreeBSD](ref/freebsd.txt)
