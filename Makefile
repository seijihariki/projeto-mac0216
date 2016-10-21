
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

POSTP=

# Make rules

release: CFLAGS+=$(RELEASEF)
release: tests$(POSTP)

debug: CFLAGS+=$(DEBUGF)
debug: tests$(POSTP)

# Make tests

tests$(POSTP): $(TESTBIN)/center$(POSTP) $(TESTBIN)/freq$(POSTP) $(TESTBIN)/parse_test$(POSTP)

$(TESTBIN)/center$(POSTP): $(OBJDIR)/center$(POSTP).o $(OBJDIR)/buffer$(POSTP).o $(OBJDIR)/error$(POSTP).o
	$(CC) $(CFLAGS) -o $@ $^

$(TESTBIN)/freq$(POSTP): $(OBJDIR)/freq$(POSTP).o $(OBJDIR)/stable$(POSTP).o $(OBJDIR)/error$(POSTP).o
	$(CC) $(CFLAGS) -o $@ $^

$(TESTBIN)/parse_test$(POSTP): $(OBJDIR)/parse_test$(POSTP).o $(OBJDIR)/stable$(POSTP).o $(OBJDIR)/parser$(POSTP).o $(OBJDIR)/error$(POSTP).o $(OBJDIR)/asmtypes$(POSTP).o $(OBJDIR)/optable$(POSTP).o
	$(CC) $(CFLAGS) -o $@ $^

# General rules

$(OBJDIR)/%$(POSTP).o: $(SRCDIR)/%.c $(INCDIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR)/%$(POSTP).o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR)/%$(POSTP).o: $(TESTSRC)/%.c
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
