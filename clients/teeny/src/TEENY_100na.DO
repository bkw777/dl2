0'TEENY - TRS-80 Model 100/102 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-10
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?USING"Installing \    \   0%";N
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:?@18,USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":CALL"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA62316,644,62316,45,"TEENY",128,64
4DATA"a@@ybF½a!š!šbJ½jF½¹ïrH½a9³¥aµâQrê¿!„!†a€³b!’¶aÆ¶vbO!Œ!†azÅ¶¾!‘Š×!—¾!“Š#µµ¯³Š€³±–!‹Š2´}ŠÓ´zH½÷Š’³jB½!„!j@½ß+aªµâQƒ9³[:ó‚›³‰yx!Žq!„aL½7cv@‰‹.Vn›³a¢³w¦W~Gçµï§³vZcQÇ¶FI)tFOv`cE‚T´v!†c0aL½>cÆ!—>!Ÿ!‡c÷:Šu´Æc]‚o´o7~EÀ!‡aJ½>r.š€³cE‚~´aL½!œ6
5DATA")´)´!L)´M‚!˜´ïpaM½!Žc>÷‰>-‚€³7c‰~Eƒçµ^A~A§³7c3ƒêµ‘~D§³1c!)tƒêµï`‚€³a€³b!’¶¤`¥!šµjg½!…!Œ) bD½K!Ÿ|C}‚ó´!—agµYYYYY>µQ!µ•c!žc!–•c!žc&+!žc!– )bB½«b@½¥-+š€³aH½tOµjD½«¡zL½!F@:ó9rL½ˆ÷¼Tµ[¥aN½I>¡7cLƒ¶´^C9´¥•~C§³_´ï!‡!rL½‘¡‰à	µð»€vµî»ÀyµÚ¹
6DATA"vZ‰ï7c7jB½«jî»Ybî»‰j@½bð»‰±j@½«[¡ƒybzÙ¼–!ƒ°~B‰¯³‚€³zÙ¼¾!‚š€³¾!…’€³ï`Š€³!šµ!~BÑ…•«Ýa‘=Ó/<Ú'LckM‚Óµ=ôˆ•¥7´‘¡îµ~B§³_´ˆ¥¥aÀ@Ÿ’Šµ!!„¡I£«AÀ¿I¥Å´‘¡ƒîµ!‹¡ƒÄ´~`!ƒ`!†!†!†!†!†!†n!˜!˜`h!ƒ}!‹!Œ!“!‘iMJ~`@!…!’!’MJ@
