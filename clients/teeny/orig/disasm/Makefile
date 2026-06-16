# Makefile for legacy TEENY for 200 & NEC disassembly
# Requires https://github.com/z88dk/z88dk
# Requires https://github.com/bkw777/dl2

MODELS = 200 NEC
NAME = TNY

AS = z88dk-z80asm
ASFLAGS := -m8085_strict -no-synth # -v
# z88dk-z80asm < 25000 has a bug that it only accepts the zilog syntax for RST.
# If $(AS) is z88dk then add -DZ88DK=ver to flags. Used in m100_common.S85
# fixed as of https://github.com/z88dk/z88dk/commit/db5c165
ASFLAGS += $(shell $(AS) |awk '/^Z80 Macro Assembler /{split($$4,a,"-");if (a[1]>0) print "-DZ88DK="a[1]}')

# -DHIMEM=addr		# [MAXRAM] generate a relocated binary with specified END address
# -DINCLUDE_DSR		# include code to check DSR
# -DBAUD=9600		# [19200] 9600 4800 2400 1200 600 300 110 75
# -DCHUNK_LEN=64	# 1-251, [128]
XFLAGS :=

# don't delete .CO when building to get to .DO
.SECONDARY:

.PHONY: all
all: $(MODELS)

.PHONY: verify
verify: verify_200 verify_NEC

.PHONY: $(MODELS)
$(MODELS): % : $(NAME)%.DO

$(patsubst %,$(NAME)%.CO,$(MODELS)): $(NAME)%.CO : $(NAME)%.S85 ../../src/%.S85
	$(AS) $(ASFLAGS) $(XFLAGS) -b -o=$@ $< && \
	$(AS) $(ASFLAGS) $(XFLAGS) -DPRGLEN=$$(wc -c <$@) -b -o=$@ $<
	@rm *.o

%.DO: %.CO
	@which -s co2ba || echo "Please install https://github.com/bkw777/dl2 for co2ba to generate $@"
	case "$*" in *NEC*) o=bsave ;; *) o=savem ;; esac ;co2ba $< $$o >$@

.PHONY: verify_%
verify_%: $(NAME)%.CO
	@cmp -l $< ../$< && echo "Built $< is identical to reference ../$<."

.PHONY: load_%
load_%: $(NAME)%.DO
	@which -s dl || echo "Please install https://github.com/bkw777/dl2 to bootstrap $<"
	dl -v -b $<

.PHONY: clean
clean:
	rm -f *.o *.bin *.sym *.lis *.map *.def *.CO *.DO
