CC:=gcc
CFLAGS=-Wall -std=c99
DEBUGF:=-g
RELEASEF:=-O2

INCDIR:=include
SRCDIR:=src
OBJDIR:=obj
BINDIR:=bin

EXECSRC:=execsrc
TESTSRC:=testsrc
TESTBIN:=testbin

DEBUG_POST:=-d
RELEASE_POST:=

# Auto dependency, executable sources and tests sources finder

DEPS:=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(wildcard $(SRCDIR)/*.c))
TESTS:=$(patsubst $(TESTSRC)/%.c, $(TESTBIN)/%, $(wildcard $(TESTSRC)/*.c))
EXECS:=$(patsubst $(EXECSRC)/%.c, $(BINDIR)/%, $(wildcard $(EXECSRC)/*.c))

# Make rules

release: CFLAGS+=$(RELEASEF)
release: execs$(RELEASE_POST)
release: tests$(RELEASE_POST)

debug: CFLAGS+=$(DEBUGF)
debug: execs$(DEBUG_POST)
debug: tests$(DEBUG_POST)

# Make executables

execs$(RELEASE_POST): $(patsubst %, %$(RELEASE_POST), $(EXECS))
execs$(DEBUG_POST): $(patsubst %, %$(DEBUG_POST), $(EXECS))

$(BINDIR)/%$(RELEASE_POST): $(OBJDIR)/%$(RELEASE_POST).o $(patsubst %, %$(RELEASE_POST), $(DEPS))
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/%$(DEBUG_POST): $(OBJDIR)/%$(DEBUG_POST).o $(patsubst %, %$(RELEASE_POST), $(DEPS))
	$(CC) $(CFLAGS) -o $@ $^

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

$(OBJDIR)/%$(RELEASE_POST).o: $(EXECSRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

# For debug

$(OBJDIR)/%$(DEBUG_POST).o: $(SRCDIR)/%.c $(INCDIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR)/%$(DEBUG_POST).o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR)/%$(DEBUG_POST).o: $(TESTSRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR)/%$(DEBUG_POST).o: $(EXECSRC)/%.c
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
	mkdir -p $(EXECSRC)
	mkdir -p $(TESTSRC)
	mkdir -p $(TESTBIN)
