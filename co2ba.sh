#!/bin/bash
# Read a .CO file and generate a BASIC loader
# Brian K. White <b.kenyon.w@gmail.com>

LANG=C

: ${LINE_GAP:=1}
: ${LINE_LEN:=256}
: ${SHIFT:=64}
: ${SIGIL:='!'}    # any of these work: ~!#$%&*`-+=/?,.|:;'  THESE FAIL: @^\_

CO_IN=$1 ;shift
ACTION=${1^^} ;shift

CO=${CO_IN##*/} ;CO=${CO:0:6} ;CO="${CO%%.*}.CO"

typeset -i i b e SUM TOP END EXE LEN n p g=${LINE_GAP}
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
printf '0CLEAR0:READT:CLEAR2,T:DEFINTI,O,C,V,L,W:DEFSNGA,K,S,T,X,E:DEFSTRB,M,D,N:READT,L,X,K,N,O,M:E=T+L-1:A=T:S=0:C=0:B=" ":W=0:CLS:PRINT"Installing "N\r'
printf '%uPRINT@20,CINT((L-(E-A))/L*100)"%%":READD:W=LEN(D):FORI=1TOW:B=MID$(D,I,1):IFB=MTHENC=O:NEXT\r' $((++n*g)) ;((p=n*g))
printf '%uV=ASC(B)-C:POKEA,V:C=0:A=A+1:S=S+V:NEXT:IFA<=ETHEN%u\r' $((++n*g)) $p
printf '%uPRINT:IFS<>KTHENPRINT"Bad Checksum":END\r' $((++n*g))

# action
case "$ACTION" in
	CALL|EXEC) printf '%u%sX\r' $((++n*g)) $ACTION ;;
	SAVEM|BSAVE) printf '%uPRINT"Done. Please type: NEW":%sN,T,E,X:NEW\r' $((++n*g)) $ACTION ;;
	*) printf '%uPRINT"top "T:PRINT"end "E:PRINT"exe "X\r' $((++n*g)) ;;
esac

# header
printf '%uDATA%u,%u,%u,%u,"%s",%u,"%c"\r' $((++n*g)) $TOP $LEN $EXE $SUM "$CO" $SHIFT "${SIGIL}"

# data
printf -v e '%u' "'${SIGIL}"
O= o=
for ((i=0;i<LEN;i++)) {

	((${#O})) || printf -v O '%uDATA"%s' $((++n*g)) "$o"

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
		printf '%s\r' "$O"
		O=
	}
}
((${#O})) && printf '%s\r' "$O"
