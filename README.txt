/*
 * DeskLink for *nix (dl)
 * Copyright (C) 2004
 * Stephen Hurd
 *
 * Redistribution of modified and unmodified copies
 * is premitted provided the copyright remains intact
 */

DeskLink+
Extensions and enhancements Copyright (C) 2005 John R. Hogerhuis

DeskLink+ is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 or any
later version as published by the Free Software Foundation.

DeskLink+ is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (in the file "COPYING"); if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111, USA.


3/11/06

Desklink+ is the maintained version of dl.c.
It is compatible with the TS-DOS, the WP-2, TEENY and POWR-DOS clients.
It can (theoretically) be compiled for any Unix variant including Linux
and MacOSX (Darwin).

It was modified originally just to be able to load files to/from a WP-2,
in particular CamelForth for WP-2.

However, it seems to be generally useful. In particular I recently tested
mounting a FAT32 partition under Puppy Linux. This release has fixes
necessary to make that work.


Historical Notes:


If you are using DeskLink+ to launch CAMEL.CO without a ramdisk, try the following:

dl /dev/ttyUSB0 -w -v -p=/home/john/wp2_files


replace /dev/ttyUSB0 with the device for the serial port you're using
replace /home/john/wp2_files with the path to the directory you want
to share with your WP-2 containing CAMEL.CO

This assumes dl is somewhere in your path... if you want to run dl from
the current directory, replace dl in the command above with ./dl

On WP-2, go to Files->DISKETTE, select CAMEL.CO and hit F2-RUN.

CamelForth should launch.

If you have a RAMDisk, I suggest copying the CAMEL.CO to RAMDisk and
running it from there. If you don't have a RAMdisk, this trick will
work but you should really spend the $4-$7 to acquire the internal
128K ramdisk chip.

Keep in mind that CAMEL.CO will not run unless you have main
RAM free. That means having only the minimum 'empty' file in the
main RAM directory. Since the WP-2 won't let you delete the default
file, if you have something there already, create a new empty file
and then you can copy the file you want to preserve in main ram to 
RAMDisk, cassette, DLPilot, DeskLink+, etc., and finally delete the
file from main RAM so only the empty "minimal" file exists.

-- John.
