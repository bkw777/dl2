0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-10
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing",N:LOCATE22,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE22,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61701,635,61701,146,"TEENY",128,64
4DATA"a@@ybFºa!ö!öbJºjFºπÔrHºaR±•a!ü≥ç!¢Tr(æçÍ!áa!ô±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìäß≤µç¬±ä!ô±±ñ!ãäE≤}äW≤zHº˜ä+±jBº!Ñ!çj@ºç|,a:≥ç!¢TÉR±y!éxq!é!éaLº7cv@âçË/Vnç!Ä0a4±wç!é\~Gçk≥Ôç:±vZcQ$¥FIç∑qFOv`cEÇÁ±v!Üc0aLº>c∆!ó>!ü!ác˜:äà±∆c]ÇÇ±o7~E¿!áaJº>ç˛.ö!ô±cEÇë±aLºç•
5DATA"6çº±çº±!èLçº±MÇ´±ÔpaMº!éc>˜âçì-Ç!ô±7câ~EÉk≥^A~Aç:±7c3Én≥ç€bÇ!ô±a!ô±bæ≥çìb•çû≤jgº!Ö!å) bDºK!ü|C}Çw≤!óaÎ≤YYYYY>µQî≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bBº´b@º•çJ,ö!ô±aHºtç”≤jDº´°zLº!èF@:Û9rLºà˜ºÿ≤[•aNºI>°7cLÉ:≤^CçL≤•ï~Cç:±çÚ±Ô!á!èrLºë°â‡ç≤£∫Ä˙≤°∫¿˝≤1∏vZâÔ7c7jBº´j°∫Y
6DATA"b°∫âj@ºb£∫â±j@º´[°Éudz>ªñ!É∞~Bâç¬±Ç!ô±z>ªæ!Çö!ô±æ!Öí!ô±ç€bä!ô±çû≤!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇW≥=Ùàï•çJ≤ë°çr≥~Bç:±çÚ±à••a¿@üí!é≥!ç!Ñ°I£´A¿øI•ç!ë≥ë°Ér≥!ã°ë~Dç:±1c!Åç∑qÉn≥~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@!Ö!í!íMJ@
