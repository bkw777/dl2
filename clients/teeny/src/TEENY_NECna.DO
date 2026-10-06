0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-06
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing",N:LOCATE22,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE22,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61616,720,61616,177,"TEENY",128,64
4DATA"a@@ybFºa!ö!öbJºjFºπÔrHºa˝∞•a%≥ç!¢Tr(æçÍ!áaY±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìä£≤µç¯±äG±±ñ!ãäÊ±}äS≤^@A^CA^MA^FA^KA^GA^DA^OzHº˜äo±ïjBº!Ñ!çj@ºç|,ëa!í≥V@Y>ßóßa!ã≥ç!¢TÉ˝∞y!éxq!é!éaLº7cv@âaLº>c∆!ó>!ü!ác˜:ä&±∆c]Ç ±o7~E¿!áaJº>ç˛.öM±cEÇ/±aLºç•6ç⁄±ç⁄±!èLç⁄±MÇ…±Ô
5DATA"paMº!éc>˜âçì-öM±ÇJ±7câ~Eç!à±ç!ê±à¶∞___!üÉ[±çË/Vnç!Ä0a!Ç±wç!é\~GçË±Ôç!à±vZcQ$¥FIç∑qFOv`cEÇù±v!Üc0ç!ê±!ázLºæRäÔ±8˜â^A~Aç!à±7c3ÉÎ±ë~Dç!à±1c!Åç∑qÉÎ±ç€bÇP±aV±bæ≥çìb•çö≤jgº!Ö!å) bDºK!ü|C}Çs≤!óaÁ≤YYYYY>µQê≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bBº´b@º•çJ,öS±aHºtçœ≤jDº´°zLº
6DATA"!èF@:Û9rLºà˜º‘≤[•aNºI>°7cLÉ6≤^Cçπ±•ï~Cç!à±ç®±Ô!á!èrLºë°â‡â≤£∫Äˆ≤°∫¿˘≤1∏vZâÔ7c7jBº´j°∫Yb°∫âj@ºb£∫â±j@º´[°Éudz>ªñ!É∞~Bâç¯±ÇP±z>ªæ!ÇöY±æ!ÖíY±ç€bäG±çö≤!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇS≥=Ùàï•ç∑±ë°çj≥~BÉË±••a¿@üí!Ü≥!ç!Ñ°I£´A¿øI•çE≤ë°Éj≥!ã°ÉD≤`!Ö22MJ@!ì!é!Ñ!Ü!Ü!å	
7DATA"!è!ç!ó!ê!Å!Ö!É!ç!é!ç!é!í~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@
