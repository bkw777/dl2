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
make clean all && sudo make install
```

## uninstall
```
sudo make uninstall
```

## manual
```
dl -h
```

```
bkw@fw:~/src/dlplus$ dl -h
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

bkw@fw:~/src/dlplus$ 
```
```
bkw@fw:~/src/dlplus$ dl -l
dl - DeskLink+ v1.5.010-47-g93f3db4 - "bootstrap" help

Available loader files (in /usr/local/lib/dl):

TRS-80 Model 100 & 102 : TEENY.100 TINY.100 TS-DOS.100 DSKMGR.100
TANDY Model 200        : TEENY.200 TS-DOS.200 DSKMGR.200
NEC PC-8201(a)/PC-8300 : TEENY.NEC TS-DOS.NEC
Kyotronic KC-85        : DSKMGR.K85
Olivetti M-10          : DSKMGR.M10 TEENY.M10

Filenames given without any leading path are taken from above.
To specify a file in the current directory, include the "./"
Examples:

   dl -b TS-DOS.100
   dl -b ~/Documents/LivingM100SIG/Lib-03-TELCOM/XMDPW5.100
   dl -b ./rxcini.DO

bkw@fw:~/src/dlplus$ 
```

## run the TPDD server, verbose, upcase, serving files from the current directory
```
dl -vu
```

## list all available TPDD client installers, and then bootstrap one of them
```
dl -l
dl -vb TS-DOS.100
```

## bootstrap a [REXCPM](http://bitchin100.com/wiki/index.php?title=REXCPM)
```
unzip REXCPMV21_b19.ZIP
dl -vb ./rxcini.DO ;dl -vu
```
## fun
The "ROOT  " and "PARENT" labels are not hard coded in TS-DOS. You can set them to other things. Sadly, this does not extend as far as being able to use ".." for "PARENT". TS-DOS thinks it's an invalid filename (even though it DISPLAYS it in the file list just fine. If it would just go ahead and send the command to "open" it, it would work.) However, plenty of other things that are all better than "ROOT  " and "PARENT" do work.
```
ROOT_LABEL=/ PARENT_LABEL=^ dl
ROOT_LABEL='-root-' PARENT_LABEL='-back-' dl
ROOT_LABEL='0:' PARENT_LABEL='^:' dl
or you can confuse someone...  
ROOT_LABEL='C:\' PARENT_LABEL='UP:' dl
```
## UR-II
Ultimate ROM II ([docs](http://www.club100.org/library/libdoc.html)) ([roms](https://bitchin100.com/wiki/index.php?title=REXsharp#Option_ROM_Images_for_Download)) has a feature where it can load a RAM version of TS-DOS from disk on-the-fly.  
This allows you to keep the TS-DOS executable on the disk instead of in ram, and it is loaded and then discarded on-demand by selecting the TS-DOS menu entry from inside UR2.

A potential problem with this, with an emulator that supports TS_DOS directories, is that UR2 doesn't know anything about directories, and just tries to load a file named "DOS___.CO".  

If you had previously used the UR-II TS-DOS feature and used it to navigate into a subdirectory that didn't contain a copy of DOS___.CO, then UR2 would normally fail to load TS-DOS after that, until you restarted the TPDD server to make it go back to the root share dir.  

This version of dlplus has special support for UR2, so that UR2 may still load DOS100.CO, DOS200.CO, or DOSNEC.CO no matter what subdirectory the server has been navigated to, and no matter if the share path contains a copy enywhere in any directory.  
When the client requests any of the special filenames, the file is searched in the current directory first, like any other file. If it's found, it's used.  
If the file is not found in the current dir, then the root share dir is tried next, and if that fails then finally the app lib dir is tried.

The [clients/](clients/) directory includes copies of [DOS100.CO](clients/ts-dos/DOS100.CO), [DOS200.CO](clients/ts-dos/DOS200.CO), and [DOSNEC.CO](clients/ts-dos/DOSNEC.CO)  
These are also installed to ```/usr/local/lib/dl``` by ```sudo make install```, but you can pretty much ignore them since they will be loaded from the lib dir any time they are needed. You don't have to place copies in your share dir like you would have to on a readl disk.

## OS Compatibility
Tested on Linux, Macos, FreeBSD

Notes for [FreeBSD](ref/freebsd.txt)
