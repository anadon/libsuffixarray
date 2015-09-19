#!/bin/sh
#
# Tests Makefile
#
# Copyright (C) 2015	Josh Marshall
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.	If not, see <http://www.gnu.org/licenses/>.
#


CFLAGS = -g -Wall -fstack-protector-all libsuffixarray-debug.a -D DEBUG -O0

SOURCE = testSA.c testLCP.c testTime.c

EXEC = testSA testLCP debugTime fastTime


##STANDARD BUILD########################################################

##all : $(EXEC) $(SOURCE)
all : libsuffixarray.a
	gcc testSA.c $(CFLAGS) -o testSA
	gcc testLCP.c $(CFLAGS) -o testLCP
	gcc testTime.c -g -Wall -fstack-protector-all libsuffixarray.a -o lightTimeDebug
	gcc testTime.c $(CFLAGS) -o heavyTimeDebug
	gcc testTime.c libsuffixarray.a -O3 -march=native -o fastTime

##$(EXEC) : $(SOURCE)
##	gcc $< $(CFLAGS) -o $@

##AUXILLARY FUNCTIONS###################################################

clean :
	rm -f $(EXEC) *.a *.o *.so

cleanAll :
	cd ../ ; make clean

deps :
	cd ../ ; make debug
