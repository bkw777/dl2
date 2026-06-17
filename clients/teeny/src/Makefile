# Makefile for TEENY
# Requires https://github.com/z88dk/z88dk
# Requires https://github.com/bkw777/dl2

MODELS = 100 K85 M10 200 NEC
NAME = TNY
SRC = TEENY.S85

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
# $ make 100 XFLAGS='=-DHIMEM=62000 -DBAUD=9600 -DINCLUDE_DSR'
XFLAGS :=

# don't delete .CO when building .DO
.SECONDARY:

.PHONY: all
all: $(MODELS)

.PHONY: $(MODELS)
$(MODELS): % : $(NAME)%.DO

$(patsubst %,$(NAME)%.CO,$(MODELS)): $(NAME)%.CO : $(SRC) %.S85
	$(AS) $(ASFLAGS) $(XFLAGS) -DMODEL_$* -b -o=$@ $< && \
	$(AS) $(ASFLAGS) $(XFLAGS) -DMODEL_$* -DPRGLEN=`wc -c <$@` -b -o=$@ $<
	@rm *.o
	@case $* in 200|NEC) echo '!NOTICE!  $* does not work. See ../orig/disasm for working $* source' ;; esac

%.DO: %.CO
	@which -s co2ba || echo "Please install https://github.com/bkw777/dl2 for co2ba to generate $@"
	case "$*" in *NEC*) o=bsave ;; *) o=savem ;; esac ;co2ba $< $$o >$@

.PHONY: verify_%
verify_%: $(NAME)%.CO
	@cmp -l $< ../orig/$< && echo "Built $< is identical to reference ../orig/$<."
	@echo "Note: default config no longer replicates the legacy binary exactly,"
	@echo "so it's normal for verify to 'fail' now."

.PHONY: load_%
load_%: $(NAME)%.DO
	@which -s dl || echo "Please install https://github.com/bkw777/dl2 to bootstrap $<"
	dl -v -b $<

.PHONY: clean
clean:
	rm -f *.o *.bin *.sym *.lis *.map *.def *.CO *.DO
