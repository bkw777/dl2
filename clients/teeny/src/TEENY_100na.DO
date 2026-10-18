0'TEENY - TRS-80 Model 100/102 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-18
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?USING"Installing \    \   0%";N
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:?@18,USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":CALL"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA62334,626,62334,114,"TEENY",128,64
4DATA"x@«bË³a!š!šbˆ¼q@@ïr†¼aÊ³¥aµâQrê¿!„!†a³b!’¶aÆ¶vbO!Œ!†azÅ¶¾!‘Š×!—¾!“ŠPµµ«³Š³±–!‹Š%´}Š*´z†¼÷Š¢³j‚¼!„!j€¼ß+a«µâQƒÊ³‹.Vnõµa‹µw¦W~G!”µïØµvZcQÇ¶FI)tFOv`cE‚P´v!†c0aŠ¼>cÆ!—>!Ÿ!‡c÷:Šq´Æc]‚k´o7~EÀ!‡aˆ¼>r.š³cE‚z´aŠ¼!œ6éµéµ!LéµM‚!”´ïpa‹¼
5DATA"!Žc>÷‰~Eƒ!”µï`‚³¤`¥àµj¥¼!…!Œ) b„¼K!Ÿ|C}‚Ä´!—aüµYYYYY>µQFµ•c!žc!–•c!žc&+!žc!– )b‚¼«b€¼¥-+š³a†¼t—´j„¼«¡:óˆ9÷¼œ´[¥aŒ¼I>¡7cLƒ´^C µ¥•~CØµ[´ï!‡!‘¡‰vZ‰ï7c7j‚¼«jî»Ybî»‰j€¼bð»‰±j€¼«[¡ƒyb«³‚³zÙ¼¾!‚š³¾!…’³ï`Š³àµ!~BÑ…•«Ýa‘=Ó/<Ú'LckM‚!€µ=ôˆ
6DATA"•¥!žµ‘¡+µ~BØµ[´ˆƒ³^A~AØµ7c3ƒ!—µ¥¥aÀ@Ÿ’Çµ!!„¡I£«AÀ¿I¥Êµ‘¡ƒ+µ!‹¡‘~DØµ1c!)tƒ!—µaŠ¼7cv@‰zÙ¼–!ƒ°~B‰>-š³‚³7c‰[:ó‚õµ‰à¿´ð»€¬´î»À¯´Ú¹yx!Žq!„~`!ƒ`!†!†!†!†!†!†n!˜!˜`h!ƒ}!‹!Œ!“!‘iMJ~`@!…!’MJ@
