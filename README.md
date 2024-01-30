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
DeskLink2 v2.0.000-61-g25f82e2
dl - DeskLink2 v2.0.000-61-g25f82e2 - main help

usage: dl [options] [tty_device] [share_path]

options:
   -0       Raw mode - no filename munging, attr = ' '
   -a c     Attr - attribute used for all files (F)
   -b file  Bootstrap - send loader file to client
   -d tty   Serial device connected to client (ttyUSB0)
   -e       Disable TS-DOS directory extension (enabled)
   -g       Getty mode - run as daemon
   -h       Print this help
   -i file  Disk image file for raw sector access, TPDD1 only
   -l       List loader files and show bootstrap help
   -m model Model: 1 for TPDD1, 2 for TPDD2 (2)
   -p dir   Share path - directory with files to be served (./)
   -r       RTS/CTS hardware flow control
   -s #     Speed - serial port baud rate 9600 or 19200 (19200)
   -u       Uppercase all filenames
   -v       Verbose/debug mode - more v's = more verbose
   -w       WP-2 mode - 8.2 filenames
   -z #     Milliseconds per byte for bootstrap (8)

The 1st non-option argument is another way to specify the tty device.
The 2nd non-option argument is another way to specify the share path.

   dl
   dl -vvu -p ~/Downloads/REX/ROMS
   dl -v -w ttyUSB1 ~/Documents/wp2files

$ 
```
```
$ dl -l
DeskLink2 v2.0.000-61-g25f82e2
Available support files in /usr/local/lib/dl

Loader files for use with -b:
-----------------------------
TRS-80 Model 100/102 : PAKDOS.100 TINY.100 D.100 TEENY.100 TS-DOS.100 DSKMGR.100 TSLOAD.100
TANDY Model 200      : PAKDOS.200 TEENY.200 TSLOAD.200 TS-DOS.200 DSKMGR.200
NEC PC-8201/PC-8300  : TS-DOS.NEC TEENY.NEC
Kyotronic KC-85      : Disk_Power.K85 DSKMGR.K85
Olivetti M-10        : TEENY.M10 DSKMGR.M10

Disk image files for use with -i:
---------------------------------
Sardine_American_English.pdd1
Disk_Power.K85.pdd1


Filenames given without any path are searched from /usr/local/lib/dl
as well as the current dir.
Examples:

   dl -b TS-DOS.100
   dl -b ~/Documents/LivingM100SIG/Lib-03-TELCOM/XMDPW5.100
   dl -vb rxcini.DO && dl -vu
   dl -vue -m 1 -i Sardine_American_English.pdd1

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
```
$ dl -vu
```

### List all available TPDD client installers, and then bootstrap one of them
```
$ dl -l
$ dl -vb TS-DOS.100
```

### Bootstrap a [REXCPM](http://bitchin100.com/wiki/index.php?title=REXCPM)
```
$ dl -vb rxcini.DO && dl -vu
```
([Full directions for REXCPM](ref/REXCPM.md))

### Update a [REX#](http://bitchin100.com/wiki/index.php?title=REXsharp)
```
dl -vb 'rx#u1.do' && dl -vu
```

