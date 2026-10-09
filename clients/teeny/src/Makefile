# Makefile for TEENY
# Requires https://github.com/z88dk/z88dk
# Requires https://github.com/bkw777/dl2

NAME = TEENY
SRC = $(NAME).S85
MODELS = 100na K85na M10na M10eu 200na NECna
URL = github.com/bkw777/dl2
CO2BA_CMD = callba      # savem / callbe
CO2BA_CMD_NEC = execba  # bsave / execba

AS = z88dk-z80asm
ASFLAGS := -m8085_strict -no-synth # -v
# Adds -DZ88DK=ver to flags if the assembler is z88dk.
ASFLAGS += $(shell $(AS) |awk '/^Z80 Macro Assembler /{split($$4,a,"-");if (a[1]>0) print "-DZ88DK="a[1]}')

# -DHIMEM=addr		# [MAXRAM] generate a relocated binary with specified END address
# -DINCLUDE_DSR		# include code for DSR check
# -DBAUD=9600		# [19200] 9600 4800 2400 1200 600 300 110 75
# -DCHUNK_LEN=64	# 1-[128]
# Use -DBAUD=9600 if you have a FB-100, FDD19, or Purple Computing drive,
# or remove the solder blob under the little cover on the bottom.
# Example usage from the make command line:
# $ make 100na XFLAGS='-DHIMEM=62000 -DBAUD=9600 -DINCLUDE_DSR'
XFLAGS :=

# don't delete .CO when building .DO
.SECONDARY:

.PHONY: all
all: $(MODELS)

.PHONY: $(MODELS)
$(MODELS): % : $(NAME)_%.DO

$(patsubst %,$(NAME)_%.CO,$(MODELS)): $(NAME)_%.CO : $(SRC) %.S85
	@$(AS) $(ASFLAGS) $(XFLAGS) -DMODEL_$* -b -o=$@ $< && \
	CMD="$(AS) $(ASFLAGS) $(XFLAGS) -DMODEL_$* -DPRGLEN=`wc -c <$@` -b -o=$@ $<" ; \
	echo "$$CMD" ; \
	eval $$CMD
	@rm *.o
	@case $* in 200*|NEC*) echo '!NOTICE!  $* does not work. See ../orig/disasm for working $* source' ;; esac

%.DO: %.CO
	@which -s co2ba || echo "Please install https://github.com/bkw777/dl2 for co2ba to generate $@"
	@case "$*" in *NEC*) o=$(CO2BA_CMD_NEC) ;; *) o=$(CO2BA_CMD) ;; esac ; \
	x="$*" ;while read a b c ;do [ "_$$b" = "_platform:" ] && break || c= ;done < $${x##*_}.S85 ; \
	CMD="PN=$(NAME) COMMENT=\"$$c - $(URL)\" co2ba $< $$o >$@" ; \
	echo "$$CMD" ; \
	eval $$CMD

.PHONY: verify_%
verify_%: $(NAME)_%.CO
	@cmp -l $< ../orig/$< && echo "Built $< is identical to reference ../orig/$<."
	@echo "Note: default config no longer replicates the legacy binary exactly,"
	@echo "so it's normal for verify to 'fail' now."

.PHONY: load_%
load_%: $(NAME)_%.DO
	@which -s dl || echo "Please install https://github.com/bkw777/dl2 to bootstrap $<"
	dl -v -b $<

.PHONY: clean
clean:
	rm -f *.o *.bin *.sym *.lis *.map *.def *.CO *.DO
