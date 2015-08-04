/* suffixarray implementation
 *
 * Copyright (C) 2015  Josh Marshall
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

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "suffixArray.h"

#ifdef DEBUG
#include <stdio.h>
#endif


////////////////////////////////////////////////////////////////////////
//  MACROS  ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


////////////////////////////////////////////////////////////////////////
//  PRIVATE FUNCTIONS //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


/*Conditionally enlarge an array, helper function for BTWRadixSort()*/
void resizeArray(const size_t swapSubiterators[256],
                                    const unsigned char *targetIndexIn,
                    size_t swapSublengths[256], const size_t *lengthIn,
                                                size_t **swap){
  const unsigned char targetIndex = *targetIndexIn;
  const size_t length = *lengthIn;

  if(swapSubiterators[targetIndex]+1 < swapSublengths[targetIndex])
    return;

  size_t suggestedSize =
                    min((swapSublengths[targetIndex] * 2 + 1), length);
  swap[targetIndex] =
            realloc(swap[targetIndex], suggestedSize * sizeof(size_t));

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


/***********************************************************************
 * WARNING: this function is dependant on toSort containing values in
 * acending order.  Some assumptions allow it to run faster.  Currently
 * O(n^2).
 *
 * This function is used to determine the start indexes for a
 * Burrow-Wheeler Transformation when applied to the source array in a
 * suffixArray struct.  This should only be called once during the
 * structure's lifetime, by makeSuffixArray().
 *
 * It works by adding indexes sequentially, and using a radix sort to
 * apply a BW transform.  At the end, toSetup's bwtArray will be
 * allocated, and correctly initalized to contain the start indexes for
 * each start of a BW array.
 **********************************************************************/
void BWTRadixSort(suffixArrayContainer *toSetup){
  size_t **storage, *storageSublengths, *storageSubiterators, **swap;
  size_t *swapSublengths, *swapSubiterators, **tmp, *tmp2;
  unsigned char targetIndex;

  const unsigned char *source = toSetup->sequence;
  const size_t length = toSetup->length;



  swap                = calloc(sizeof(size_t*), 256);
  storage             = calloc(sizeof(size_t*), 256);
  storageSublengths   = calloc(sizeof(size_t), 256);
  swapSublengths      = calloc(sizeof(size_t), 256);
  storageSubiterators = calloc(sizeof(size_t), 256);

  swapSubiterators    = malloc(sizeof(size_t) * 256);


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
    printf("==========================================================="
                                                      "=============");
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

  for(int i = 0; i < 256; i++)  free(swap[i]);
  free(swap);
  free(swapSublengths);
  free(swapSubiterators);
  free(storageSublengths);
  swapSublengths = swapSubiterators = storageSublengths = NULL;
  tmp = swap = NULL;

  toSetup->suffixArray = (size_t*) malloc(sizeof(size_t) * length);

  size_t placeIndex = 0;
  for(short i = 0; i < 256; i++){
    size_t queueLength = storageSubiterators[i];
    for(size_t j = 0; j < queueLength; j++){
      toSetup->suffixArray[placeIndex++] = (length - 1 + storage[i][j]) % length;
#ifdef DEBUG
      printf("%c\t", source[storage[i][j]]);
#endif
    }
    free(storage[i]);
  }
  free(storage);
  free(storageSubiterators);

}


/***********************************************************************
 * NOTE TODO FIXME: this is stupidly parallelizable; openCL this
 * This code is non-optimal
 *
 * This function is used to determine the longest common sequence from
 * the start of 2 nieghboring indexes.  This function should only be
 * called once over the lifetime of a suffixArray from makeSuffixArray()
 **********************************************************************/
void AppendIdentInit(suffixArrayContainer *toSetup){
  const unsigned char *source = toSetup->sequence;
  const size_t inputLength = toSetup->length;

  size_t *appendIdent = (size_t*) malloc(sizeof(size_t) * inputLength);

  appendIdent[0] = 0; //can't have and first characters in common with
                        //nothing
  for(size_t i = 1; i < inputLength; i++){
    size_t maxIndex = max((toSetup->suffixArray[i-1]+1) % toSetup->length, (toSetup->suffixArray[i]+1) % toSetup->length);
    //NOTE TODO FIXME this is possible in O(n) time, not O(n^2) like this
    for(appendIdent[i] = 0; appendIdent[i] + maxIndex < inputLength;
                                                      appendIdent[i]++){
			//NOTE TODO FIXME: the following may be broken, but I don't think
			//it is -- I'm just not 100% sure right now.
      if(source[(toSetup->suffixArray[i-1] + appendIdent[i] + 1) % toSetup->length] !=
											source[(toSetup->suffixArray[i] + appendIdent[i] + 1) % toSetup->length])
        break;
    }
  }

  toSetup->LCPArray = appendIdent;
}


////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS  //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


suffixArrayContainer makeSuffixArray(const unsigned char* inputSequence,
                                              const size_t inputLength){
																								
  suffixArrayContainer toReturn = {inputSequence, NULL, inputLength, NULL, NULL};

  if(inputLength == 0 || inputSequence == NULL){  exit(-1); }

  BWTRadixSort(&toReturn);
  AppendIdentInit(&toReturn);

  return toReturn;
}


suffixArrayContainer copySequenceToLocal(suffixArrayContainer toMod){
  if(toMod.internalSequence != NULL) return toMod;

  toMod.internalSequence = malloc(sizeof(size_t) * toMod.length);
  memcpy(toMod.internalSequence, toMod.sequence,
                                        sizeof(size_t) * toMod.length);

  suffixArrayContainer toReturn = {toMod.internalSequence,
								toMod.internalSequence, toMod.length, toMod.suffixArray,
																												toMod.LCPArray};

  return toReturn;
}


void freeSuffixArray(suffixArrayContainer *toFree){
  if(toFree->internalSequence) free(toFree->internalSequence);
  free(toFree->suffixArray);
  free(toFree->LCPArray);
}


#ifdef DEBUG
void printSuffixArrayContainer(suffixArrayContainer toDump){
	printf("i\tsuftab\tlcptab\tbwttab\tSsuftab[i]\n"); fflush(stdout);
	for(size_t i = 0; i < toDump.length; i++){
		printf("%lu\t", i); fflush(stdout);
		printf("%lu\t", toDump.suffixArray[i]); fflush(stdout);
		printf("%lu\t", toDump.LCPArray[i]); fflush(stdout);
		printf("%c\t", toDump.sequence[toDump.suffixArray[i]]); fflush(stdout);
		for(size_t j = (1 + toDump.suffixArray[i]) % toDump.length; j < toDump.length; j++){
			printf("%c", toDump.sequence[j]); fflush(stdout);
		}
		printf("\n");	fflush(stdout);
	}
}
#endif

