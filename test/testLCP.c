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