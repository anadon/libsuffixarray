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
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include "suffixarray.h"

#ifdef DEBUG
#include <stdio.h>
//#undef DEBUG
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


typedef struct recursiveBucketSortArgs{
  const unsigned char *source;
  const size_t length;
  size_t numSequences;
  size_t depth;
  size_t *toSort;
}recursiveBucketSortArgs;


void *recursiveBucketSort(void *input){
  recursiveBucketSortArgs *args = input;
  size_t *nextNumInBucket;
  size_t *bucketIndexArray;
  size_t **nextBuckets;
  size_t endedSequence = 0;
  size_t toReturnIndex = 0;
	size_t j;
  short numThreads = 0, nextThread = 0;
  pthread_t *threads;
	recursiveBucketSortArgs *toPass;
  
#ifdef DEBUG
  char *str1 = "Sorting %lu items at depth %lu { %lu:%c";
  char *str2 = ", %lu:%c";
  char *str3 = " } \n";
  size_t strLength = strlen(str1) + (args->numSequences * strlen(str2) * 3) + strlen(str3) + 100;
  char *toPrint = calloc(sizeof(char), strLength);
  char *tmp = calloc(sizeof(char), strLength);
  sprintf(tmp, str1, args->numSequences, args->depth, args->toSort[0], 
                                        args->source[args->toSort[0]]);
  strcat(toPrint, tmp);
  
  for(size_t i = 1; i < args->numSequences; i++){
    memset(tmp, 0, strLength);
    sprintf(tmp, str2, args->toSort[i], args->source[args->toSort[i]]);
    strcat(toPrint, tmp);
  }
  memset(tmp, 0, strLength);
  sprintf(tmp, str3);
  strcat(toPrint, tmp);
  
  write(fileno(stdout), toPrint, strlen(toPrint));
  free(tmp);
  free(toPrint);
#endif
  
  nextNumInBucket = malloc(sizeof(size_t) * 256);
head:
	
	//handle ultra-simple cases of sorting.
	if(args->numSequences == 1) return args->toSort;
  
	for(j = 0; j < args->numSequences; j++){
		if(args->toSort[j] + args->depth >= args->length){
			endedSequence = args->toSort[j];
			j++;
			break;
		}
	}
	for(; j < args->numSequences; j++)
		args->toSort[j-1] = args->toSort[j];
	
	if(endedSequence != 0){
		args->numSequences--;
		args->toSort[args->numSequences] = endedSequence;
		if(args->numSequences == 1){
			return args->toSort;
		}
	}
	
  //prescan for allocation
  memset(nextNumInBucket, 0, sizeof(size_t) * 256);
  for(size_t i = 0; i < args->numSequences; i++)
    nextNumInBucket[args->source[args->depth + args->toSort[i]]]++;
	
  for(short i = 0; i < 256; i++)
		if(nextNumInBucket[i] > 1)
      numThreads++;

  
	//scan for all same symbol early exit condition
	for(short i = 0; i < 256; i++){
		if(nextNumInBucket[i] == args->numSequences){
			args->depth++;
			//return recursiveBucketSort(args);
			goto head;//a hack to critically reduce stack overhead
		}
	}

  //allocate space
	nextBuckets = malloc(sizeof(size_t*) * 256);
  for(short i = 0; i < 256; i++)
		if(nextNumInBucket[i] > 0)
			nextBuckets[i] = malloc(sizeof(size_t) * nextNumInBucket[i]);
  
  threads = malloc(sizeof(pthread_t) * numThreads);
	toPass = malloc(sizeof(recursiveBucketSortArgs) * numThreads);
  
  //partially sort
	bucketIndexArray = calloc(sizeof(size_t), 256);
  for(size_t i = 0; i < args->numSequences; i++){
		unsigned char target = args->source[args->depth + args->toSort[i]];
		nextBuckets[target][bucketIndexArray[target]++] = args->toSort[i];
  }
	free(bucketIndexArray);
  
  //delegate remaining sorting
	nextThread = 0;
  for(short i = 0; i < 256; i++){
    if(nextNumInBucket[i] > 1){
      recursiveBucketSortArgs tmp = {args->source, args->length, 
                    nextNumInBucket[i], args->depth+1, nextBuckets[i]};
      memcpy(&toPass[nextThread], &tmp, sizeof(recursiveBucketSortArgs));
      pthread_create(&threads[nextThread], NULL, recursiveBucketSort, &toPass[nextThread]);
      nextThread++;
    }
  }

  //integrate sorted arrays
  nextThread = 0;
	toReturnIndex = 0;
  for(short i = 0; i < 256; i++){
		if(nextNumInBucket[i] == 1){
      args->toSort[toReturnIndex++] = nextBuckets[i][0];
      free(nextBuckets[i]);  nextBuckets[i] = NULL;
    }else if(nextNumInBucket[i] > 1){
      size_t *sorted;
      pthread_join(threads[nextThread++], (void*) &sorted);
      for(j = 0; j < nextNumInBucket[i]; j++){
        args->toSort[toReturnIndex++] = sorted[j];
      }
      free(nextBuckets[i]);  nextBuckets[i] = NULL;
    }
  }
  free(threads);  threads = NULL;
  free(toPass);  toPass = NULL;
	free(nextBuckets);
	free(nextNumInBucket);
  
  return args->toSort;
}


