0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-09
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing",N:LOCATE22,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE22,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61695,641,61695,254,"TEENY",128,64
4DATA"a@@ybFºa!ö!öbJºjFºπÔrHºaL±•a!ü≥ç!¢Tr(æçÍ!áa!ì±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìä≥≤µçî±ä!ì±±ñ!ãäè±}äc≤zHº˜ä%±jBº!Ñ!çj@ºç|,a:≥ç!¢TÉL±y!éxq!é!éaLº7cv@âaLº>c∆!ó>!ü!ác˜:ä“±∆c]ÇÃ±o7~E¿!áaJº>ç˛.ö!ì±cEÇ€±aLºç•6çÜ±çÜ±!èLçÜ±MÇı±ÔpaMº!éc>˜âçì-Ç!ì±7câ~EÉw≥çË/Vnç!Ä
5DATA"0a.±wç!é\~Gçw≥Ôç4±vZcQ$¥FIç∑qFOv`cEÇπ±v!Üc0É<±^A~Aç4±7c3Éz≥ë~Dç4±1c!Åç∑qÉz≥ç€bÇ!ì±a!ì±bæ≥çìb•ç™≤jgº!Ö!å) bDºK!ü|C}Ç!É≤!óa˜≤YYYYY>µQ†≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bBº´b@º•çJ,ö!ì±aHºtçﬂ≤jDº´°zLº!èF@:Û9rLºà˜º‰≤[•aNºI>°7cLÉ∆≤^CçI≤•ï~Cç4±çD≤Ô!á!èrLºë°â‡ô≤£∫ÄÜ≤°∫
6DATA"¿â≤1∏vZâÔ7c7jBº´j°∫Yb°∫âj@ºb£∫â±j@º´[°Éudz>ªñ!É∞~Bâçî±Ç!ì±z>ªæ!Çö!ì±æ!Öí!ì±ç€bä!ì±ç™≤!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇc≥=Ùàï•çG≤ë°ç~≥~Bç4±ç<±à••a¿@üí!ö≥!ç!Ñ°I£´A¿øI•çU≤ë°É~≥!ã°ÉT≤~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@!Ö!í!íMJ@
