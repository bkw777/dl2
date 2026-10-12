0'TEENY - TRS-80 Model 100/102 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-12
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?USING"Installing \    \   0%";N
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:?@18,USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":CALL"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA62322,638,62322,233,"TEENY",128,64
4DATA"x@«b?³a!š!šbˆ¼q@@ïr†¼a>³¥aµâQrê¿!„!†a„³b!’¶aÆ¶vbO!Œ!†azÅ¶¾!‘Š×!—¾!“Š!•µµ³³Š„³±–!‹Š9´}ŠË´z†¼÷Š–³j‚¼!„!j€¼ß+a«µâQƒ>³[:ó‚Ÿ³‰yx!Žq!„aŠ¼7cv@‰‹.VnŸ³a¦³w¦W~GÙµï«³vZcQÇ¶FI)tFOv`cE‚X´v!†c0aŠ¼>cÆ!—>!Ÿ!‡c÷:Šy´Æc]‚s´o7~EÀ!‡aˆ¼>r.š„³cE‚!‚´aŠ¼!œ6
5DATA"-´-´!L-´M‚!œ´ïpa‹¼!Žc>÷‰>-š„³‚„³7c‰~EƒÙµ^A~A«³7c3ƒÜµï`‚„³¤`¥!Œµj¥¼!…!Œ) b„¼K!Ÿ|C}‚å´!—aYµYYYYY>µQ!‚µ•c!žc!–•c!žc&+!žc!– )b‚¼«b€¼¥-+š„³a†¼tAµj„¼«¡zŠ¼!F@:ó9rŠ¼ˆ÷¼Fµ[¥aŒ¼I>¡7cLƒ¨´^CÀ´¥•~C«³c´ï!‡!rŠ¼‘¡‰à{µð»€hµî»ÀkµÚ¹vZ‰ï7c7j‚¼«jî»Ybî»‰j
6DATA"€¼bð»‰±j€¼«[¡ƒybzÙ¼–!ƒ°~B‰³³‚„³zÙ¼¾!‚š„³¾!…’„³ï`Š„³!Œµ!~BÑ…•«Ýa‘=Ó/<Ú'LckM‚Åµ=ôˆ•¥>´‘¡ãµ~B«³c´ˆƒ„³¥¥aÀ@Ÿ’ÿµ!!„¡I£«AÀ¿I¥‚µ‘¡ƒãµ!‹¡‘~D«³1c!)tƒÜµ~`!ƒ`!†!†!†!†!†!†n!˜!˜`h!ƒ}!‹!Œ!“!‘iMJ~`@!…!’MJ@