/***********************************************************************
 * WARNING: this function may be incorrect.  
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
size_t* BWTRadixSort(const unsigned char *source, const size_t length){
  recursiveBucketSortArgs toPass = {source, length, length, 0, malloc(sizeof(size_t) * length)};

  for(size_t i = 0; i < length; i++)  toPass.toSort[i] = i;
  
  size_t *toReturn = recursiveBucketSort(&toPass);
  for(size_t i = 0; i < length; i++){
    toReturn[i] = (toReturn[i] + length - 1)%length;
  }

  return toReturn;
}


/***********************************************************************
 * NOTE TODO FIXME: this is stupidly parallelizable; openCL this
 * This code is non-optimal
 *
 * This function is used to determine the longest common sequence from
 * the start of 2 nieghboring indexes.  This function should only be
 * called once over the lifetime of a suffixArray from makeSuffixArray()
 **********************************************************************/
size_t* AppendIdentInit(const unsigned char *source, const size_t length, const size_t *bwtArray){

  size_t *appendIdent = (size_t*) malloc(sizeof(size_t) * length);

  appendIdent[0] = 0; //can't have and first characters in common with
                        //nothing
  for(size_t i = 1; i < length; i++){
    size_t maxIndex = max((bwtArray[i-1]+1) % length, (bwtArray[i]+1) % length);
    //NOTE TODO FIXME this is possible in O(n) time, not O(n^2) like this
    for(appendIdent[i] = 0; appendIdent[i] + maxIndex < length;
                                                      appendIdent[i]++){
      if(source[(bwtArray[i-1] + appendIdent[i] + 1) % length] !=
                      source[(bwtArray[i] + appendIdent[i] + 1) % length])
        break;
    }
  }

  return appendIdent;
}


////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS  //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


suffixArray makeSuffixArray(const unsigned char* inputSequence,
                                              const size_t inputLength){

  assert(inputLength > 0);
  assert(inputSequence != NULL);

  const size_t *bwtArray = BWTRadixSort(inputSequence, inputLength);
  const size_t *LCPArray = AppendIdentInit(inputSequence, inputLength, bwtArray);

  suffixArray toReturn = {inputSequence, false, inputLength, bwtArray, LCPArray};
  return toReturn;
}


suffixArray copySequenceToLocal(suffixArray toMod){
  assert(false == toMod.doIOwnSequence);
  
  unsigned char *tmp = malloc(sizeof(size_t) * toMod.length);
  memcpy(tmp, toMod.sequence, sizeof(size_t) * toMod.length);
  
  suffixArray toReturn = {tmp, true, toMod.length, toMod.bwtArray, toMod.LCPArray};
  return toReturn;
}


void freeSuffixArray(suffixArray *toFree){
  suffixArrayCaster *force = (suffixArrayCaster*) toFree;
  if(force->doIOwnSequence) free(force->sequence);
  free(force->bwtArray);
  free(force->LCPArray);
}


#ifdef DEBUG
void printSuffixArrayContainer(suffixArray toDump){
  printf("i\tsuftab\tlcptab\tbwttab\tSsuftab[i]\n"); fflush(stdout);
  for(size_t i = 0; i < toDump.length; i++){
    printf("%lu\t", i); fflush(stdout);
    printf("%lu\t", toDump.bwtArray[i]); fflush(stdout);
    printf("%lu\t", toDump.LCPArray[i]); fflush(stdout);
    printf("%c\t", toDump.sequence[toDump.bwtArray[i]]); fflush(stdout);
    for(size_t j = (1 + toDump.bwtArray[i]) % toDump.length; j < toDump.length; j++){
      printf("%c", toDump.sequence[j]); fflush(stdout);
    }
    printf("\n");  fflush(stdout);
  }
}
#endif
