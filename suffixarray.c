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
#endif


////////////////////////////////////////////////////////////////////////
//  MACROS  ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


////////////////////////////////////////////////////////////////////////
//  PRIVATE FUNCTIONS //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


#ifdef DEBUG
void printBucket(size_t *bucket[256], size_t bucketSize[256]){
  for(int i = 0; i < 256; i++){
    if(bucketSize[i] == 0) continue;
    fprintf(stderr, "(");
    for(size_t j = 0; j < bucketSize[i] - 1; j++){
      fprintf(stderr, "%lu, ", bucket[i][j]);
    }
    fprintf(stderr, "%lu), ", bucket[i][bucketSize[i]-1]);
  }
  fprintf(stderr, "\n");
  fflush(stdout);
}


void printLMSandLS(unsigned char* LMSandLS, size_t length){
  fprintf(stderr, "{");
  for(size_t i = 0; i < length - 2; i++){
    fprintf(stderr, "%c, ", LMSandLS[i] == 1 ? 'L' : (LMSandLS[i] == 2 ? 'S' : 'M'));
  }
  fprintf(stderr, "%c}\n", (LMSandLS[length-1] == 1 ? 'L' : 'S'));
  fflush(stdout);
}
#endif


/***********************************************************************
 * WARNING: this function may be incorrect.
 *
 * This function is used to determine the start indexes for a
 * Burrow-Wheeler Transformation when applied to the source array in a
 * suffixArray struct.  This should only be called once during the
 * structure's lifetime, by makeSuffixArray().  There are various very
 * small variations in output which may or may not be valid between
 * various incarnations of this algorithm's implementation here.  Until
 * a reverse burrow-wheeler transform is implemented in the tests, the
 * accuracy of this function is not rigerously verified, although it may
 * still be usable for string operations.
 * 
 * TODO: consider adding code to reverse the order of all the last
 * suffixes with the same starting symbol -- this may lead to more
 * well behaved ordering.
 **********************************************************************/
