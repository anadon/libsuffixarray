/*Test the Burrow-Wheeler Transformation table in the suffix array 
 * implementation
 * 
 *   Copyright (C) 2015  Josh Marshall
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#include "suffixarray.h"


int main(int argc, char** argv){
  
	FILE *fd = fopen(argv[1], "r");
	fseek(fd, 0, SEEK_END);
	size_t length = ftell(fd);
	rewind(fd);
	
	void *sequence = malloc(length);
	fread(sequence, 1, length, fd);
	fclose(fd);
	
  SuffixArray toTest = makeSuffixArray((unsigned char*) sequence, length);
	
	freeSuffixArray(&toTest);
	free(sequence);
	
  return 0;
}