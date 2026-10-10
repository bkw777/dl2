0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-10
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing",N:LOCATE22,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE22,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61702,634,61702,241,"TEENY",128,64
4DATA"a@@ybFºa!ö!öbJºjFºπÔrHºaS±•a ≥ç!¢Tr(æçÍ!áa!ö±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìä®≤µç√±ä!ö±±ñ!ãäF≤}äX≤zHº˜ä,±jBº!Ñ!çj@ºç|,a;≥ç!¢TÉS±y!éxq!é!éaLº7cv@âçË/Vnç!Ä0a5±wç!é\~Gçl≥Ôç;±vZcQ$¥FIç∑qFOv`cEÇË±v!Üc0aLº>c∆!ó>!ü!ác˜:äâ±∆c]ÇÉ±o7~E¿!áaJº>ç˛.ö!ö±cEÇí±aLºç•6
5DATA"çΩ±çΩ±!èLçΩ±MÇ¨±ÔpaMº!éc>˜âçì-Ç!ö±7câ~EÉl≥^A~Aç;±7c3Éo≥ç€bÇ!ö±a!ö±bæ≥çìb•çü≤jgº!Ö!å) bDºK!ü|C}Çx≤!óaÏ≤YYYYY>µQï≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bBº´b@º•çJ,ö!ö±aHºtç‘≤jDº´°zLº!èF@:Û9rLºà˜ºŸ≤[•aNºI>°7cLÉ;≤^CçM≤•ï~Cç;±çÛ±Ô!á!èrLºë°â‡é≤£∫Ä˚≤°∫¿˛≤1∏vZâÔ7c7jBº´j°∫Yb
6DATA"°∫âj@ºb£∫â±j@º´[°Éudz>ªñ!É∞~Bâç√±Ç!ö±z>ªæ!Çö!ö±æ!Öí!ö±ç€bä!ö±çü≤!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇX≥=Ùàï•çK≤ë°çs≥~Bç;±çÛ±à••a¿@üí!è≥!ç!Ñ°I£´A¿øI•ç!í≥ë°És≥!ã°ë~Dç;±1c!Åç∑qÉo≥~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@!Ö!íMJ@