size_t *sais(const unsigned char *source, const size_t length){
  //DECLARATIONS////////////////////////////////////////////////////////
  size_t *bucket[256];
  size_t bucketSize[256];
  size_t bucketFrontCounter[256];
  size_t bucketEndCounter[256];
  
  unsigned char *LMSandLS;

  //INITIALIZATION//////////////////////////////////////////////////////
  size_t *data = malloc(sizeof(size_t) * length);
  LMSandLS = malloc(sizeof(unsigned char) * length);

#ifdef DEBUG
  if(!data) exit(-1);
  if(!LMSandLS) exit(-1);
#endif

  /*prescan for buckets************************************************/
  //calculate bucket sizes
  memset(bucketSize, 0, sizeof(size_t)* 256);
  memset(bucketFrontCounter, 0, sizeof(size_t)* 256);
  memset(bucketEndCounter, 0, sizeof(size_t)* 256);
  for(size_t i = 0; i < length; i++)
    bucketSize[source[i]]++;

  //calculate bucket start and stops
  bucket[0] = data;
  for(short i = 1; i < 256; i++)
    bucket[i] = &bucket[i-1][bucketSize[i-1]];

#ifdef DEBUG
  //first place where bucket data can be printed
  fprintf(stderr, "%lu\n", length);
  printBucket(bucket, bucketSize);
#endif

  //MAIN PROCESSING/////////////////////////////////////////////////////
  /*set up L, S, and LMS metadata**************************************/
  /*0 = undefined, 1 = L, 2 = S, 3 = LMS*/
  LMSandLS[length-1] = 1;
  /*The paper stipulates an additional universally minimal character
   * which is definitionally LMS, but the addition of a control
   * character is pedantic and doesn't allow a general purpose
   * implementation.  What is done here pre-empts that approach with
   * forcing the last character to be a L characters, which keeps
   * mathematical corectness but requires minor tweaks further on.*/

  //Assign characters' values right to left (end to beginning) for L, S,
  //and LMS
  for(size_t i = length-2; i != ((size_t)0)-1; i--){
    if(source[i] > source[i+1]){
      LMSandLS[i] = 1;
      if(LMSandLS[i+1] == 2){
        LMSandLS[i+1] = 3;
      }
    }else if(source[i] < source[i+1]){
      LMSandLS[i] = 2;
    }else{
      if(LMSandLS[i+1] == 1){
        LMSandLS[i] = 1;
      }else{
        LMSandLS[i] = 2;
      }
    }
  }
  if(LMSandLS[0] == 2){
    LMSandLS[0] = 3;
  }

#ifdef DEBUG
  printLMSandLS(LMSandLS, length);

  fprintf(stderr, "\n\nAdding to buckets\n\n\n");
#endif

  /*Add entries to buckets*********************************************/
  //This is supposed to prepare the data to be induce sorted.

  //LMS type right-to-left scan -- Add LMS entries to the ends of
  //various buckets going from right to left.  The result is partially
  //full buckets with LMS entries in acending order.
  for(size_t i = length-1; i != ((size_t)0)-1; i--){
    if(LMSandLS[i] == 3){
      const unsigned char target = source[i];
      bucket[target][(bucketSize[target] - bucketEndCounter[target]) - 1] = i;
      bucketEndCounter[target]++;
    }
  }

#ifdef DEBUG
  printBucket(bucket, bucketSize);
#endif

  //inducing SA from SA1 step 2
  //L type left-to-right scan, not exactly a direct reasoning for this,
  //please refer to the paper.  Bounds checking was used in place of
  //checking for negative values so that -1 didn't have to be used,
  //allowing architentually maximal string length.
	const unsigned char bucketLocation = source[length-1];
  bucket[bucketLocation][0] = length-1;
  bucketFrontCounter[bucketLocation]++;
  for(int i = 0; i < 256; i++){
    for(size_t j = 0; j < bucketFrontCounter[i]; j++){
      if(!bucket[i][j]) continue;
      const size_t target = bucket[i][j]-1;

      if(LMSandLS[target] == 1){
        bucket[source[target]][bucketFrontCounter[source[target]]] = target;
        bucketFrontCounter[source[target]]++;
      }
    }
    for(size_t j = bucketSize[i] - bucketEndCounter[i]; j < bucketSize[i]; j++){
      if(!bucket[i][j]) continue;
      const size_t target = bucket[i][j]-1;

      if(LMSandLS[target] == 1){
        bucket[source[target]][bucketFrontCounter[source[target]]] = target;
        bucketFrontCounter[source[target]]++;
      }
    }
  }

#ifdef DEBUG
  printBucket(bucket, bucketSize);
#endif

  //step 3 of setting up SA
  //S type right to left scan.  Still difficult to follow reasoning.
  //The paper seems to suggest looping over all values and some other
  //various checking to make sure they're valid.  Bounds checking here
  //is again used.  It also has the benefit of more effectively
  //enforcing a reduction in the size of SA1 than the outlined
  //algorithm.
  for(int i = 255; i >= 0; i--){
    if(bucketSize[i] == 0) continue;
    for(size_t j = bucketSize[i] - 1; j >= bucketSize[i] - bucketEndCounter[i] && j != ((size_t)0)-1; j--){
      if(!bucket[i][j]) continue;
      const size_t target = bucket[i][j]-1;

      if(LMSandLS[target] == 2){
        unsigned char target2 = source[target];
        bucket[target2][bucketSize[target2] - (bucketEndCounter[target2]+1)] = target;
        bucketEndCounter[target2]++;
      }
    }

    for(size_t j = bucketFrontCounter[i] - 1; j != ((size_t)0)-1; j--){
      if(!bucket[i][j]) continue;
      const size_t target = bucket[i][j]-1;

      if(LMSandLS[target] == 2){
        const unsigned char target2 = source[target];
        bucket[target2][bucketSize[target2] - (bucketEndCounter[target2]+1)] = target;
        bucketEndCounter[target2]++;
      }
    }
  }

#ifdef DEBUG
  printBucket(bucket, bucketSize);
#endif

  /*Start induction****************************************************/

#ifdef DEBUG
  fprintf(stderr, "\n\nStarting Induction\n\n\n");
#endif

  memset(bucketEndCounter, 0, sizeof(size_t)* 256);
  //S type right to left scan.  Still difficult to follow reasoning, but
  //it seems to work.
  for(int i = 255; i >= 0; i--){
    if(!bucketSize[i]) continue;
    const size_t loopUntil = ((size_t)0)-1;
    for(size_t j = bucketSize[i] - 1; j != loopUntil; j--){
      if(!bucket[i][j]) continue;
      const size_t target = bucket[i][j]-1;

      if(LMSandLS[target] != 1){
        const unsigned char target2 = source[target];
        bucket[target2][(bucketSize[target2]-1) - bucketEndCounter[target2]] = target;
        bucketEndCounter[target2]++;
      }
    }
  }

#ifdef DEBUG
  printBucket(bucket, bucketSize);
#endif

  //CLEANUP AND RETURN//////////////////////////////////////////////////

  free(LMSandLS);

  return data;
}


