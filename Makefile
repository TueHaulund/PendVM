SHELL = /bin/sh

#prefix=/home/joule/deberg/pendulum/pendvm

CC=gcc
INSTALL=install
LATEX=latex
DVIPS=dvips

DEBUG=-DDEBUG
CFLAGS=-g -std=c89 -Wno-pointer-sign
LDFLAGS=
LIBS=

OBJS=main.o memory.o machine.o commands.o pal_parse.o

all: pendvm

.c.o:
	$(CC) $(DEBUG) $(CFLAGS) -c $<

pendvm: $(OBJS)
	$(CC) $(OBJS) $(LIBS) $(LDFLAGS) -o $@

pendvm.dvi: pendvm.tex
	$(LATEX) $<

pendvm.ps: pendvm.dvi
	$(DVIPS) -o $@ $<

clean:
	rm -f $(OBJS) pendvm pendvm.dvi pendvm.aux
