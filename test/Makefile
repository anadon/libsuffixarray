#!/bin/sh

CFLAGS = -g -Wall -fstack-protector-all ../libsuffixarray-debug.a -D DEBUG -lpthread

SOURCE = testBWT.c testLCP.c

EXEC = testBWT testLCP


##STANDARD BUILD########################################################

all : $(EXEC) $(SOURCE)

$(EXEC) : $(SOURCE)
	gcc $< $(CFLAGS) -o $@

##AUXILLARY FUNCTIONS###################################################

clean : 
	rm -f $(EXEC)
