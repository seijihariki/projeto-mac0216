CC:=gcc
CFLAGS=-Wall -std=c99
DEBUGF:=-g
RELEASEF:=-O2

INCDIR:=include
SRCDIR:=src
OBJDIR:=obj
BINDIR:=bin

TESTSRC:=testsrc
TESTBIN:=testbin

DEBUG_POST:=-d
RELEASE_POST:=

# Auto dependency and tests finder

DEPS:=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(wildcard $(SRCDIR)/*.c))
TESTS:=$(patsubst $(TESTSRC)/%.c, $(TESTBIN)/%, $(wildcard $(TESTSRC)/*.c))

# Make rules

release: CFLAGS+=$(RELEASEF)
release: tests$(RELEASE_POST)

debug: CFLAGS+=$(DEBUGF)
debug: tests$(DEBUG_POST)

# Make tests

tests$(RELEASE_POST): $(patsubst %, %$(RELEASE_POST), $(TESTS))
tests$(DEBUG_POST): $(patsubst %, %$(DEBUG_POST), $(TESTS))

$(TESTBIN)/%$(RELEASE_POST): $(OBJDIR)/%$(RELEASE_POST).o $(patsubst %, %$(RELEASE_POST), $(DEPS))
	$(CC) $(CFLAGS) -o $@ $^

$(TESTBIN)/%$(DEBUG_POST): $(OBJDIR)/%$(DEBUG_POST).o $(patsubst %, %$(RELEASE_POST), $(DEPS))
	$(CC) $(CFLAGS) -o $@ $^

# General rules

# For release

$(OBJDIR)/%$(RELEASE_POST).o: $(SRCDIR)/%.c $(INCDIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR)/%$(RELEASE_POST).o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR)/%$(RELEASE_POST).o: $(TESTSRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

# For debug

$(OBJDIR)/%$(DEBUG_POST).o: $(SRCDIR)/%.c $(INCDIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR)/%$(DEBUG_POST).o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR)/%$(DEBUG_POST).o: $(TESTSRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

# Upload to git

upload: clean
	git add --all
	git commit
	git push

# Clean binaries

clean:
	rm -rf $(OBJDIR)/*.o
	rm -rf $(BINDIR)/*
	rm -rf $(TESTBIN)/*

# Configure folder structure

configure:
	mkdir -p $(INCDIR)
	mkdir -p $(SRCDIR)
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)
	mkdir -p $(TESTSRC)
	mkdir -p $(TESTBIN)
