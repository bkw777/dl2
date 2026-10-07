0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-06
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing",N:LOCATE22,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE22,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61611,725,61611,100,"TEENY",128,64
4DATA"a@@ybFºa!ö!öbJºjFºπÔrHºa¯∞•a%≥ç!¢Tr(æçÍ!áaT±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìäû≤µçÛ±äB±±ñ!ãä·±}äN≤^@A^BA^FA^HA^PA^RA^TA^VzHº˜äj±ïjBº!Ñ!çj@ºç|,ëa!ç≥V@Y>ßóßa!Ü≥ç!¢TÉ¯∞y!éxq!é!éaLº7cv@âaLº>c∆!ó>!ü!ác˜:ä!°±∆c]Ç!õ±o7~E¿!áaJº>ç˛.öH±cEÇ*±aLºç•6ç’±ç’±!èLç’±MÇƒ
5DATA"±ÔpaMº!éc>˜âçì-öH±ÇE±7câ~Eç!É±ç!ã±à¶∞___!üÉV±çË/Vnç!Ä0a}±wç!é\~Gç„±Ôç!É±vZcQ$¥FIç∑qFOv`cEÇò±v!Üc0ç!ã±!ázLºæRäÍ±8˜â^A~Aç!É±7c3ÉÊ±ë~Dç!É±1c!Åç∑qÉÊ±ç€bÇK±aQ±bæ≥çìb•çï≤jgº!Ö!å) bDºK!ü|C}Çn≤!óa‚≤YYYYY>µQã≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bBº´b@º•çJ,öN±aHºtç ≤jDº´°zL
6DATA"º!èF@:Û9rLºà˜ºœ≤[•aNºI>°7cLÉ1≤^Cç¥±•ï~Cç!É±ç£±Ô!á!èrLºë°â‡Ñ≤£∫ÄÒ≤°∫¿Ù≤1∏vZâÔ7c7jBº´j°∫Yb°∫âj@ºb£∫â±j@º´[°Éudz>ªñ!É∞~BâçÛ±ÇK±z>ªæ!ÇöT±æ!ÖíT±ç€bäB±çï≤!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇN≥=Ùàï•ç≤±ë°çe≥~BÉ„±••a¿@üí!Å≥!ç!Ñ°I£´A¿øI•ç@≤ë°Ée≥!ã°Éø±`!Ö22MJ@!ì!é!Ü!Ü!é!í
7DATA"!É!ç	!è!ó!ê!Ñ!Ü!é!Ñ!Å!Ö!è!ç!Ü!å!é!ç~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@
