//GPLv3 this properly at some point

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>

#include "suffixArray.h"


#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })



void resizeArray(const size_t swapSubiterators[256], 
																		const unsigned char *targetIndexIn, 
										size_t swapSublengths[256], const size_t *lengthIn,
																								size_t **swap){
	const unsigned char targetIndex = *targetIndexIn;
	const size_t length = *lengthIn;
	
	if(swapSubiterators[targetIndex]+1 < swapSublengths[targetIndex]) return;
	
	size_t suggestedSize = min((swapSublengths[targetIndex] * 2 + 1),  length);
	swap[targetIndex] = realloc(swap[targetIndex], suggestedSize * sizeof(size_t));
	
#ifdef DEBUG
	if(swap[targetIndex] == NULL){
		fprintf(stderr, "ALLOCATION FAILED");
		exit(-ENOMEM);
	}else{
		fprintf(stderr, "ALLOCATION SUCCEEDED\n");
	}
#endif
			
	swapSublengths[targetIndex] = suggestedSize;
}


/*
 * WARNING: this function is dependant on toSort containing values in
 * acending order.  Some assumptions allow it to run in O(n/2) time
 * 
 * TODO -- this implementation is still non-optimal
 */
void *BWTRadixSort(suffixArray *toSetup){
	size_t **storage, *storageSublengths, *storageSubiterators, **swap;
	size_t *swapSublengths, *swapSubiterators, **tmp, *tmp2;
	unsigned char targetIndex;
	
	const unsigned char *source = toSetup->sequence;
	const size_t length = toSetup->length; 
	
	
	
	swap 								= calloc(sizeof(size_t*), 256);
	storage 						= calloc(sizeof(size_t*), 256);
	storageSublengths 	= calloc(sizeof(size_t), 256);
	swapSublengths 			= calloc(sizeof(size_t), 256);
	storageSubiterators = calloc(sizeof(size_t), 256);
	
	swapSubiterators 		= malloc(sizeof(size_t) * 256);

	
	//Non-comparatively sort the items
	for(size_t h = 0; h < length; h++){
		memset(swapSubiterators, 0, 256 * sizeof(size_t));
		size_t earlyLoopExit = 0;
		
		for(size_t i = 0; i < 256; i++){
			if(earlyLoopExit >= h) break;
			for(size_t j = 0; j < storageSubiterators[i]; j++){
				targetIndex = source[((length-1) - h) + storage[i][j]];
				
				resizeArray(swapSubiterators, &targetIndex, swapSublengths, &length, swap);
				
				swap[targetIndex][swapSubiterators[targetIndex]++] = storage[i][j];
			}
			earlyLoopExit += storageSubiterators[i];
		}
#ifdef DEBUG
		for(int i = 0; i < 256; i++){
			if(swapSubiterators[i] == 0) continue;
			printf("[%d]->:\t", i);
			for(int j = 0; j < swapSubiterators[i]; j++){
				printf("%lu\t", swap[i][j]);
			}
			printf("\n");
		}
		printf("========================================================================");
		printf("\n\n");
		fflush(stdout);
#endif
			
		//Since we know these are in ascending order we just take the
		//first unsorted element since the rest pf the elements are too
		//short to sort.  We then use this shortcut to speed copying the
		//remainging elements. This should halve the time.
		targetIndex = source[h];
		
		resizeArray(swapSubiterators, &targetIndex, swapSublengths, &length, swap);
		
		swap[targetIndex][swapSubiterators[targetIndex]++] = h;
		
		tmp = storage;
		storage = swap; 
		swap = tmp;
		
		tmp2 = swapSubiterators;
		swapSubiterators = storageSubiterators;
		storageSubiterators = tmp2;
		
		tmp2 = swapSublengths;
		swapSublengths = storageSublengths;
		storageSublengths = tmp2;
	}
	
	for(int i = 0; i < 256; i++)	free(swap[i]);
	free(swap);
	free(swapSublengths);
	free(swapSubiterators);
	free(storageSublengths);
	swapSublengths = swapSubiterators = storageSublengths = NULL;
	tmp = swap = NULL;
	
	toSetup->bwtArray = (size_t*) malloc(sizeof(size_t) * length);
		
	size_t placeIndex = 0;
	for(short i = 0; i < 256; i++){
		size_t queueLength = storageSubiterators[i];
		for(size_t j = 0; j < queueLength; j++){
			toSetup->bwtArray[placeIndex++] = (length - 1 + storage[i][j]) % length;
#ifdef DEBUG
			printf("%c\t", source[storage[i][j]]);
#endif
		}
		free(storage[i]);
	}
	free(storage);
	free(storageSubiterators);
	
	return NULL;
}


	/*
	 * 
	 * NOTE TODO FIXME: this is stupidly parallelizable; openCL this
	 * This code is non-optimal
	 */
void *AppendIdentInit(suffixArray *toSetup){
	const unsigned char *source = toSetup->sequence;
	const size_t inputLength = toSetup->length;
	
	size_t *appendIdent = (size_t*) malloc(sizeof(size_t) * inputLength);
	
	appendIdent[0] = 0;	//can't have and first characters in common with 
												//nothing
	for(size_t i = 1; i < inputLength; i++){
		size_t maxIndex = max(toSetup->bwtArray[i-1], toSetup->bwtArray[i]);
		appendIdent[i] = 0;//NOTE TODO FIXME this is possible in O(n) time, not O(n^2) like this
		for(appendIdent[i] = 0; appendIdent[i] + maxIndex < inputLength; appendIdent[i]++){
			if(source[toSetup->bwtArray[i-1] + appendIdent[i]] != source[toSetup->bwtArray[i] + appendIdent[i]])
				break;
		}
	}
	
	toSetup->appendIdent = appendIdent;
	
	return NULL;
}
	
	
suffixArray makeSuffixArray(const unsigned char* inputSequence, 
																							const size_t inputLength){
		suffixArray toReturn = {inputSequence, NULL, inputLength, NULL, NULL};
		
		if(inputLength == 0 || inputSequence == NULL){	exit(-1);	}
		
		BWTRadixSort(&toReturn);
		
		AppendIdentInit(&toReturn);
		
		toReturn.internalSequence = NULL;
		
		//merge threads
	return toReturn;
}


suffixArray copySequenceToLocal(suffixArray toMod){
	if(toMod.internalSequence != NULL) return toMod;
	
	toMod.internalSequence = malloc(sizeof(size_t) * toMod.length);
	memcpy(toMod.internalSequence, toMod.sequence, sizeof(size_t) * toMod.length);
	
	suffixArray toReturn = {toMod.internalSequence, toMod.internalSequence, 
											toMod.length, toMod.bwtArray, toMod.appendIdent};
	
	return toReturn;
}

	
void freeSuffixArray(suffixArray toFree){
	if(toFree.internalSequence) free(toFree.internalSequence);
	free(toFree.bwtArray);
	free(toFree.appendIdent);
}
	

