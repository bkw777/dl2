# Initializing a [REXCPM](http://bitchin100.com/wiki/index.php?title=REXCPM)
REXCPM is a pure RAM device. It has no flash or eeprom, and so has to be loaded with software when first installed, or any time the 100's internal memory battery dies all the way, or if the REXCPM is removed from the 100 for more than a minute or so.

## 1: Download and unzip the following all into one directory:  
* REXCPMV21_b19.ZIP (or whatever is the latest version at the time)  
from http://bitchin100.com/wiki/index.php?title=REXCPM#Software  

* M100_OPTION_ROMS.zip (or T200_ or NEC_)  
from http://bitchin100.com/wiki/index.php?title=REXCPM#Option_ROM_Images_for_Download  

* CPMUPD.CO and CPM210.BK (or CPM410.BK if you have a 4 Meg unit)  
from http://bitchin100.com/wiki/index.php?title=M100_CP/M  

```
$ mkdir rexcpm
$ cd rexcpm
$ wget http://bitchin100.com/wiki/images/0/03/REXCPMV21_b19.ZIP
$ wget http://bitchin100.com/wiki/images/6/63/M100_OPTION_ROMS.zip
$ wget http://bitchin100.com/wiki/images/8/8e/CPMUPD.CO
$ wget http://bitchin100.com/wiki/images/9/9d/Cpm210.bk
$ unzip REXCPMV21_b19.ZIP
$ unzip M100_OPTION_ROMS.zip
```

## 2: bootstrap rxcini.DO and then start the tpdd server with the upcase option.  

```
$ dl -vb rxcini.DO && dl -vu
```
Enter BASIC on the 100 and type `RUN "COM:98N1ENN"` \[Enter\]  
Press Enter on the modern machine.  
```
Load REXCPM code? Yes  
Init REXCPM dir? Yes  
Execute choices.. sure? (y/n) Y  
Name: RXC_12  
```
"Name:" is the basename portion of `RXC_*.BR` from the REXCPM\*.ZIP file.  
The \* part may change over time, and you don't type the .BR part.  
Currently in REXCPMV21_b19.ZIP this is `RXC_12.BR`, so you enter `RXC_12` at that prompt.

When rxcini finishes:  
Turn the 100 off and back on (electrically triggers the REXCPM into a default state)  
Hard-reset the 100: CTRL+BREAK+RESET (frees the ram used by rxcini)  
Enter BASIC and type: `CALL 63012` \[Enter\]  (installs RXCMGR from the REXCPM to the 100's main menu)  

### At this point:  
* dl will be left running in tpdd server mode on the modern machine. Leave that running while performing all the following actions on the 100.  
* the 100 is at the main menu  
* there is a `RXCMGR` entry on the main menu

The REXCPM now has it's basic firmware installed which provides the same REX functionality as [REX#](http://bitchin100.com/wiki/index.php?title=REXsharp) or [REX Classic](http://tandy.wiki/REX). You can use RXCMGR to load option rom images from TPDD, select & activate installed rom images, create and restore ram backup images.

What is not done yet:
* TS-DOS option rom not installed yet  
* CP/M not installed yet

## 3: Install the TS-DOS option rom image  
Run RXCMGR  
Press TAB to get to the ROM screen  
Press F2 (Load)  
Loading from image filename: TSD100  
Press Enter again after the TS-DOS image finishes loading, to activate the TS-DOS rom. (Tis also launches TS-DOS as if you had selected it from the main menu, so at this point you are out of RXCMGR and in the TS-DOS program. It looks very similar to the 100's main menu, because it's the same RAM file list.)

## 4: Use TS-DOS to copy the CP/M installer onto the 100  
The end of the previous step launched TS-DOS so you should already be sitting in TS-DOS right now.  
Press F4 to switch from the RAM file list to the DISK file list.  
Use the arrow keys to highlight CPMUPD.CO  
Press F1, Enter (don't type anything after "Load as: ")  
Press F8 to exit TS-DOS and return to the main menu  

## 5: run the CP/M installer  
Run BASIC  
Type: `CLEAR 0,60000` \[enter\]  
Press F8 to return to the main menu  
Run CPMUPD.CO  
Enter file name: CPM210.BK   (or CPM410.BK if you have a 4M unit)  
Are you sure? (y/n) Y  

## Done  
REXCPM is now fully installed with both the REX and CP/M parts.  
Enter CP/M by pressing CTRL+C at the main menu.  
Return from CP/M to the normal main menu by pressing F8.
