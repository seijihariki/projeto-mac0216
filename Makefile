
CC:=gcc
CFLAGS:=-Wall -std=c99 -O2

INCDIR:=include
SRCDIR:=src
OBJDIR:=obj
BINDIR:=bin

TESTSRC:=testsrc
TESTBIN:=$(TESTSRC)/bin

# Make all

all: tests

# Make tests

tests: $(TESTBIN)/justify $(TESTBIN)/freq

$(TESTBIN)/justify: $(OBJDIR)/justify.o $(OBJDIR)/buffer.o
	$(CC) $(CFLAGS) -o $@ $^

$(TESTBIN)/freq: $(OBJDIR)/freq.o $(OBJDIR)/stable.o
	$(CC) $(CFLAGS) -o $@ $^

# General rules

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR)/%.o: $(TESTSRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

# Clean binaries

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/*
	rm -f $(TESTBIN)/*
