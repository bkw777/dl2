0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-09
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing",N:LOCATE22,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE22,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61622,714,61622,56,"TEENY",128,64
4DATA"a@@ybFºa!ö!öbJºjFºπÔrHºaÉ∞•a!é≥ç!¢Tr(æçÍ!áaJ±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìäô≤µçÓ±äM±±ñ!ãäÈ±}äI≤^FA^BA^DA^HA^JA^LA^NA^PzHº˜äu±ïjBº!Ñ!çj@ºç|,ëa.≥V@Y>ßóßa)≥ç!¢TÉÉ∞y!éxq!é!éaLº7cv@âaLº>c∆!ó>!ü!ác˜:ä,±∆c]Ç&±o7~E¿!áaJº>ç˛.öS±cEÇ5±aLºç•6ç‡±ç‡±!èLç‡±MÇœ±Ôp
5DATA"aMº!éc>˜âçì-ÇS±7câ~EÉ]≥çË/Vnç!Ä0a!à±wç!é\~Gç]≥Ôç!é±vZcQ$¥FIç∑qFOv`cEÇì±v!Üc0ç!ñ±!ázLºæRäd≥8˜â^A~Aç!é±7c3É`≥ë~Dç!é±1c!Åç∑qÉ`≥ç€bÇM±aY±bæ≥çìb•çê≤jgº!Ö!å) bDºK!ü|C}Çi≤!óa›≤YYYYY>µQÜ≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bBº´b@º•çJ,öY±aHºtç≈≤jDº´°zLº!èF@:Û9rLºà˜º ≤[•aNº
6DATA"I>°7cLÉ,≤^CçØ±•ï~Cç!é±çû±Ô!á!èrLºë°â‡ˇ≤£∫ÄÏ≤°∫¿Ô≤1∏vZâÔ7c7jBº´j°∫Yb°∫âj@ºb£∫â±j@º´[°Éudz>ªñ!É∞~BâçÓ±ÇM±z>ªæ!ÇöJ±æ!ÖíJ±ç€bäM±çê≤!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇI≥=Ùàï•ç≠±ë°çm≥~Bç!é±ç!ñ±à¶∞___!üÉa±••a¿@üí	≥!ç!Ñ°I£´A¿øI•çª±ë°Ém≥!ã°É∫±~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É
7DATA"}!ã!å!ì!ëiMJ~`@`!Ö22MJ@!Ü!Ü!ï!Ö!ì!é	!è!ó!ê!Ñ!Ü!é!Ñ!à!ó
