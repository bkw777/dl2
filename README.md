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
DeskLink+ v1.5.010-90-gf089dd1
dl - DeskLink+ v1.5.010-90-gf089dd1 - main help

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
   -p dir   Share path - directory with files to be served (.)
   -r       RTS/CTS hardware flow control
   -s #     Speed - serial port baud rate 9600 or 19200 (19200)
   -u       Uppercase all filenames
   -v       Verbose/debug mode - more v's = more verbose
   -w       WP-2 mode - 8.2 filenames
   -z #     Milliseconds per byte for bootstrap (7)

The 1st non-option argument is another way to specify the tty device.
The 2nd non-option argument is another way to specify the share path.

   dl
   dl -vvvu -p ~Downloads/REX/ROMS
   dl -vw ttyUSB1 ~/Documents/wp2files

$ 
```
```
$ dl -l
dl - DeskLink+ v1.5.010-47-g93f3db4 - "bootstrap" help

Available loader files (in /usr/local/lib/dl):

TRS-80 Model 100 & 102 : TEENY.100 TINY.100 TS-DOS.100 DSKMGR.100
TANDY Model 200        : TEENY.200 TS-DOS.200 DSKMGR.200
NEC PC-8201(a)/PC-8300 : TEENY.NEC TS-DOS.NEC
Kyotronic KC-85        : DSKMGR.K85 Disk_Power.K85
Olivetti M-10          : DSKMGR.M10 TEENY.M10

Filenames without any leading path are searched from above
if not found in the current directory.
Examples:

   dl -b TS-DOS.100
   dl -vb ~/Documents/LivingM100SIG/Lib-03-TELCOM/XMDPW5.100
   dl -vb rxcini.DO

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
$ dl -vb rxcini.DO && dl -vu
```
([full directions](REXCPM.md))

## UR-II
Ultimate ROM II ([docs](http://www.club100.org/library/libdoc.html)) ([roms](https://bitchin100.com/wiki/index.php?title=REXsharp#Option_ROM_Images_for_Download)) has a feature where it can load a RAM version of TS-DOS from disk on-the-fly.  
This allows you to keep the TS-DOS executable on the disk instead of in ram, and it is loaded and then discarded on-demand by selecting the TS-DOS menu entry from inside UR2.

That normally requires that there be a copy of DOS100.CO on the "disk" so that UR-II can load it. And since this "disk" is actually a server that can cd into other directories, you would normally need a copy of the file in every single directory.  
This version of dlplus has special support for that so that the TS-DOS button always works, even if the file doesn't exist in the current directory, or even if the file doesn't exist anywhere within the share tree.

There are copies of [DOS100.CO](clients/ts-dos/DOS100.CO), [DOS200.CO](clients/ts-dos/DOS200.CO), and [DOSNEC.CO](clients/ts-dos/DOSNEC.CO) installed to ```/usr/local/lib/dl``` by ```sudo make install```.

When the client machine requests any of these files, dlplus first looks in the current directory like normal. If it's there, that is what is used.  
Failing that, then it looks in the root share dir. Failing that, finally it gets the file from /usr/local/lib/dl. This way the TS-DOS button in Ultimate ROM II just always works by magic.

[More details](ref/ur2.txt)

## sector access - disk images
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

Those examples are both TPDD1, but both TPDD1 and TPDD2 are supported. There just are no known database application disks like Sardine on TPDD2 media to make a good TPDD2 example. You can load up the image of the TPDD2 Utility Disk included with pdd.sh just to see that it works, but that isn't useful for anything.  

Example, using Sardine with a Model 100 with an [Ultimate ROM II rom](http://www.club100.org/library/librom.html) installed (or loaded in a [REX](http://bitchin100.com/wiki/index.php?title=Rex)):  
One way to use Sardine is to let UR-II load/unload the program (SAR100.CO for model 100, or SAR200.CO for model 200) from disk into ram on the fly instead of installing permanently in ram normally, and then the program accesses a special dictionary data disk with sector access commands.  
So for this to work, UR-II has to be able to load SAR100.CO from disk, and then SAR100.CO needs to be able to read raw sectors from the dictionary disk.  
This involves two features of dlplus. First, magic files. SAR100.CO is one of the "magic" files bundled with the app, which are always loadable from a client even if there is no such file in the share directory. When UR-II tries to load a file by that name, if there is a file by that name in the current working directory it is used, but even if there is no such file, the file access still works because then it just comes from /usr/local/lib/dl .  
Second, disk image files and sector-access commands. If a disk image file is loaded with the -i option, then when a client tries to use sector-access commands, they work, and the data reads from / writes to the image file. If the given filename does not exist it will be created if the client issues a format command. If the given filename does not exist and is not given with any leading path, then it is searched for in /usr/local/lib/dl, as a few special disks are bundled with the app, and the Sardine dictionary is one.  

To try it out,  

1: Run dl with the following commandline arguments,
```
$ dl -vue -m 1 -i Sardine_American_English.pdd1
```
This tells dlplus to strictly emulate a TPDD1, disable some TPDD2 features and TS-DOS directory support which confuses SAR100.CO, and load the Sardine American English dictionary disk for sector-access commands.  
SAR100.CO is always being provided automatically regardless of any commandline options, and "-i Sardine_American_English.pdd1" will get "Sardine_American_English.pdd1" from /usr/local/lib/dl.  

2: Enter the UR-2 menu.  
Notice the SARDIN entry with the word OFF under it.  
Hit enter on SARDIN.  
Say Y if you get a prompt about HIMEM.  
This loads SAR100.CO into ram, and now the SARDIN entry says ON under it.

3: Enter T-Word and start a new document and type some text.  

4: Press GRPH+F to invoke Sardine to spell-check the document.  
This will invoke the SAR100.CO previously loaded, which will try to do TPDD1 FDC-mode sector access, wich dlplus will respond to with data from the .pdd1 file.  

Another example, [installing Disk Power for Kyotronic KC-85](clients/disk_power/Disk_Power.txt)

Disk image files may be created 2 ways:  
* Use [pdd.sh](https://github.com/bkw777/pdd.sh) **dd** command to read a real disk from a real drive into a disk image file.  
* Run `dl -v -m 1 -i filename.pdd1` or `dl -v -m 2 -i filename.pdd2` with a non-existing or empty file, and then use a client to format disk and write sectors.

Disk image format [disk_image_files.txt](ref/disk_image_files.txt)

## trivia
The "ROOT  " and "PARENT" labels are not hard coded in TS-DOS. You can set them to other things. Sadly, this does not extend as far as being able to use ".." for "PARENT", but many other things work. The ROOT label allows almost anything, the PARENT label is only limited by what TS-DOS thinks is a valid filename. Here are a few examples that do work.
```
$ ROOT_LABEL=/ PARENT_LABEL=^ dl
$ ROOT_LABEL='-root-' PARENT_LABEL='-back-' dl
$ ROOT_LABEL='0:' PARENT_LABEL='^:' dl
or you can confuse someone...  
$ ROOT_LABEL='C:\' PARENT_LABEL='UP:' dl
```
## OS Compatibility
Tested on Linux, Macos, [FreeBSD](ref/freebsd.txt)
