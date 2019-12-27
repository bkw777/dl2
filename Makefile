# Makefile for DeskLink+

OS := $(shell uname)
CFLAGS += -O2 -Wall
PREFIX = /usr/local
APP_NAME = dl
APP_LIB_DIR = $(PREFIX)/lib/$(APP_NAME)
APP_DOC_DIR = $(PREFIX)/share/doc/$(APP_NAME)

DEFAULT_TTY = /dev/ttyUSB0
LOADER_FILE = $(APP_LIB_DIR)/LOADER.DO

SOURCES = dl.c dir_list.c
LOADER_SRC = teeny/LOADER.DO
DOCS = teeny/teenydoc.txt dl.do README.txt README.md LICENSE

DEFINES = \
	-DDEFAULT_TTY=$(DEFAULT_TTY) \
	-DLOADER_FILE=$(LOADER_FILE)

ifdef DEBUG
 CFLAGS += -g
else
 CFLAGS += -s
endif

ifeq ($(OS),Darwin)
else
 LDFLAGS += -lutil
endif

.PHONY: all
all: $(APP_NAME)

$(APP_NAME): $(SOURCES)
	gcc $(CFLAGS) $(DEFINES) $(SOURCES) $(LDFLAGS) -o $(@)

install: $(APP_NAME) $(LOADER_SRC) $(DOCS)
	install -o root -m 0755 -d $(APP_LIB_DIR) $(APP_DOC_DIR)
	install -o root -m 0644 $(LOADER_SRC) $(LOADER_FILE)
	install -o root -m 0644 -t $(APP_DOC_DIR) $(DOCS)
	install -o root -m 0755 $(APP_NAME) $(PREFIX)/bin/$(APP_NAME)

uninstall:
	rm -rf $(APP_LIB_DIR) $(APP_DOC_DIR) $(PREFIX)/bin/$(APP_NAME)

clean:
	rm -f $(APP_NAME)
