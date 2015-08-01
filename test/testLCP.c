/*Test the longest common prefix table in the suffix array 
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
#include "../suffixArray.h"

int main(int argc, char** argv){
	
	char *original = argv[1];
	char *expected = argv[2];
	
	printf("Constructing suffix array...\n"); fflush(stdout);
	
	suffixArray toTest = makeSuffixArray((unsigned char*) original, strlen(original));
	
	printf("BWT array construction is "); fflush(stdout);
	
	for(int i = 0; i < strlen(original); i++){
		if(original[toTest.bwtArray[i]] != expected[i]){
			printf("invalid!\n");
			printf("Expected %s\n", expected);
			printf("Recieved ");
			for(int k = 0; k < strlen(original); k++){
				printf("%c", original[toTest.bwtArray[k]]);
				fflush(stdout);
			}
			
			printf("\n");
			
			return 1;
		}
	}
	printf("valid!\n");
	return 0;
}