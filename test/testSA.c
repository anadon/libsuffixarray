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
#ifndef BOOLEAN
#define BOOLEAN
#include <stdbool.h>
#endif

#include "suffixarray.h"
#include "qsufsort.h"


void printErrorCase(char *original, char *expected, SuffixArray toTest, int length){
  printf("invalid!\n");
  printf("Expected %s\n", expected);
  printf("Recieved ");
  for(int k = 0; k < length; k++){
    printf("%c", original[(toTest.sa_data[k] + length -1)%length]);
    fflush(stdout);
  }

  printf("\n");
  for(int k = 0; k < length; k++)
    printf("%lu, ", toTest.sa_data[k]);

  printf("\n");
	
	EnhancedSuffixArray tmp = makeEnhancedSuffixArray(toTest);
	printSuffixArrayContainer(tmp);
	freeEnhancedSuffixArray(&tmp);	

}

int main(int argc, char** argv){

  char *original = argv[1];
  char *expected = argv[2];
  size_t length = strlen(original);
	int *qInput = malloc(sizeof(int) * length);
	int *qOutput = malloc(sizeof(int) * length);
	bool wasAnError = false;

  printf("Constructing suffix array...\n"); fflush(stdout);

  SuffixArray toTest = makeSuffixArray((unsigned char*) original, length);
	
	for(ssize_t i = 0; i < length; i++)
		qInput[i] = argv[1][i];
	
	suffixsort(qInput, qOutput, length, 256, 0); 

  printf("BWT array construction is "); fflush(stdout);

  for(size_t i = 0; i < length; i++){
		int left = qOutput[i+1];
		int right = toTest.sa_data[i];
		if(left != right){
			wasAnError = true;
			printf("\nSuffix Array implementations disagree on index %lu\t", i);
			printf("%lu vs %d", toTest.sa_data[i] , qOutput[i+1]);
		}
  }
	printf("\n");
	
	if(wasAnError){
      printErrorCase(original, expected, toTest, length);
			return 1;
	}
  freeSuffixArray(&toTest);
  printf("valid!\n");
  return 0;
}