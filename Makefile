
CC:=gcc
CFLAGS:=-Wall -std=c99 -O2

INCDIR:=include
SRCDIR:=src
OBJDIR:=obj
BINDIR:=bin

TESTSRC:=testsrc
TESTBIN:=testbin

# Make all

all: tests

# Make tests

tests: $(TESTBIN)/center $(TESTBIN)/freq

$(TESTBIN)/center: $(OBJDIR)/center.o $(OBJDIR)/buffer.o $(OBJDIR)/error.o
	$(CC) $(CFLAGS) -o $@ $^

$(TESTBIN)/freq: $(OBJDIR)/freq.o $(OBJDIR)/stable.o $(OBJDIR)/error.o
	$(CC) $(CFLAGS) -o $@ $^

# General rules

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR)/%.o: $(TESTSRC)/%.c
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