## "Magic Files" / Ultimate ROM II / TSLOAD
Ultimate ROM II ([docs](http://www.club100.org/library/libdoc.html)) ([roms](https://bitchin100.com/wiki/index.php?title=REXsharp#Option_ROM_Images_for_Download)) has a feature where it can load a RAM version of TS-DOS or Sardine from disk on-the-fly.  
This allows you to keep the TS-DOS and/or Sardine executables on the disk instead of installed in ram, and it is loaded and then discarded on-demand by selecting the TS-DOS or Sardine entry from the UR-II menu.

Similarly, a stand-alone program called TSLOAD is available that does the same thing. TSLOAD allows you to install an executable in ram that is much smaller than TS-DOS, and leave the actual TS-DOS executable on the disk. Then when you want to use TS-DOS, to run TSLOAD, and it loads TS-DOS from the disk and runs it, but does not install it.

On a real drive this requires keeping a copy of DOS100.CO (or DOS200.CO, or DOSNEC.CO, and/or SAR100.CO, SAR200.CO etc) on a disk, or on every disk if you want to avoid having to swap them all the time.  
On an emulator like dlplus, the equivalent of that would mean keeping a copy of DOS100.CO in whatever directory you are sharing.  
Additionally, since dlplus can CD into subdirectories, and since UR2 doesn't know about subdirectories, it means you would also have to keep a copy in every subdirectory.  
(if you didn't , then what can happen is, you cd into some directory without a copy of DOS100.CO in it, exit TS-DOS, and then you can never get back in to TS-DOS because the UR2/TSLOAD don't know about cding up to the root directory, they just try to load DOS100.CO, which fails. The only fix is to restart dlplus so it's no longer cd'd in some subdirectory, or copying DOS100.CO to the subdirectory on the host side.)

But this version of dlplus has special support for this feature so that the TS-DOS and Sardine entries in UR-II always work "by magic" in any directory, even if the files don't exist in the current working directory, or the share root, or anywhere within the shared tree.

There are copies of [DOS100.CO](clients/ts-dos/DOS100.CO), [DOS200.CO](clients/ts-dos/DOS200.CO), [DOSNEC.CO](clients/ts-dos/DOSNEC.CO) ,[SAR100.CO](clients/ts-dos/SAR100.CO) and [SAR200.CO](clients/ts-dos/SAR200.CO) installed to ```/usr/local/lib/dl``` by ```sudo make install```. (Sadly, no SARNEC.CO . It is known to have existed, but is currently still lost to time. If you have a copy, please upload it! There may or may not have ever existed any versions of TS-DOS or Sardine for Olivetti M10 or Kyotronic KC-85, but the code in dlplus is ready to support them if they ever turn up, or any other files you may wish to work by magic like that.)

When the client machine requests any of these filenames, dlplus first looks in the current directory like normal. If a file matching the requested filename is there, that is what is used, just like for any other file. This can be used to override the files bundled with dlplus, just for the current working directory.  
Failing that, then it looks in the root share dir. This is another way to override the files bundled with dlplus, for the entire shared directory tree.  
Failing that, it gets the file from /usr/local/lib/dl. These files are bundled with dlplus, and means you never have to manually supply DOS100.CO or any of the others in any share paths.  
This way the TS-DOS and Sardine options in Ultimate ROM II always work "by magic" without you having to do anything to provide the files like with a real drive & disk, yet if you want to use some other version of the files, you can, by just placing them in the share path just like placing them on a disk.

There are bootstrap installers for TSLOAD for Model 100 and 200 bundled with the other client installers.

[More details](ref/ur2.txt)

## Sector Access / Disk Images
For a TPDD1 disk image
```
$ dl -v -m 1 -i disk_image.pdd1
```  

For a TPDD2 disk image
```
$ dl -v -m 2 -i disk_image.pdd2
```

Support for disk image files that allow use of raw sector access commands on a virtual disk image file.  
Limitations: Only supports sector access to the disk image. You can't "mount" the disk image and access the files on a disk as files, just as raw sectors.

Useful working examples: Sardine_American_English.pdd1, Disk_Power_KC-85.pdd1

Those examples are both TPDD1 disks, but both TPDD1 and TPDD2 are supported. There just are no known database application disks like the Sardine dictionary disk on TPDD2 media to make a good TPDD2 example. You could use the image of the TPDD2 Utility Disk included with [pdd.sh](https://github.com/bkw777/pdd.sh) just to see that it works, but that isn't useful for anything.

Example, using Sardine with a Model 100 with [Ultimate ROM II](http://www.club100.org/library/librom.html):  
One way to use Sardine is to let UR-II load/unload the program from disk into ram on the fly instead of installing permanently in ram like normal. Sardine uses raw sector access commands to read a special dictionary data disk.  
For this to work, UR-II has to be able to load SAR100.CO from a normal filesystem disk using normal file/filesystem access, and then SAR100.CO needs to be able to read raw sectors from the special dictionary data disk.  
This involves two features of dlplus. First, **magic files**. SAR100.CO is one of the "magic" files bundled with the app, which are always loadable from a client at any time from any directory even if there is no such file in the directory being served as the virtual "disk". When UR-II tries to load a file by that particular name, if there is a file by that name in the current working directory it is used, but if there is no such file, dlplus just serves up the one from /usr/local/lib/dl, and the client never knows the difference.  
Second, **disk image files and sector-access commands**. If a disk image file is loaded with the **-i** option, then when a client tries to use sector-access commands, they work, and the data reads from / writes to the image file. If the specified filename does not exist it will be created if/when the client issues a format command. If the specified filename does not exist and, is not given with any leading path, then it is searched for in /usr/local/lib/dl, as a few special disks are bundled with the app, and the Sardine dictionary disk is one such.  

To try it out,  

1: Run dl with the following commandline arguments,
```
$ dl -vue -m 1 -i Sardine_American_English.pdd1
```
This set of flags tells dlplus to strictly emulate a TPDD1, disable some TPDD2 features and TS-DOS directory support which confuse SAR100.CO, and use the Sardine American English dictionary disk image file for any sector-access commands the client might issue.  
SAR100.CO is always being provided by default regardless of any commandline options so you don't have to do anything extra for that, and assuming "Sardine_American_English.pdd1" doesn't exist in your current working directory, dlplus will use the copy in /usr/local/lib/dl.  
The disk image file is marked read-only and behaves the same as a normal disk with the write-protect notch open (write-protected).  

2: Enter the UR-2 menu.  
Notice the "SARDIN" entry with the word "OFF" under it.  
Hit enter on SARDIN.  
If you get a prompt about HIMEM, answer Y.  
This loads SAR100.CO into ram.
Notice the SARDIN entry now says "ON" under it.

3: Enter T-Word and start a new document and type some text.  

4: Press GRPH+F to invoke Sardine to spell-check the document.  
This will invoke the SAR100.CO previously loaded, which will try to use TPDD1 FDC-mode sector access commands, wich dlplus will respond to with data from the .pdd1 file.  

Another example, [installing Disk Power for Kyotronic KC-85](clients/disk_power/Disk_Power.txt)

Disk image files may be created 2 ways:  
* One method is you may use the **dd** command within [pdd.sh](https://github.com/bkw777/pdd.sh) to read a real TPDD1 or TPDD2 disk from a real TPDD1 or TPDD2 drive, and output a disk image file.  
* Another method is you may run `dl -v -m 1 -i filename.pdd1` (for TPDD1) or `dl -v -m 2 -i filename.pdd2` (for TPDD2) where filename.pddN either doesn't exist or is zero bytes, and then use a client (like TS-DOS or pdd.sh) to format the "disk". The format command will cause dlplus to generate the empty disk image.  
In the case of TPDD1, there is more than one kind of format command. If the client uses the "Operation-mode" format command, the generated disk image will be a valid filesystem disk, which just means it will have a particular logical sector size (64 bytes) and valid Space Management Table and File Control Blocks. If the client uses the "FDC-mode" format command then the generated image will be a raw data format not a filesystem disk. This just means it will have whatever logical sector size was specified by the parameters to the format command, and no SMT or FCB data, and won't be usable for saving files. In both cases, this mimics what a real drive does.  
TPDD2 does not have an "FDC-mode" and there is only one kind of format command and only one kind of new empty disk format.

More details about the disk image format [disk_image_files.txt](ref/disk_image_files.txt)

## ROOT & PARENT labels
The "ROOT  " and "PARENT" labels are not hard coded in TS-DOS. You can set them to other things. Almost anything may be used for the ROOT label. The PARENT label is limited to things that TS-DOS thinks is a valid filename. Sadly ".." can't be used for PARENT, but here are a few examples that do work.
```
$ ROOT_LABEL=/ PARENT_LABEL=^ dl
$ ROOT_LABEL='-root-' PARENT_LABEL='-back-' dl
$ ROOT_LABEL='0:' PARENT_LABEL='^:' dl
or you can confuse someone...  
$ ROOT_LABEL='C:\' PARENT_LABEL='UP:' dl
```

## co2ba.sh
Also included is a bash script to read a binary .CO file and output an ascii BASIC loader .DO file,  
which may then be used with the bootstrap function to re-create the original binary .CO file on the portable.  
All KC-85 platform machines are supported including TRS-80 Model 100, TANDY 102 & 200, Kyotronic KC-85, Olivetti M10, NEC PC-8201 & PC-8300.  
It's simple and doesn't handle all situations or do anything fancy like relocating, but it handles the common case and serves as a reference and starting point for making a custom loader.  
See [co2ba](co2ba.md)

## OS Compatibility
Tested on Linux, [Mac](ref/mac.md), [FreeBSD](ref/freebsd.md), and [Windows](ref/windows.md).

## TODO
* support big-endian platforms  
* file/filesystem access on disk images - currently can only use for sector access

## History / Credits
[DeskLink for ms-dos](https://ftp.whtech.com/club100/com/dl-arc.exe.gz) 1987 Travelling Software  
1.0-1.3  [DeskLink for *nix](http://m100.bbsdev.net/) 2004 Stephen Hurd  
1.4      [DeskLink+](https://www.bitchin100.com/files/linux/dlplus.zip) 2005 John R. Hogerhuis  
1.5      2019 Brian K. White  
2.0      DeskLink2 2023 Brian K. White  
