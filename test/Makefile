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


CFLAGS = -g -Wall -fstack-protector-all ../libsuffixarray-debug.a -D DEBUG -lpthread -O0

SOURCE = testSA.c testLCP.c testTime.c

EXEC = testSA testLCP testTime


##STANDARD BUILD########################################################

##all : $(EXEC) $(SOURCE)
all :
	gcc testSA.c $(CFLAGS) -o testSA
	gcc testLCP.c $(CFLAGS) -o testLCP
	gcc testTime.c $(CFLAGS) -o testTime

##$(EXEC) : $(SOURCE)
##	gcc $< $(CFLAGS) -o $@

##AUXILLARY FUNCTIONS###################################################

clean :
	rm -f $(EXEC)

cleanAll :
	cd ../ ; make clean

deps :
	cd ../ ; make debug