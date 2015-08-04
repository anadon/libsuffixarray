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
#include <stdlib.h>

#include "../suffixarray.h"

int main(int argc, char** argv){
  
  char *original = argv[1];
  
  printf("Constructing suffix array...\n"); fflush(stdout);
  
  suffixArrayContainer toTest = makeSuffixArray((unsigned char*) original, strlen(original));
  
  printf("LCP array construction is "); fflush(stdout);
  
  for(int i = 0; i < strlen(original); i++){
    if(toTest.LCPArray[i] != atoi(argv[i+2])){
      printf("invalid!\n");
      printf("Expected ");
			for(int k = 0; k < strlen(original); k++){
				printf("%s", argv[k+2]);
			}
      printf("\nRecieved ");
      for(int k = 0; k < strlen(original); k++){
        printf("%lu", toTest.LCPArray[k]);
        fflush(stdout);
      }
      printf("\n");
      
			printSuffixArrayContainer(toTest);
			
      return 1;
    }
  }
  printf("valid!\n");
  return 0;
}