#!/bin/bash
# Read a .CO file and generate a BASIC loader
# Brian K. White <b.kenyon.w@gmail.com>

LANG=C

: ${LINE_LEN:=256}
: ${SHIFT:=64}
: ${SIGIL:='!'}    # any of these work: ~!#$%&*`-+=/?,.|:;'  THESE FAIL: @^\_

CO_IN=$1 ;shift
ACTION=${1^^} ;shift

CO=${CO_IN##*/} ;CO=${CO:0:6} ;CO="${CO%%.*}.CO"

typeset -i i b e SUM TOP END EXE LEN LN
typeset -a d=()

abrt () { printf '%s: Usage\n%s IN.CO [call|exec|savem|bsave] > OUT.DO\n%s\n' "$0" "${0##*/}" "$@" >&2 ;exit 1 ; }

# read a binary file into to global int array d[]
ftoi () {
	[[ -r "$1" ]] || abrt "Can't read \"$1\""
	local -i i= ;local x= LANG=C ;d=()
	while IFS= read -d '' -r -n 1 x ;do printf -v d[i++] '%u' "'$x" ;done < $1
}

###############################################################################

# help
[[ "$CO_IN" ]] || abrt

# read the .CO file into d[]
ftoi "$CO_IN"

# parse & discard the .CO header
((TOP=${d[0]}+${d[1]}*256))
((LEN=${d[2]}+${d[3]}*256))
((EXE=${d[4]}+${d[5]}*256))
d=(${d[*]:6})
((LEN==${#d[*]})) || abrt "Corrupt .CO file?\nHeader declares LEN=$LEN\nFile has ${#d[*]} bytes after header"
((END=TOP+LEN-1))
SUM= ;for ((i=0;i<LEN;i++)) { ((SUM+=${d[i]})) ; }

# loader
printf '0%c%s - loader: co2ba.sh b.kenyon.w@gmail.com %(%F)T\r' "'" "$CO" -1
printf '0CLEAR0:READT:CLEAR2,T:READT,L,X,S,N$,O%%,M$:E=T+L-1:A=T:K=0:C%%=0:CLS:PRINT"Installing "N$" ..";\r'
printf '1PRINT".";:READD$:D%%=LEN(D$):FORI%%=1TOD%%:B$=MID$(D$,I%%,1):IFB$=M$THENC%%=1:GOTO3\r'
printf '2B%%=ASC(B$)-O%%*C%%:POKEA,B%%:C%%=0:A=A+1:K=K+B%%\r'
printf '3NEXTI%%:IFA<=ETHEN1\r'
printf '4PRINT:IFK<>STHENPRINT"BAD CHECKSUM":END\r'
LN=4

# action
case "$ACTION" in
	CALL|EXEC) printf '%u%sX\r' $((++LN)) $ACTION ;;
	SAVEM|BSAVE) printf '%uPRINT"Done. Please type: NEW":%sN$,T,E,X\r' $((++LN)) $ACTION ;;
	*) printf '%uR$=CHR$(13):PRINT"top",T,R$"end",E,R$"exe",X,R$"Please type: NEW"\r' $((++LN)) ;;
esac

# header
printf '%uDATA%u,%u,%u,%u,"%s",%u,"%c"\r' $((++LN)) $TOP $LEN $EXE $SUM "$CO" $SHIFT "${SIGIL}"

# data
printf -v e '%u' "'${SIGIL}"
O= o=
for ((i=0;i<LEN;i++)) {

	((${#O})) || printf -v O '%uDATA"%s' $((++LN)) "$o"

	b=${d[i]}
	(( ( b<32 && b!=9 ) || b==34 || b==e )) && {
		printf -v o '%03o' $((b+SHIFT))
		printf -v o '%c%b' "${SIGIL}" "\\$o"
	} || {
		printf -v o '%03o' $b
		printf -v o '%b' "\\$o"
	}

	((${#O}+${#o}<LINE_LEN)) && {
		O+=$o
		o=
	} || {
		printf '%s"\r' "$O"
		O=
	}
}
((${#O})) && printf '%s"\r' "$O"
