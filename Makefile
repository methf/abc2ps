#
# makefile for abc2ps
#

PROGRAM = abc2ps

# compilation options
#CC = gcc
#CFLAGS = -Wall  -I/usr/demo/SOUND/include 

# using Sun audio library
#CC = cc
#CFLAGS = -I/usr/demo/SOUND/include  -D SUN 
#LDFLAGS = -L/usr/demo/SOUND/lib -laudio -lm

# using supplied audio functions
CC = cc
CFLAGS = 
LDFLAGS = -lm

# installation directories
BINDIR = /usr/local/bin
MANDIR = /usr/local/man/man1


OBJECTS = abc2ps.o parse.o syms.o style.o buffer.o format.o pssubs.o subs.o util.o music.o 
# playback.o


$(PROGRAM): $(OBJECTS)
	$(CC) -o $(PROGRAM) $(OBJECTS) $(LDFLAGS)

%.o: %.c abc2ps.h
	$(CC) $(CFLAGS) -c $*.c

install:
	strip $(PROGRAM)
	mv $(PROGRAM) $(BINDIR)
	gzip -c $(PROGRAM).1 > $(PROGRAM).1.gz
	mv $(PROGRAM).1.gz $(MANDIR)

clean:
	rm *.o $(PROGRAM)
