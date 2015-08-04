##
##	Makefile for building and installing the suffix array libraries
##
## Copyright (C) 2015	Josh Marshall
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.	If not, see <http://www.gnu.org/licenses/>.
##

##.major.minor.fix
#VERSION = .1.0.1

MAJOR = .1
MINOR = .0
FIX = .1

CFLAGS_DEBUG = -g -Wall -fstack-protector-all -lpthread -fpic -D DEBUG
CFLAGS = -pipe -march=native -O3 -lpthread -fpic

STATIC_LINK = ar rcsu
STATIC_LINK_DEBUG = $(STATIC_LINK)
SHARED_LINK = gcc -shared
SHARED_LINK_DEBUG = gcc -shared -g

HEADERS = suffixarray.h
SOURCE = suffixarray.c

OBJECTS = suffixarray.o
OBJECTS_DEBUG = suffixarray-debug.o

STATICLIB = libsuffixarray.a
SHAREDLIB = libsuffixarray.so
STATICLIB_DEBUG = libsuffixarray-debug.a
SHAREDLIB_DEBUG = libsuffixarray-debug.so



##STANDARD BUILD########################################################

all : $(STATICLIB) $(SHAREDLIB)


$(STATICLIB) : $(OBJECTS)
	$(STATIC_LINK) $@ $(OBJECTS)


$(SHAREDLIB) : $(OBJECTS)
	$(SHARED_LINK) $(OBJECTS) -o $@


$(OBJECTS) : $(SOURCE) $(HEADERS)
	gcc -c $(CFLAGS_DEBUG) $< -o $@


##DEBUG#################################################################

debug : $(STATICLIB_DEBUG) $(SHAREDLIB_DEBUG)
	cd test; make


$(STATICLIB_DEBUG) : $(OBJECTS_DEBUG)
	$(STATIC_LINK_DEBUG) $(STATICLIB_DEBUG) $(OBJECTS_DEBUG)


$(SHAREDLIB_DEBUG) : $(OBJECT_DEBUG)
	$(SHARED_LINK_DEBUG) $(OBJECTS_DEBUG) -o $(SHAREDLIB_DEBUG)


$(OBJECTS_DEBUG) : $(SOURCE) $(HEADERS)
	gcc -c $(CFLAGS_DEBUG) $< -o $@


##AUXILLARY FUNCTIONS###################################################

clean :
	rm -f *.a *.so *.o
	cd test; make clean

install : $(STATICLIB) $(SHAREDLIB)
	install -C $(HEADERS) /usr/include/$(HEADERS)
	install -C $(SHAREDLIB) /usr/lib/$(SHAREDLIB)$(MAJOR)$(MINOR)$(FIX)
	install -C $(STATICLIB) /usr/lib/$(STATICLIB)$(MAJOR)$(MINOR)$(FIX)
	ln -rsf /usr/lib/$(SHAREDLIB)$(MAJOR)$(MINOR)$(FIX) /usr/lib/$(SHAREDLIB)$(MAJOR)$(MINOR)
	ln -rsf /usr/lib/$(SHAREDLIB)$(MAJOR)$(MINOR) /usr/lib/$(SHAREDLIB)$(MAJOR)
	ln -rsf /usr/lib/$(SHAREDLIB)$(MAJOR) /usr/lib/$(SHAREDLIB)
	ln -rsf /usr/lib/$(STATICLIB)$(MAJOR)$(MINOR)$(FIX) /usr/lib/$(STATICLIB)$(MAJOR)$(MINOR)
	ln -rsf /usr/lib/$(STATICLIB)$(MAJOR)$(MINOR) /usr/lib/$(STATICLIB)$(MAJOR)
	ln -rsf /usr/lib/$(STATICLIB)$(MAJOR) /usr/lib/$(STATICLIB)

uninstall :
	rm -f /usr/include/$(HEADERS)
	rm -f /usr/lib/$(SHAREDLIB)$(MAJOR)$(MINOR)
	rm -f /usr/lib/$(SHAREDLIB)$(MAJOR)$(MINOR)$(FIX) 
	rm -f /usr/lib/$(SHAREDLIB)$(MAJOR) /usr/lib/$(SHAREDLIB)
	rm -f /usr/lib/$(STATICLIB)$(MAJOR)$(MINOR)
	rm -f /usr/lib/$(STATICLIB)$(MAJOR)$(MINOR)$(FIX) 
	rm -f /usr/lib/$(STATICLIB)$(MAJOR) /usr/lib/$(STATICLIB)