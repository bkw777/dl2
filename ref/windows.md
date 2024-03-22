# Notes for Windows

## 1 - Install either Cygwin or MSYS2
Cygwin and MSYS2 are pretty similar systems. Pick either one.  
MSYS2 is more convenient. If you don't already have any opinion, use MSYS2.

### for Cygwin
* Install [Cygwin](https://www.cygwin.com/)  
  When it gets to the **Select Packages** screen,  
  select these additional packages to install:  
  **cygwin-devel make gcc-g++ git**  
  * View->Full  
  * Scroll or search to find **cygwin-devel**  
  * Pull-down menu to the right in the "New" column  
  * Select the highest number that doesn't say "(Test)"  
  * Repeat for: **gcc-g++**, **make**, **git**  
* Launch a Cygwin terminal window

### for MSYS2
* Install [MSYS2](https://www.msys2.org/)  
* Close the URCT window that opens after install  
* Launch an MSYS window  
  Start -> MSYS2 -> MSYS2 MSYS  
* Update the installed packages: ```$ pacman -Syu```  
* If the window closed, launch new MSYS window  
* Update again: ```$ pacman -Syu```  
* Install git, gcc, & make:  ```$ pacman -Sy git gcc make```  

## 2 - Download, build, & install dl2
```
git clone https://github.com/bkw777/dl2.git
cd dl2
make clean all && make install
```

## Platform notes

* Getty/daemon mode is #ifdef'd out at compile-time on Windows. No getty option.

* Serial tty devices are named like ttyS#  
Use ```ls /dev/tty*``` to find the serial tty device after plugging in a usb-serial adapter.  
Then use ```ttyS4``` (for example) as the last argument on the dl command line.

* The Windows user might need to be in the Administrator group, I haven't done much testing.

## Example usage session - initialize a REXCPM
Initializing a REXCPM excersizes both the bootstrap and normal file access functions.
In addition to the packages above, install the "unzip" package, or download and unzip the the files from Windows and skip the download & unzip steps shown here.  
You want to get the latest versions from the REXCPM documentation page anyway instead of the exact versions shown below.

Start with a cold-reset of the Model 100: SHIFT+CTRL+BREAK+RESET  
(this erases all RAM, including all files)

* Download & unzip the REXCPM setup files for the Model 100
```
bkw@win10pro_bkw /cygdrive/c/Users/bkw/Documents/REX
$ curl -O http://www.bitchin100.com/wiki/images/0/03/REXCPMV21_b19.ZIP
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 19859  100 19859    0     0   197k      0 --:--:-- --:--:-- --:--:--  199k

bkw@win10pro_bkw /cygdrive/c/Users/bkw/Documents/REX
$ curl -O http://www.bitchin100.com/wiki/images/6/63/M100_OPTION_ROMS.zip
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100  172k  100  172k    0     0   220k      0 --:--:-- --:--:-- --:--:--  220k

bkw@win10pro_bkw /cygdrive/c/Users/bkw/Documents/REX
$ curl -O http://www.bitchin100.com/wiki/images/8/8e/CPMUPD.CO
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100  1206  100  1206    0     0   1088      0  0:00:01  0:00:01 --:--:--  1089

bkw@win10pro_bkw /cygdrive/c/Users/bkw/Documents/REX
$ curl -O http://www.bitchin100.com/wiki/images/a/a7/Cpm410.bk
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100  198k  100  198k    0     0   899k      0 --:--:-- --:--:-- --:--:--  904k

bkw@win10pro_bkw /cygdrive/c/Users/bkw/Documents/REX
$ unzip  REXCPMV21_b19.ZIP
Archive:  REXCPMV21_b19.ZIP
  inflating: rxctst.DO
  inflating: rxcini.DO
  inflating: rxcupg.DO
  inflating: rxcutl.do
  inflating: RXC_12.BR

bkw@win10pro_bkw /cygdrive/c/Users/bkw/Documents/REX
$ unzip M100_OPTION_ROMS.zip
Archive:  M100_OPTION_ROMS.zip
  inflating: checksums.txt
  inflating: IS100.BX
  inflating: MFORTH.BX
  inflating: MP100.bx
  inflating: notes.txt
  inflating: R2C1D.BX
  inflating: R2C100.BX
  inflating: SAR100.BX
  inflating: SUP100.BX
  inflating: TSD100.BX
  inflating: TSR100.BX
  inflating: UR2100.BX
  inflating: ANLYST.BX

bkw@win10pro_bkw /cygdrive/c/Users/bkw/Documents/REX
$
```


* Identify the serial port tty device
```
bkw@win10pro_bkw /cygdrive/c/Users/bkw/Documents/REX
$ ls /dev/tty*
/dev/tty  /dev/ttyS6
```

* Run dl, specifying ttyS6 for the tty device  
The command line is two consecutive commands with different arguments.  
First ```dl -vb rxcini.DO ttyS6``` uses the bootstrap function to send rxcini.DO to the 100 and run it,  
As soon as the previous command is done sending rxcini.DO, the next command ```dl -vu ttyS6``` immediately starts providing normal TPDD file access, with uppercase filename conversion.

rxcini.DO while it is running will use the TPDD to load the REXCPM firmware image,  
and then RXCMGR uses TPDD to load the TS-DOS option rom image,  
and then you use TS-DOS to copy CPMUPD.CO to the 100,  
and then CPMUPD.CO uses TPDD to load the CP/M disk image.

```
bkw@win10pro_bkw /cygdrive/c/Users/bkw/Documents/REX
$ dl -vb rxcini.DO ttyS6 && dl -vu ttyS6
DeskLink+ v2.0.000-16-g9d2a488
Loading: "rxcini.DO"
Serial Device: /dev/ttyS6
Working Dir  : /cygdrive/c/Users/bkw/Documents/REX
Opening "/dev/ttyS6" ... OK
Bootstrap: Installing "rxcini.DO"

Prepare BASIC to receive:

    RUN "COM:98N1ENN" [Enter]    <-- TANDY/Olivetti/Kyotronic
    RUN "COM:9N81XN"  [Enter]    <-- NEC

Press [Enter] when ready...
Sending "rxcini.DO" ...
10 clear1000,62000:SCREEN0:GOSUB300:GOSUB75
15 RESTORE 90:FORI=0TO1:READR(I),Q$(I),C$(I,0),C$(I,1):CH(I)=0:NEXTI
20 I=0
25 K=0:GOSUB80
30 GOSUB85
35 Z$=INKEY$:IFZ$=""THEN 35
40 IF ASC(Z$)=32 THEN K=(K+1)MOD2:GOTO 30
45 IF ASC(Z$)=13 THEN CH(I)=K:I=I+1
50 IF I<2 THEN GOTO 25
55 PRINT@280,"Execute choices.. sure? (y/n)   ";
60 Z$=INKEY$:IFZ$=""THEN 60
65 IF Z$="y" OR Z$="Y" THEN 198 ELSE 10
75 PRINT@280,"selection:<space> choice:<enter>";
76 print@40,"This program is able to load REXCPM"
77 print@80,"software, and able to wipe clean"
78 print@120,"the directory."
80 PRINT@R(I)*40,Q$(I):RETURN
85 PRINT@R(I)*40+19,C$(I,K):RETURN
90 DATA 5,"Load REXCPM code? ", "No ","Yes"
91 DATA 6,"Init REXCPM dir?  ", "No ","Yes"

198 if CH(0)=0 and CH(1)=0 then end
199 a=CH(0): hl= CH(1)
200 gosub300
201 n=1:restore400:gosub280:q0=q1
213 gosub240
215 FORX=1TOpeek(m)step2:k=16*peek(z)+peek(z+1)-1105:POKEQ,k:Q=Q+1:cs=cs+k:z=z+2:NEXT:GOTO213
216 gosub270

220 n=2:restore500:gosub280
223 gosub240
224 pokeq0+2,m-256*INT(m/256):pokeq0+3,INT(m/256):callq0+4,0,q:cs=cs+256*peek(q0+1)+peek(q0):q=q+peek(m)/2:GOTO223
226 gosub270

230 rem a$=inkey$:ifa$=""then 207
232 callq1,a,hl:END

240 READP$:m=65536+varptr(p$):print".";:z=256*peek(m+2)+peek(m+1)
241 on (1+((-n)*(peek(z)=47))) goto 242,216,226
242 return

270 print">";q-1:READP:ifcs=Pthenreturn
272 beep:print"Checksum Error! Check file!":end

280 cs=0:print"Stage ";n;": ";:readq1:q=q1:printq;"<";:ifq>=himemthenreturn
281 beep:print"Himem conflict!":end

400 DATA 64704
401 DATA AAAAAAAAOFKPDCMAPMDCMBPMCKMCPMEGCDFOCDFGOBOLHONG
402 DATA EBAHAHAHAHEPCDHONGEBIBOFBCBDNFBGAAFPCKMAPMBJCCMA
403 DATA PMNBOBCDAFAFMCNGPMMJ
404 DATA /
405 DATA 6667
300 CLS:print"REXCPM INITIALIZE v3"
304 return
500 DATA 62000
501 DATA DCFOPDHNDCFPPDDIAAOLCCFMPDCBDCPEDHMNOGBHCBAGAAMN
502 DATA CNPFPOAAMCBGPECBAHAAMNCNPFPOABMCBGPECBAFAAMNCNPF
503 DATA POACMCBGPECBPPPFHOPOMJMCBGPEMNFPPFDKFOPDLHMKBOPD
504 DATA CBGMPDMNKCBBCBHPPDMNKCBBMNEEEGAFMKAOPFDOAGLIMCHO
505 DATA PCCBILPGDGCOCDDGECCDDGFCCDCIAODGCANPCDMCJPPCBBEG
506 DATA AAOLNJDOABDCGHPDCBAHAAMNDIPEMCBAPECBAABKMNDIPEMK
507 DATA HOPCDOADDCIFPGCBABABMNDIPEMCBAPEKPMNCEPFDOACMNCK
508 DATA PFDOABDCGHPDKPDCGGPDDOBLOHDOEBOHCBAAIACCGKPDCKGK
509 DATA PDDKGHPDKEPOKAMKBOPDOLCBADAAMNDIPEHINGBCMKBAPEDO
510 DATA ANOHDKGHPDOGEAEHCKGKPDHMOGHPLAGHMNNEDJMDOOPCDKFP
511 DATA PDLHMKEMPDCBIIPDMNKCBBDOADMNCKPFKPMNCEPFCBAAKADG
512 DATA PPCDHMPOKDMCDHPDCBAAKBDGAACDHMPOKCMCEDPDKPMNCKPF
513 DATA DOABMNCEPFCBKFPDMDKCBBAAAAAAAAAAAAAAAAAAAAAAAAAA
514 DATA AAAAAAAAANAKAJEGGPHCGNGBHEDKFIFIFIFIFIFIANAKAAAJ
515 DATA EOGBGNGFCACADKAAANAKAJEJGOGJHEGJGBGMGJHKGJGOGHCA
516 DATA GEGJHCGFGDHEGPHCHJCOCOCOAAAHANAKAJEDGPGNHAGMGFHE
517 DATA GFCBCBAAAHAHANAKEDGBGOCHHECAHCHFGOCBCBANAKFCGFGN
518 DATA GPHGGFCAFCEFFICAGIGPGPGLCAEDEOFEEMCNFIANAKFAGPHH
519 DATA GFHCCAGDHJGDGMGFCOAAAHAHANAKEDGIGFGDGLCAFEFAEEEE
520 DATA CBCBAAAHAHANAKEDGIGFGDGLHDHFGNCAEFHCHCGPHCCBCBAA
521 DATA CBOKPDMDBPPECBLEPDMDBPPECBPLPDMNJBFHCKFMPDPJKPMN
522 DATA CKPFDOABMNCEPFMDKAFLDJDIEODBEEAAHNDCGAPDOFCBIFPG
523 DATA CCGIPDCBLHPGCCGKPDDKGAPDPOAEMCFIPEOLCCGIPDMDGEPE
524 DATA DKGAPDPOADMCGEPEOLCCGKPDOBKPEPDOFKMNBIPFMNBIPFHN
525 DATA DCGAPDMNBIPFIBEPHMMNBIPFIBEPOLCKGIPDOLCECFMKJCPE
526 DATA BKMNBIPFIBEPBDMDIEPEOLCCGIPDCKGKPDHJCPMNBIPFPDMN
527 DATA APPFFPDCGBPDMNAPPFDCGCPDOFFHIDFPDKGHPDLHMCLKPEMD
528 DATA NPPEHMPOMAMCNIPECBAAIADKGHPDPOPPMKLHPEDOPPDCGHPD
529 DATA DOADMNCKPFCBAAIAMNAPPFHHCDIDFPBFMCLAPEMNAPPFDCGD
530 DATA PDFHHLCPDCGEPDLKMCBMPEDOIAEHDKGCPDLIMKABPFKPDCGC
531 DATA PDDKGBPDEHCCGKPDOBKPIGDCGFPDMJCAOGCAMKAPPFNLMIMJ
532 DATA PFNLNIOGBAMKBJPFPBNDMIMJCBAAAAMDCNPFCBACAAPFHNOG
533 DATA AHAHAHAHAHPGIAGHGPPBPFOGPAAPAPAPAPLEGHPBOGAPLFGP
534 DATA PDDKAALIDKAAPCDKAAMEDKAAONDKAAKBDKAAJIHOGFHOMJCB
535 DATA ILPFMNKCBBCBJGPFMNKCBBCBJNPFDKFOPDLHMCHIPFCBKDPF
536 DATA MNKCBBCBKMPFDKFPPDLHMCIIPFCBLBPFMDKCBBANAKAJEBGD
537 DATA HEGJGPGODNAADBDJDCDADACMAAEMGPGBGECMAAEOGPCAGMGP
538 DATA GBGECMAAEJGOGJHEAAEOGPCAGJGOGJHEAA
539 DATA /
540 DATA 104506

DONE

"dl -b" will now exit.
Re-run "dl" (without -b this time) to run the TPDD server.

DeskLink+ v2.0.000-16-g9d2a488
Serial Device: /dev/ttyS6
Working Dir  : /cygdrive/c/Users/bkw/Documents/REX
Opening "/dev/ttyS6" ... OK
-------------------------------------------------------------------------------
"ANLYST.BX"     ANLYST.BX
"CHECKS.TX"     checksums.txt
"CPM410.BK"     Cpm410.bk
"CPMUPD.CO"     CPMUPD.CO
"IS100 .BX"     IS100.BX
"M100_O.ZI"     M100_OPTION_ROMS.zip
"MFORTH.BX"     MFORTH.BX
"MP100 .BX"     MP100.bx
"NOTES .TX"     notes.txt
"R2C100.BX"     R2C100.BX
"R2C1D .BX"     R2C1D.BX
"REXCPM.ZI"     REXCPMV21_b19.ZIP
"RXCINI.DO"     rxcini.DO
"RXCTST.DO"     rxctst.DO
"RXCUPG.DO"     rxcupg.DO
"RXCUTL.DO"     rxcutl.do
"RXC_12.BR"     RXC_12.BR
"SAR100.BX"     SAR100.BX
"SUP100.BX"     SUP100.BX
"TSD100.BX"     TSD100.BX
"TSR100.BX"     TSR100.BX
"UR2100.BX"     UR2100.BX
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
"ANLYST.BX"     ANLYST.BX
"CHECKS.TX"     checksums.txt
"CPM410.BK"     Cpm410.bk
"CPMUPD.CO"     CPMUPD.CO
"IS100 .BX"     IS100.BX
"M100_O.ZI"     M100_OPTION_ROMS.zip
"MFORTH.BX"     MFORTH.BX
"MP100 .BX"     MP100.bx
"NOTES .TX"     notes.txt
"R2C100.BX"     R2C100.BX
"R2C1D .BX"     R2C1D.BX
"REXCPM.ZI"     REXCPMV21_b19.ZIP
"RXCINI.DO"     rxcini.DO
"RXCTST.DO"     rxctst.DO
"RXCUPG.DO"     rxcupg.DO
"RXCUTL.DO"     rxcutl.do
"RXC_12.BR"     RXC_12.BR
"SAR100.BX"     SAR100.BX
"SUP100.BX"     SUP100.BX
"TSD100.BX"     TSD100.BX
"TSR100.BX"     TSR100.BX
"UR2100.BX"     UR2100.BX
-------------------------------------------------------------------------------
```

Here I typed ```RXC_12``` at the filename prompt in rxcini

```
Open for read: "RXC_12.BR"
................................................................................................................................................................................................-------------------------------------------------------------------------------
"ANLYST.BX"     ANLYST.BX
"CHECKS.TX"     checksums.txt
"CPM410.BK"     Cpm410.bk
"CPMUPD.CO"     CPMUPD.CO
"IS100 .BX"     IS100.BX
"M100_O.ZI"     M100_OPTION_ROMS.zip
"MFORTH.BX"     MFORTH.BX
"MP100 .BX"     MP100.bx
"NOTES .TX"     notes.txt
"R2C100.BX"     R2C100.BX
"R2C1D .BX"     R2C1D.BX
"REXCPM.ZI"     REXCPMV21_b19.ZIP
"RXCINI.DO"     rxcini.DO
"RXCTST.DO"     rxctst.DO
"RXCUPG.DO"     rxcupg.DO
"RXCUTL.DO"     rxcutl.do
"RXC_12.BR"     RXC_12.BR
"SAR100.BX"     SAR100.BX
"SUP100.BX"     SUP100.BX
"TSD100.BX"     TSD100.BX
"TSR100.BX"     TSR100.BX
"UR2100.BX"     UR2100.BX
-------------------------------------------------------------------------------
```

After rxcini completed:  
- Typed ```CALL 63012``` in BASIC to install RXCMGR from the REXCPM  
- Exited BASIC and launched RXCMGR from the main menu  
- Pressed TAB to switch to the ROM screen in RXCMGR  
- Pressed F2 for Load  
- Entered ```TSD100```
- Pressed Enter on the new TS-DOS entry to install the TS-DOS option rom (which also launches it)

```
Open for read: "TSD100.BX"
................................................................................................................................................................................................................................................................-------------------------------------------------------------------------------
"ANLYST.BX"     ANLYST.BX
"CHECKS.TX"     checksums.txt
"CPM410.BK"     Cpm410.bk
"CPMUPD.CO"     CPMUPD.CO
"IS100 .BX"     IS100.BX
"M100_O.ZI"     M100_OPTION_ROMS.zip
"MFORTH.BX"     MFORTH.BX
"MP100 .BX"     MP100.bx
"NOTES .TX"     notes.txt
"R2C100.BX"     R2C100.BX
"R2C1D .BX"     R2C1D.BX
"REXCPM.ZI"     REXCPMV21_b19.ZIP
"RXCINI.DO"     rxcini.DO
"RXCTST.DO"     rxctst.DO
"RXCUPG.DO"     rxcupg.DO
"RXCUTL.DO"     rxcutl.do
"RXC_12.BR"     RXC_12.BR
"SAR100.BX"     SAR100.BX
"SUP100.BX"     SUP100.BX
"TSD100.BX"     TSD100.BX
"TSR100.BX"     TSR100.BX
"UR2100.BX"     UR2100.BX
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
"ANLYST.BX"     ANLYST.BX
"CHECKS.TX"     checksums.txt
"CPM410.BK"     Cpm410.bk
"CPMUPD.CO"     CPMUPD.CO
"IS100 .BX"     IS100.BX
"M100_O.ZI"     M100_OPTION_ROMS.zip
"MFORTH.BX"     MFORTH.BX
"MP100 .BX"     MP100.bx
"NOTES .TX"     notes.txt
"R2C100.BX"     R2C100.BX
"R2C1D .BX"     R2C1D.BX
"REXCPM.ZI"     REXCPMV21_b19.ZIP
"RXCINI.DO"     rxcini.DO
"RXCTST.DO"     rxctst.DO
"RXCUPG.DO"     rxcupg.DO
"RXCUTL.DO"     rxcutl.do
"RXC_12.BR"     RXC_12.BR
"SAR100.BX"     SAR100.BX
"SUP100.BX"     SUP100.BX
"TSD100.BX"     TSD100.BX
"TSR100.BX"     TSR100.BX
"UR2100.BX"     UR2100.BX
-------------------------------------------------------------------------------
```

Now TS-DOS option rom is installed.  
Next, use TS-DOS to copy CPMUPD.CO from "disk" to the 100.

```
Open for read: "CPMUPD.CO"
..........
-------------------------------------------------------------------------------
"ANLYST.BX"     ANLYST.BX
"CHECKS.TX"     checksums.txt
"CPM410.BK"     Cpm410.bk
"CPMUPD.CO"     CPMUPD.CO
"IS100 .BX"     IS100.BX
"M100_O.ZI"     M100_OPTION_ROMS.zip
"MFORTH.BX"     MFORTH.BX
"MP100 .BX"     MP100.bx
"NOTES .TX"     notes.txt
"R2C100.BX"     R2C100.BX
"R2C1D .BX"     R2C1D.BX
"REXCPM.ZI"     REXCPMV21_b19.ZIP
"RXCINI.DO"     rxcini.DO
"RXCTST.DO"     rxctst.DO
"RXCUPG.DO"     rxcupg.DO
"RXCUTL.DO"     rxcutl.do
"RXC_12.BR"     RXC_12.BR
"SAR100.BX"     SAR100.BX
"SUP100.BX"     SUP100.BX
"TSD100.BX"     TSD100.BX
"TSR100.BX"     TSR100.BX
"UR2100.BX"     UR2100.BX
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
"ANLYST.BX"     ANLYST.BX
"CHECKS.TX"     checksums.txt
"CPM410.BK"     Cpm410.bk
"CPMUPD.CO"     CPMUPD.CO
"IS100 .BX"     IS100.BX
"M100_O.ZI"     M100_OPTION_ROMS.zip
"MFORTH.BX"     MFORTH.BX
"MP100 .BX"     MP100.bx
"NOTES .TX"     notes.txt
"R2C100.BX"     R2C100.BX
"R2C1D .BX"     R2C1D.BX
"REXCPM.ZI"     REXCPMV21_b19.ZIP
"RXCINI.DO"     rxcini.DO
"RXCTST.DO"     rxctst.DO
"RXCUPG.DO"     rxcupg.DO
"RXCUTL.DO"     rxcutl.do
"RXC_12.BR"     RXC_12.BR
"SAR100.BX"     SAR100.BX
"SUP100.BX"     SUP100.BX
"TSD100.BX"     TSD100.BX
"TSR100.BX"     TSR100.BX
"UR2100.BX"     UR2100.BX
-------------------------------------------------------------------------------
```

- Exited TS-DOS  
- Entered BASIC and did ```CLEAR0,60000``` to make room for CPMUPD to run  
- Launched CPMUPD from the main menu  
- Entered ```CPM410.BK``` at the filename prompt in CPMUPD because my REXCPM has a 4MB chip.

```
Open for read: "Cpm410.bk"
.................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................


bkw@win10pro_bkw /cygdrive/c/Users/bkw/Documents/REX
$
```

Press Ctrl+C on the pc to quit dlplus.

REXCPM is now fully installed.
