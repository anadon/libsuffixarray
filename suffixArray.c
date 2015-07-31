//GPLv3 this properly at some point

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "suffixArray.h"


 #define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })



/*
 * WARNING: this function is dependant on toSort containing values in
 * acending order.  Some assumptions allow it to run in O(n/2) time
 * 
 * TODO -- this implementation is still non-optimal
 */
void BWTRadixSort(suffixArray toSetup){
	size_t **storage, storageSublengths[256], storageSubiterators[256], **swap;
	size_t swapSublengths[256], swapSubiterators[256], **tmp;
	unsigned char targetIndex;
	
	const unsigned char *source = toSetup.sequence;
	const size_t length = toSetup.length; 
	
	
	toSetup.bwtArray = (size_t*) malloc(sizeof(size_t) * length);
	for(size_t i = 0; i < length; i++)
		toSetup.bwtArray[i] = i;
	
	swap = malloc(sizeof(size_t*) * 256);
	storage = malloc(sizeof(size_t) * 256);
	
	memset(swap, 0, 256 * sizeof(size_t));
	memset(storage, 0, 256 * sizeof(size_t));
	memset(storageSublengths, 0, 256 * sizeof(size_t));
	memset(storageSubiterators, 0, 256 * sizeof(size_t));
	memset(swapSublengths, 0, 256 * sizeof(size_t));
	
	
	//Non-comparatively sort the items
	for(size_t h = 0; h < length; h++){
		memset(swapSubiterators, 0, 256 * sizeof(size_t));
		
		for(size_t i = 0; i < 256; i++){
			for(size_t j = 0; j < storageSublengths[i]; j++){
				targetIndex = source[(length - 1) - h + storage[i][storageSubiterators[i]]];
				if(swapSubiterators[targetIndex]+1 >= swapSublengths[targetIndex]){
					size_t suggestedSize = min((swapSublengths[targetIndex] * 2 + 1), length);
					swap[targetIndex] = realloc(swap[targetIndex], suggestedSize * sizeof(size_t));
					swapSublengths[targetIndex] = suggestedSize;
				}
				swap[targetIndex][swapSubiterators[targetIndex]++] = storage[i][storageSubiterators[i]++];
			}
		}
			
		//Since we know these are in ascending order we just take the
		//first unsorted element since the rest pf the elements are too
		//short to sort.  We then use this shortcut to speed copying the
		//remainging elements. This should halve the time.
		targetIndex = source[length-h];
		if(swapSubiterators[targetIndex]+1 >= swapSublengths[targetIndex]){
			size_t suggestedSize = min((swapSublengths[targetIndex] * 2 + 1),  length);
			swap[targetIndex] = realloc(swap[targetIndex], suggestedSize * sizeof(size_t));
			swapSublengths[targetIndex] = suggestedSize;
		}
		swap[targetIndex][swapSubiterators[source[length-h]]] = toSetup.bwtArray[h];
		
		tmp = storage;
		storage = swap; 
		swap = tmp;
	}
	
	for(int i = 0; i < 256; i++)	free(swap[i]);
	free(swap);
	tmp = swap = NULL;
		
	size_t placeIndex = 0;
	for(short i = 0; i < 256; i++){
		size_t queueLength = storageSubiterators[i];//as a side effect, 
		//storageSubiterators contains the index+1 of the last item in the 
		//array.  It's cached here because gcc can't optimize this well
		//enough right now.
		for(size_t j = 0; j < queueLength; j++){
			toSetup.bwtArray[placeIndex++] = storage[i][j];
		}
		free(storage[i]);
	}
	free(storage);
	
}


	/*
	 * 
	 * NOTE TODO FIXME: this is stupidly parallelizable; openCL this
	 * This code is non-optimal
	 */
void AppendIdentInit(suffixArray toSetup){
	const unsigned char *source = toSetup.sequence;
	const size_t inputLength = toSetup.length;
	
	size_t *appendIdent = (size_t*) malloc(sizeof(size_t) * inputLength);
	
	appendIdent[0] = 0;	//can't have and first characters in common with 
												//nothing
	for(size_t i = 1; i < inputLength; i++){
		size_t shorterIndex = inputLength - i;
		appendIdent[i] = 0;//NOTE TODO FIXME this is possible in O(n) time, not O(n^2) like this
		for(appendIdent[i] = 0; appendIdent[i] < shorterIndex; appendIdent[i]++){
			if(source[i-1 + appendIdent[i]] != source[i + appendIdent[i]])
				break;
		}
	}
	
	toSetup.appendIdent = appendIdent;
	
}
	
	
suffixArray makeSuffixArray(const unsigned char* inputSequence, 
																							const size_t inputLength){
		suffixArray toReturn = {inputSequence, NULL, inputLength, NULL, NULL};
		
		if(inputLength == 0 || inputSequence == NULL){	exit(-1);	}
		
		//thread 1 this
		toReturn.bwtArray = (size_t*) malloc(sizeof(size_t) * inputLength);
		for(size_t i = 0; i < inputLength; i++)
			toReturn.bwtArray[i] = i;
		BWTRadixSort(toReturn);
		
		//thread 2 this
		toReturn.appendIdent = (size_t*) malloc(sizeof(size_t) * inputLength);
		AppendIdentInit(toReturn);
		
		
		//thread 0 -- do other stuff before waiting on other threads to 
		//merge in.
		toReturn.internalSequence = NULL;
		
		//merge threads
	return toReturn;
}
	
void freeSuffixArray(suffixArray toFree){
	if(toFree.internalSequence) free(toFree.internalSequence);
	free(toFree.bwtArray);
	free(toFree.appendIdent);
}
	

