0'TEENY - TRS-80 Model 100/102 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-09
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?USING"Installing \    \   0%";N
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:?@18,USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":CALL"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA62313,647,62313,244,"TEENY",128,64
4DATA"a@@ybF½a!š!šbJ½jF½¹ïrH½a6³¥aµâQrê¿!„!†aý³b!’¶aÆ¶vbO!Œ!†azÅ¶¾!‘Š×!—¾!“Š#µµ!„´Šý³±–!‹Š!ÿ´}ŠÓ´zH½÷Š³jB½!„!j@½ß+aªµâQƒ6³[:ó‚˜³‰yx!Žq!„aL½7cv@‰aL½>cÆ!—>!Ÿ!‡c÷:ŠB´Æc]‚¼³o7~EÀ!‡aJ½>r.šý³cE‚K´aL½!œ6v´v´!Lv´M‚e´ïpaM½!Žc>÷‰>-‚ý³7c‰~Eƒçµ‹.Vn
5DATA"˜³aŸ³w¦W~Gçµï¤³vZcQÇ¶FI)tFOv`cE‚)´v!†c0ƒ¬³^A~A¤³7c3ƒêµ‘~D¤³1c!)tƒêµï`‚ý³aý³b!’¶¤`¥!šµjg½!…!Œ) bD½K!Ÿ|C}‚ó´!—agµYYYYY>µQ!µ•c!žc!–•c!žc&+!žc!– )bB½«b@½¥-+šý³aH½tOµjD½«¡zL½!F@:ó9rL½ˆ÷¼Tµ[¥aN½I>¡7cLƒ¶´^C9´¥•~C¤³4´ï!‡!rL½‘¡‰à	µð»€vµî»À
6DATA"yµÚ¹vZ‰ï7c7jB½«jî»Ybî»‰j@½bð»‰±j@½«[¡ƒybzÙ¼–!ƒ°~B‰!„´‚ý³zÙ¼¾!‚šý³¾!…’ý³ï`Šý³!šµ!~BÑ…•«Ýa‘=Ó/<Ú'LckM‚Óµ=ôˆ•¥7´‘¡îµ~B¤³¬³ˆ¥¥aÀ@Ÿ’Šµ!!„¡I£«AÀ¿I¥Å´‘¡ƒîµ!‹¡ƒÄ´~`!ƒ`!†!†!†!†!†!†n!˜!˜`h!ƒ}!‹!Œ!“!‘iMJ~`@!…!’!’MJ@
