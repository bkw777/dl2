#!/bin/bash
# Read a .CO file and generate a BASIC loader
# Brian K. White <b.kenyon.w@gmail.com>

LANG=C

: ${FIRST:=0}
: ${LINE_GAP:=1}
: ${LINE_LEN:=256}
: ${UNSAFE:=0 1 2 3 4 5 6 7 8 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 34}
: ${EDITSAFE:=false}
: ${METHOD:=A}
: ${ESC:=!}
: ${XOR:=128}

COFN=$1 ;shift
ACTION=${1^^} ;shift
PN=${COFN##*/} ;PN=${PN:0:6} ;PN=${PN%%.*}

typeset -i i b SUM TOP END EXE LEN n g q c=${XOR}
typeset -ia d=()

printf -v q '%u' "'$ESC" ;UNSAFE+=" $q"
$EDITSAFE && UNSAFE+=" 127"
readonly g=$LINE_GAP u=",${UNSAFE// /,}," ESC q c
n=$FIRST

abrt () { printf '%s: Usage\n%s IN.CO [call|exec|callba|execba|savem|bsave] > OUT.DO\n%s\n' "$0" "${0##*/}" "$@" >&2 ;exit 1 ; }

# read a binary file into to global int array d[]
ftoi () {
	[[ -r "$1" ]] || abrt "Can't read \"$1\""
	local -i i= ;local x= LANG=C ;d=()
	while IFS= read -d '' -r -n 1 x ;do printf -v d[i++] '%u' "'$x" ;done < $1
}

###############################################################################

# help
[[ "$COFN" ]] || abrt

# read the .CO file into d[]
ftoi "$COFN"

# parse & discard the .CO header
((TOP=${d[0]}+${d[1]}*256))
((LEN=${d[2]}+${d[3]}*256))
((EXE=${d[4]}+${d[5]}*256))
d=(${d[*]:6})
((LEN==${#d[*]})) || abrt "Corrupt .CO file?\nHeader declares LEN=$LEN\nFile has ${#d[*]} bytes after header"
((END=TOP+LEN-1))
SUM= ;for ((i=0;i<LEN;i++)) { ((SUM+=${d[i]})) ; }

# loader
printf '%u%c%s - loader: co2ba.sh b.kenyon.w@gmail.com %(%F)T\r' $n "'" "$PN" -1
case $METHOD in
	A) # Adolph/B9/White encoding - Safe bytes copy unchanged, unsafe write !+byte^128
		printf '%uREADF:CLEAR2,F:DEFINTA-E:DEFSNGF-K:DEFSTRL-O:READF,A,J,G,N:E=%u:M="%c":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing "N"   0%%"\r' $n $c $ESC
		printf '%uREADL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:POKEI,B:D=0:I=I+1:K=K+B:NEXT:?@18,USING"###%%";(I-F)*100/A:IFI<=HTHEN%u\r' $((++n*g)) $n
	;;
	B) # Same as A but avoids using IF in the inner loop, but actually runs slower
		printf '%uREADF:CLEAR2,F:DEFINTA-E,O-P:DEFSNGF-K:DEFSTRL-N:READF,A,J,G,N:E=%u:M="":C=0:I=F:H=F+A-1:K=0:D=0:O=%u:P=0:CLS:?"Installing "N"   0%%"\r' $n $c $q
		printf '%uREADL:FORC=1TOLEN(L):B=ASC(MID$(L,C,1)):P=SGN(BXORO):B=BXORE*D:POKEI,B:I=I+P:K=K+B*P:D=PXOR1:NEXT:?@18,USING"###%%";(I-F)*100/A:IFI<=HTHEN%u\r' $((++n*g)) $n
	;;
	H) # Classic quasi hex pairs
		typeset -ra h=({a..p})  # hex data output alphabet
		printf '%uREADF:CLEAR2,F:DEFINTA-E:DEFSNGF-K:DEFSTRL-N:READF,A,J,G,N:E=%u:M="":C=0:I=F:H=F+A-1:K=0:CLS:?"Installing "N"   0%%";\r' $n "'${h[0]}"
		printf '%uREADL:FORC=1TOLEN(L)STEP2:B=(ASC(MID$(L,C,1))-E)*16+ASC(MID$(L,C+1,1))-E:POKEI,B:I=I+1:K=K+B:NEXT:?@18,USING"###%%";(I-F)*100/A:IFI<=HTHEN%u\r' $((++n*g)) $n
	;;
	I) # The simplest csv ints
		printf '%uREADF:CLEAR2,F:DEFINTA-E:DEFSNGF-K:DEFSTRL-N:READF,A,J,G,N:H=F+A-1:K=0:CLS:?"Installing "N:FORI=FTOH:READB:POKEI,B:K=K+B:?".";:NEXT:?\r' $n
	;;
esac

printf '%uIFK<>GTHEN?"Bad Checksum":ELSE' $((++n*g))

# action
case "$ACTION" in
	CALL|EXEC) printf '%sJ\r' $ACTION ;;
	SAVEM|BSAVE) printf '?"Please type: NEW":%sN,F,H,J\r' $ACTION ;;
	CALLBA|EXECBA) printf 'M=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":%s"J:CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL\r' ${ACTION:0:4} ;;
	*) printf '?"top "F:?"end "H:?"exe "J\r' ;;
esac

# header
printf '%uDATA%u,%u,%u,%u,"%s"\r' $((++n*g)) $TOP $LEN $EXE $SUM "$PN"

# data
O= o=
for ((i=0;i<LEN;i++)) {

	((${#O})) || {
		O="$((++n*g))DATA" 
		case $METHOD in
			H|I) ;;
			*) O+='"' ;;
		esac
		O+="$o"
	}

	b=${d[i]}

	case $METHOD in
		I) o=$b, ;;
		H) o=${h[b/16]}${h[b%16]} ;;
		*)
			[[ $u = *,${b},* ]] && {
				printf -v o '%03o' $((b^c))
				printf -v o '%c%b' $ESC "\\$o"
			} || {
				printf -v o '%03o' $b
				printf -v o '%b' "\\$o"
			}
			;;
	esac

	((${#O}+${#o}<LINE_LEN)) && {
		O+=$o
		o=
	} || {
		[[ "$METHOD" == "I" ]] && O=${O:0:-1}
		printf '%s\r' "$O"
		O=
	}

}

((${#O})) && {
	[[ "$METHOD" == "I" ]] && O=${O:0:-1}
	printf '%s\r' "$O"
}
