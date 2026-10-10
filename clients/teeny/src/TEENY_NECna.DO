0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-10
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing",N:LOCATE22,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE22,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61708,628,61708,85,"TEENY",128,64
4DATA"a@@ybFºa!ö!öbJºjFºπÔrHºaY±•a ≥ç!¢Tr(æçÍ!áa ±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìä®≤µç…±ä ±±ñ!ãäL≤}ä^≤zHº˜ä2±jBº!Ñ!çj@ºç|,a;≥ç!¢TÉY±y!éxq!é!éaLº7cv@âçË/Vnç!Ä0a;±wç!é\~Gçl≥Ôç¡±vZcQ$¥FIç∑qFOv`cEÇÓ±v!Üc0aLº>c∆!ó>!ü!ác˜:äè±∆c]Çâ±o7~E¿!áaJº>ç˛.ö ±cEÇò±aLºç•6çC≤
5DATA"çC≤!èLçC≤MÇ≤±ÔpaMº!éc>˜âçì-Ç ±7câ~EÉl≥^A~Aç¡±7c3Éo≥ç€bÇ ±çìb•çü≤jgº!Ö!å) bDºK!ü|C}Çx≤!óaÏ≤YYYYY>µQï≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bBº´b@º•çJ,ö ±aHºtç‘≤jDº´°zLº!èF@:Û9rLºà˜ºŸ≤[•aNºI>°7cLÉ;≤^CçS≤•ï~Cç¡±ç˘±Ô!á!èrLºë°â‡é≤£∫Ä˚≤°∫¿˛≤1∏vZâÔ7c7jBº´j°∫Yb°∫âj@ºb£∫â±j@
6DATA"º´[°Éudz>ªñ!É∞~Bâç…±Ç ±z>ªæ!Çö ±æ!Öí ±ç€bä ±çü≤!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇX≥=Ùàï•çQ≤ë°çs≥~Bç¡±ç˘±à••a¿@üí!è≥!ç!Ñ°I£´A¿øI•ç!í≥ë°És≥!ã°ë~Dç¡±1c!Åç∑qÉo≥~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@!Ö!íMJ@
