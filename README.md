# dlplus
DeskLink+ is a Tandy Portable Disk Drive emulator or "TPDD Server" implimented in C.

[Original README](README.txt)

[Documentation for DeskLink](dl.do)

Original source: <http://bitchin100.com/files/linux/dlplus.zip>

Serial Cable: <http://tandy.wiki/Model_100_102_200_600_Serial_Cable>

## news
Added -b option for "bootstrap"

To install teeny* on the Model 100, just run (on a host linux/osx/freebsd machine):
```
dl -b
```
and follow the prompt.

After that, do the following in BASIC:
```
 NEW
 LOADM "TEENY"
```
look at "Top:#####"  
then enter:  
```CLEAR 0,#####``` (##### = the value from "Top:#####")  
then you may run TEENY.CO from the main menu

Any time you run any other *.CO machine language programs, you may need to repeate the CLEAR command before you can run TEENY again.

This essentially takes the place of teeny-linux / teeny-freebsd / teeny-macosx .  
This just sends LOADER.DO to the M100.  
The default included LOADER.DO containes a BASIC program which creats TEENY.CO  
but you may replace LOADER.DO with another one to install something else like "tiny" or "dskmgr" or possibly even ts-dos etc.