size_t* AppendIdentInit(const unsigned char *source, const size_t length, const size_t *sArray){

#ifdef DEBUG
  fprintf(stderr, "Starting Prepend Identity metadata\n"); fflush(stdout);
#endif

  size_t *appendIdent = malloc(sizeof(size_t) * length);
  //~ size_t *runningLPT = malloc(sizeof(size_t) * length);

	//~ memset(runningLPT, 0, sizeof(size_t) * length);
  appendIdent[0] = 0; //can't have and first characters in common with
											//nothing
  for(size_t i = 1; i < length; i++){
		//~ if(!runningLPT[sArray[i]]){
      size_t maxIndex = length - max(sArray[i-1], sArray[i]);
      for(appendIdent[i] = 0; appendIdent[i] < maxIndex; appendIdent[i]++){
        if(source[sArray[i-1] + appendIdent[i]] !=
                      source[sArray[i] + appendIdent[i]])
          break;
      }
			
			//~ runningLPT[sArray[i]] = appendIdent[i];
			//~ for(size_t j = sArray[i]+1; j < length && runningLPT[j-1] > 1; j++){
				//~ runningLPT[j] = runningLPT[j-1]-1;
			//~ }
	  //~ }else{
			//~ appendIdent[i] = runningLPT[sArray[i]];
		//~ }
  }
#ifdef DEBUG
  fprintf(stderr, "Finished Prepend Identity metadata\n"); fflush(stdout);
#endif

  return appendIdent;
}


////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS  //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


SuffixArray makeSuffixArray(const unsigned char* inputSequence,
                                              const size_t inputLength){

  assert(inputLength > 0);
  assert(inputSequence != NULL);

#ifdef DEBUG
  fprintf(stderr, "Initializing BWTArray\n"); fflush(stdout);
#endif

  SuffixArray toReturn = {inputSequence, false, inputLength,
																			sais(inputSequence, inputLength)};

#ifdef DEBUG
  fprintf(stderr, "Finished initializing BWTArray\n"); fflush(stdout);
#endif

  return toReturn;
}


EnhancedSuffixArray makeEnhancedSuffixArray(const SuffixArray toProcess){

#ifdef DEBUG
  fprintf(stderr, "Initializing EnhancedSuffixArray\n"); fflush(stdout);
#endif

  EnhancedSuffixArray toReturn = {toProcess,
        AppendIdentInit(toProcess.sequence, toProcess.length,
                                                    toProcess.sa_data)};

#ifdef DEBUG
  fprintf(stderr, "Finished initializing EnhancedSuffixArray\n"); fflush(stdout);
#endif

  return toReturn;
}


SuffixArray copySequenceToLocal(const SuffixArray toMod){
  assert(false == toMod.doIOwnSequence);

  unsigned char *tmp = malloc(sizeof(size_t) * toMod.length);
  memcpy(tmp, toMod.sequence, sizeof(size_t) * toMod.length);

  SuffixArray toReturn = {tmp, true, toMod.length, toMod.sa_data};
  return toReturn;
}


void freeSuffixArray(SuffixArray *toFree){
  SuffixArrayCaster *force = (SuffixArrayCaster*) toFree;
  if(force->doIOwnSequence) free(force->sequence);
  free(force->sa_data);
}


void freeEnhancedSuffixArray(EnhancedSuffixArray *toFree){
  freeSuffixArray(&toFree->sa_struct);
  free((size_t*)toFree->LCPArray);
}


#ifdef DEBUG
void printSuffixArrayContainer(EnhancedSuffixArray toDump){
  fprintf(stderr, "i\tsuftab\tlcptab\tbwttab\tSsuftab[i]\n"); fflush(stdout);
  for(size_t i = 0; i < toDump.sa_struct.length; i++){
    fprintf(stderr, "%lu\t", i); fflush(stdout);
    fprintf(stderr, "%lu\t", toDump.sa_struct.sa_data[i]); fflush(stdout);
    fprintf(stderr, "%lu\t", toDump.LCPArray[i]); fflush(stdout);
    fprintf(stderr, "%c\t", toDump.sa_struct.sequence[(toDump.sa_struct.sa_data[i] - 1 + toDump.sa_struct.length)%toDump.sa_struct.length]); fflush(stdout);
    for(size_t j = toDump.sa_struct.sa_data[i]; j < toDump.sa_struct.length; j++){
      fprintf(stderr, "%c", toDump.sa_struct.sequence[j]); fflush(stdout);
    }
    fprintf(stderr, "\n");  fflush(stdout);
  }
}
#endif
