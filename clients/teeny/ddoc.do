Documentation for D.CO and WEENY.CO by Ron Wiesen
 
 
        General
 
D.BA is a relocating loader which contains two disk file transfer programs: D.CO and WEENY.CO.  The D.CO and WEENY.CO programs are descendents from a family of small size disk file transfer utilities.  The family progeny follows.
 
"In the beginning, God (or somebody) said 'Let there be TINY.'  TINY beget TEENY.  TEENY beget WEENY.  WEENY beget D."  A yellow polka-dot bikini played a role in all the begettin' but it was a minor role.
 
This documentation covers WEENY and D in depth, particularly with respect to their interface with other programs rather than direct command use by an operator.  For operator usage, refer to file TEENYD.DO and note that there is a minor difference in the prompt.
 
 
        Attributes
 
The distinguishing attributes of the family are listed in the table below.
 
+-----+----+-----------+----+---------+
|Name |Size|HIMEM-Range|Bufr|Terminate|
+-----+----+-----------+----+---------+
|TINY |760 |62200-62959|No  |MENU     |
|TEENY|747 |Relocatable|No  |MENU     |
|WEENY|737 |Relocatable|Yes |MENU     |
|D.   |756 |Relocatable|Yes |Return   |
+-----+----+-----------+----+---------+
 
 
                HIMEM Range
 
TINY is a fixed allocation program.  In a COmmand file format, the 6-byte allocation header of TINY.CO defines a TOP address of 62200, an object module length of 760 bytes (END of 62959), and an EXEcution entry address of 62200.
 
TEENY and its progeny are relocatable and are necessarily enclosed in relocating loaders.  TEENY.BA encloses TEENY.CO; D.BA encloses D.CO and WEENY.CO.  The relocating loaders prompt you for an END address and then create a COmmand file with machine language code and allocation header that correspond to the END address that you specify.  When you LOADM or RUNM the COmmand file, an image of the object module is loaded (copied) into HIMEM protected memory.
 
 
                Buffer
 
WEENY and D are command buffered via the Typeahead buffer.  So other programs (BASIC or machine language) can drive WEENY and D by loading "simulated" keystrokes into the Typeahead buffer and then calling the HIMEM image of WEENY or D.
 
The capacity of the Typeahead buffer is 32 keystrokes.  All WEENY and D commands must terminate with a carriage return (CR).  For an 11-character command, 12 simulated keystrokes must be loaded into the Typeahead buffer as shown below.
 
00000000011C11111112222222222333
12345678901R34567890123456789012
++++++++++++
L MYFILE.DO 
 
Because WEENY and D remember file names of prior commands, there is more than enough capacity in the Typeahead buffer to hold a 4-command Load/Kill/Save/Quit sequence which consists of 20 simulated keystrokes as shown below.  Note that the Quit command includes a 1-character file name (Q) that guarantees termination regardless of the outcome (e.g., file name error) of the commands that precede it.
 
00000000011C1C1C111C222222222333
12345678901R3R5R789R123456789012
++++++++++++++++++++
L MYFILE.DO 
            K 
              S 
                Q Q 
 
 
                Terminate
 
WEENY and D differ in how they terminate.  In fact, this is the only functional difference between them.  For the Quit command: WEENY terminates to the main MENU, D terminates by Returning control to the program that Called it.
 
 
                        WEENY
 
Once a program Calls WEENY, the calling program can not regain control.  Considering the capacity of the Typeahead buffer and the way WEENY terminates, a program that Calls WEENY can govern only a few memory/disk file transfers and then one of two things happens.
 
1.  Where a Quit command is present in the Typeahead buffer, TEENY terminates to the main MENU.
 
2.  Where no Quit command is present in the Typeahead buffer, control remains in WEENY.  This allows actual keystrokes by the operator to govern additional file transfers.  Eventually the operator issues a Quit command and WEENY terminates to the main MENU.
 
 
                        D
 
A program that Calls D always regains control.  Control returns to D in one of two ways.
 
1.  Where a Quit command is present in the Typeahead buffer, D returns immediately and the operator has no opportunity to intervene.
 
2.  Where no Quit command is present in the Typeahead buffer, control remains in D and the operator governs additional file transfers.  Eventually the operator issues a Quit command and D returns control to the program that called it.
 
 
        Using D as a Called Subroutine
 
A program (BASIC or machine language) can Call D and be assured of regaining control as long as the program appends a guaranteed Quit command (Q_Q) to the other command or commands that it places in the Typeahead buffer.  The capacity limit of the Typeahead buffer must be considered.
 
Another consideration is that D "remembers" file names of prior commands only back to the time it was called.  In other words, D "forgets" file names from prior invocations.
 
A program that takes these two considerations into account can use D to govern an unlimited amount of memory/disk file transfers.  The BASIC program DBATCH.BA is a batch file driver for D.  It supplies a Q_Q command within the capacity limit of the Typeahead buffer but it relies on the operator to construct 1-command per line batch files (.DO files) with explicit file names.  Of course you can modify DBATCH.BA to include its own "file name memory" so that an explicit file name is needed only in the lead command line of a multi-command sequence.  DBATCH.BA is extensively documented in the DBDOC.DO file; the DBDEMO.DO batch file provides a simple demonstration.
 
 
        Advantage of WEENY Over D
 
Among all members of the family of small disk file transfer utilities, WEENY is the smallest.  It requires 737 bytes of HIMEM memory, so it claims the title of the "World's teeniest DOS."  Because it can interface with other programs, WEENY qualifies (barely) as a Disk Operating System (DOS) but D is a better DOS.
 
Where an image of the highest possible memory allocation object module of WEENY (62223-62959) is in HIMEM memory, WEENY can't be beat!  The maximum size file that WEENY can transfer is 28974 bytes.  The chart below shows the configuration for the maximum size file transfer.
 
=====:======:=====:========:=====:=====
     :      :     :        :Paste:Nonam
LoMem:MAXRAM:HIMEM:MAXFILES:bufr :BASIC
=====:======:=====:========:=====:=====
32768:62960 :62223:0       :Empty:Empty
.....:......:.....:........:.....:.....
 
The amount of free memory reported at the main MENU under this configuration is:
 
29168 Bytes - prior to file Load,
00194 Bytes - for file Save.
 
To maintain this configuration and Load the maximum size file from disk, invoke BASIC and issue the following command which invokes WEENY.
 
CLEAR0,HIMEM:CALLHIMEM
 
The CLEAR0,HIMEM phrase is needed to release the 256 bytes of string space which is reserved by BASIC each time it's invoked.
 
Where the maximum size file is present in memory, less than 256 bytes of memory are free before BASIC is invoked.  In this case BASIC reserves no string space when it's invoked, so no CLEAR phrase is needed.  To Save the maximum size file to disk, invoke BASIC and issue the following command which invokes WEENY.
 
CALLHIMEM
