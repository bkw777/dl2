dl "DeskLink for *nix"

Description
-----------
dl is a program for *nix users which has (or will have) the same capabilities as
DeskLink for DOS.  That is, act a PPD disk drive for your Model T laptop.

Usage
-----
Currently, there are two ways of running dl, as a getty replacement, or
from a command-line.  The getty replacement method allows an "always on" drive
whereas the command-line allows you to start and stop the service at will.

To set dl up as a getty replacement,
you'll need to edit /etc/ttys and add
a line somewhat like the following:

ttyd0   "/home/m100/dl/dl -g"              m100    on insecure

(see ttys(5) for more information)
then send a HUP signal to the init process like so:

kill -HUP 1

To run from a command-line, you pass the tty device name as the ONLY
argument to dl for example:

./dl ttyd0

To stop, just press ^C

-v will provide reams of useless info about what's going on.

Compiling
---------
To compile a getty replacement, use the command:

make

Using the loader function
-------------------------
Executing the following command from BASIC will load and run "loader.ba"
without needing a DOS loaded on the laptop.  This is how I load TEENY

OPEN"COM:98N1D"FOROUTPUTAS1:?#1,"XX";:RUN"COM:98N1D

To create the loader.ba file, run the teeny program (available from
http://m100.bbsdev.net) and save teeny.ba then save teeny.ba to
the PC.  On the PC, copy teeny.ba to loader.ba

I've included TEENY as loader.ba.
