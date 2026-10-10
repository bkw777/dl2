0'TEENY - TRS-80 Model 100/102 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-10
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?USING"Installing \    \   0%";N
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:?@18,USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":CALL"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA62326,634,62326,95,"TEENY",128,64
4DATA"a@@ybF½a!š!šbJ½jF½¹ïrH½aÃ³¥aµâQrê¿!„!†aŠ³b!’¶aÆ¶vbO!Œ!†azÅ¶¾!‘Š×!—¾!“Š!˜µµ¹³ŠŠ³±–!‹Š<´}ŠÎ´zH½÷Šœ³jB½!„!j@½ß+a«µâQƒÃ³[:ó‚¥³‰yx!Žq!„aL½7cv@‰‹.Vn¥³a¬³w¦W~GÜµï±³vZcQÇ¶FI)tFOv`cE‚^´v!†c0aL½>cÆ!—>!Ÿ!‡c÷:Š!ÿ´Æc]‚y´o7~EÀ!‡aJ½>r.šŠ³cE‚!ˆ´aL½
5DATA"!œ63´3´!L3´M‚!¢´ïpaM½!Žc>÷‰>-‚Š³7c‰~EƒÜµ^A~A±³7c3ƒßµï`‚Š³¤`¥!µjg½!…!Œ) bD½K!Ÿ|C}‚è´!—a\µYYYYY>µQ!…µ•c!žc!–•c!žc&+!žc!– )bB½«b@½¥-+šŠ³aH½tDµjD½«¡zL½!F@:ó9rL½ˆ÷¼Iµ[¥aN½I>¡7cLƒ«´^CÃ´¥•~C±³i´ï!‡!rL½‘¡‰à~µð»€kµî»ÀnµÚ¹vZ‰ï7c7jB½«jî»Ybî»‰j
6DATA"@½bð»‰±j@½«[¡ƒybzÙ¼–!ƒ°~B‰¹³‚Š³zÙ¼¾!‚šŠ³¾!…’Š³ï`ŠŠ³!µ!~BÑ…•«Ýa‘=Ó/<Ú'LckM‚Èµ=ôˆ•¥Á´‘¡ãµ~B±³i´ˆ¥¥aÀ@Ÿ’ÿµ!!„¡I£«AÀ¿I¥‚µ‘¡ƒãµ!‹¡‘~D±³1c!)tƒßµ~`!ƒ`!†!†!†!†!†!†n!˜!˜`h!ƒ}!‹!Œ!“!‘iMJ~`@!…!’MJ@
