#!/bin/sh

CFLAGS_DEBUG = -g -Wall -fstack-protector-all -lpthread -fpic -D DEBUG
CFLAGS = -pipe -march=native -O3 -lpthread -fpic

STATIC_LINK = ar rcsu
STATIC_LINK_DEBUG = $(STATIC_LINK)
SHARED_LINK = gcc -shared 
SHARED_LINK_DEBUG = gcc -shared -g 

SOURCE = suffixArray.c 

OBJECTS = suffixArray.o
OBJECTS_DEBUG = suffixArray-debug.o

STATICLIB = libsuffixarray.a
SHAREDLIB = libsuffixarray.so
STATICLIB_DEBUG = libsuffixarray-debug.a
SHAREDLIB_DEBUG = libsuffixarray-debug.so



##STANDARD BUILD########################################################

all : $(STATICLIB) $(SHAREDLIB)

$(STATICLIB) : $(OBJECTS)
	$(STATIC_LINK) $(STATICLIB) $(OBJECTS)


$(SHAREDLIB) : $(OBJECTS)
	$(SHARED_LINK) $(OBJECTS) -o $(SHAREDLIB)


$(OBJECTS) : $(SOURCE)
	gcc -c $(CFLAGS_DEBUG) $< -o $@

	
##DEBUG#################################################################

debug : $(STATICLIB_DEBUG) $(SHAREDLIB_DEBUG)
	cd test; make


$(STATICLIB_DEBUG) : $(OBJECTS_DEBUG)
	$(STATIC_LINK_DEBUG) $(STATICLIB_DEBUG) $(OBJECTS_DEBUG)


$(SHAREDLIB_DEBUG) : $(OBJECT_DEBUG)
	$(SHARED_LINK_DEBUG) $(OBJECTS_DEBUG) -o $(SHAREDLIB_DEBUG)


$(OBJECTS_DEBUG) : $(SOURCE)
	gcc -c $(CFLAGS_DEBUG) $< -o $@


##AUXILLARY FUNCTIONS###################################################

clean :
	rm -f $(OBJECTS) $(STATICLIB) $(SHAREDLIB) $(OBJECTS_DEBUG)
	rm -f $(STATICLIB_DEBUG) $(SHAREDLIB_DEBUG)
	cd test; make clean
