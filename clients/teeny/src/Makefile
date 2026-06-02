# Makefile for TEENY
# Requires https://github.com/z88dk/z88dk
# Requires https://github.com/bkw777/dl2

MODELS = 100 # 200 NEC M10 K85
NAME = TNY
SRC = TEENY.S85

AS = z88dk-z80asm
ASFLAGS := -m8085 -no-synth # -v

# -DHIMEM=addr         # generate a relocated binary with specified HIMEM/END address
XFLAGS :=

# don't delete .CO when building .DO etc.
.SECONDARY:

.PHONY: all
all: $(MODELS)

.PHONY: $(MODELS)
$(MODELS): % : $(NAME)%.DO

# We need the length of the binary to set ORG, but we don't know the length
# until after compile. So we compile once with default org 0 and no CO header
# just to get the length. Then compile again with correct ORG & CO header.
# Also generate map file (-m) to provide the function call addresses,
# and filter .map to produce .calls
$(patsubst %,$(NAME)%.CO,$(MODELS)): $(NAME)%.CO : $(SRC) %.S85
	$(AS) $(ASFLAGS) $(XFLAGS) -DMODEL_$* -b -o=$@ $< && \
	n=$$(wc -c <$@) && \
	$(AS) $(ASFLAGS) $(XFLAGS) -DMODEL_$* -DPRGLEN=$$n -b -o=$@ $<
	@rm *.o

%.DO: %.CO
	@which -s co2ba || echo "Please install https://github.com/bkw777/dl2 for co2ba to generate $@"
	co2ba $< savem >$@

.PHONY: verify_%
verify_%: $(NAME)%.CO
	@cmp -l -b $< ../orig/$< && echo "Built $< is identical to reference ../orig/$<."

.PHONY: load_%
load_%: $(NAME)%.DO
	@which -s dl || echo "Please install https://github.com/bkw777/dl2 to bootstrap $<"
	dl -v -b $<

.PHONY: clean
clean:
	rm -f *.o *.bin *.sym *.lis *.map *.def *.CO *.DO
