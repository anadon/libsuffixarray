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
#include <unistd.h>

#include "suffixarray.h"

//#ifdef DEBUG
#include <stdio.h>
//#endif

////////////////////////////////////////////////////////////////////////
//  DEFINES  ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


#define _L_ (1)
#define _S_ (2)
#define _LMS_ (3)

#define u8  unsigned char
#define s16 signed short

//1 for always SAIS, 0 for always working recursive bucket sort
#define DEREFERENCE_BREAK_EVEN (0.0)


////////////////////////////////////////////////////////////////////////
//  MACROS  ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


////////////////////////////////////////////////////////////////////////
//  PRIVATE STRUCTURES  ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


/***********************************************************************
This is to help make sequence operations more general and safer.
***********************************************************************/
typedef struct{
  size_t startIndex, endIndex;
}lSeqAbst, sSeqAbst, seqAbst; //Sequence Abstraction

typedef struct{
  size_t size;
  lSeqAbst *bucket;
}lBucket;

typedef struct{
  size_t size;
  sSeqAbst *bucket;
}sBucket;

typedef struct{
  size_t size;
  lBucket *buckets;
}lBuckets;

typedef struct{
  size_t size;
  sBucket *buckets;
}sBuckets;
  
//This is to aid in passing around sequence abstractions.
typedef struct{
  size_t size;
  size_t *S;
}sequence;



////////////////////////////////////////////////////////////////////////
//  PRIVATE INTERFACE FUNCTIONS  ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////


#ifdef DEBUG
void printBucket(size_t **bucket, size_t oneD, size_t *twoD){
  for(int i = 0; i < oneD; i++){
    if(twoD[i] == 0) continue;
    fprintf(stderr, "(");
    for(size_t j = 0; j < twoD[i] - 1; j++){
      fprintf(stderr, "%lu, ", bucket[i][j]);
    }
    fprintf(stderr, "%lu), ", bucket[i][twoD[i]-1]);
  }
  fprintf(stderr, "\n");
  fflush(stdout);
}


void printArray(size_t *array, size_t size){
  for(size_t i = 0; i < size; i++)
    printf("%lu, \t", array[i]);
  printf("\n");
  fflush(stdout);
}


void printLMSandLS(unsigned char *LMSandLS, size_t length){
  fprintf(stderr, "{");
  for(size_t i = 0; i < length - 2; i++){
    fprintf(stderr, "%c, ", LMSandLS[i] == _L_ ? 'L' : (LMSandLS[i] == _S_ ? 'S' : 'M'));
  }
  fprintf(stderr, "%c}\n", (LMSandLS[length-1] == _L_ ? 'L' : 'S'));
  fflush(stdout);
}
#endif


sequence initSequence(size_t length){
  sequence toReturn;
  toReturn.size = length;
  toReturn.S = malloc(sizeof(*toReturn.S) * toReturn.size);
  
#ifdef DEBUG
  if(length == 0){
    fprintf(stderr, "initSequnce: invalid length argument\n");
    fflush(stderr);
    exit(errno);
  }
  if(toReturn.S == NULL){
    fprintf(stderr, "initSequnce: failed to allocate space to sequence\n");
    fflush(stderr);
    exit(errno);
  }
#endif
  
  return toReturn;
}


/***********************************************************************
 * Create array of indexes from S which omit all except the last 
 * repeated value.
***********************************************************************/
sequence removeRuns(const u8 *S, const size_t size){
  sequence toReturn = initSequence(size);
  toReturn.size = 0;
  for(size_t i = 0; i < size; i++) //if(S[i] != S[i+1])
    toReturn.S[toReturn.size++] = i;
  toReturn.S = realloc(toReturn.S, toReturn.size * sizeof(*toReturn.S));
  return toReturn;
}


/***********************************************************************
 * Re-add runs of elements into SSA
 * 
 * proxy: the SSA arranged repeats removed indexes
 * 
***********************************************************************/
sequence addRuns(const u8 *input, const size_t length, sequence proxy){

//DECLARATIONS//////////////////////////////////////////////////////////
  sequence toReturn;
  //size_t numToExpand;
  
//INITIALIZATIONS///////////////////////////////////////////////////////
  toReturn = initSequence(length);
  memcpy(toReturn.S, proxy.S, sizeof(*toReturn.S) * toReturn.size);
  //toReturn.size = 0;
  
//OPERATIONS////////////////////////////////////////////////////////////
  /*for(size_t i = 1; i < proxy.size; i++){
    const char isARepeat = input[proxy.S[i]] == input[proxy.S[i]-1];
    if(!isARepeat){
      toReturn.S[toReturn.size++] = proxy.S[i];
    }else{
      //size_t j = i+1;
      while(proxy.S[i] - numToExpand > 0 && 
            input[proxy.S[i] - (numToExpand+1)] == 
                                        input[proxy.S[i] - numToExpand])
        numToExpand++;

      size_t *foundToRepeat;
      //Here we flatten the 2D array and will just use a more complex
      //accessing portion in order to avoid more calls to malloc.
      foundToRepeat = malloc(sizeof(*foundToRepeat) * numToExpand * 3);
      //NOTE: in parsing as entries become exausted, the pointers for
      //reading and writing should be seperate so that in the course of 
      //each expantion iteration old entries are removed in an efficient
      //manner.
      
      //populate the entries to expand
      if(proxy.S[i]+1 < length || input[proxy.S[i]] < input[proxy.S[i]+1]){//left expansion
        for(size_t k = numToExpand-1; k != ((size_t)0)-1; k--){
          toReturn.S[toReturn.size++] = proxy.S[i] - k;
        }
      }else{//right expansion
        for(size_t k = 0; k < numToExpand; k++){
          toReturn.S[toReturn.size++] = proxy.S[i] - k;
        }
      }
      
    }
  }*/

//CLEAN UP//////////////////////////////////////////////////////////////


  return toReturn;
}


u8 getAlphabetSize(const u8 *input, const size_t size){
  u8 toReturn = 0;
  for(size_t i = 0; i < size; i++) 
    toReturn = input[i] > toReturn ? input[i] : toReturn;
  return toReturn+1;
}


////////////////////////////////////////////////////////////////////////
//  PRIVATE SUFFIX ARRAY IMPLEMENTATIONS  //////////////////////////////
////////////////////////////////////////////////////////////////////////

/*Resursive Bucket Sort************************************************/
/***********************************************************************
* Recursive bucket sort is a simple algorithm with O(n^2) time and space
* requirements.  It sorts each layer progresively from each first value 
* of each suffix to the point the suffix is in it's own bucket.  This is
* useful for small scale verification of correctness, but is too costly
* to use in practice.
* 
* @bucket :contains a number of indicies from source to be sorted
* @source :contains sequence from which the suffix array is being 
*          constructed
* @depth  :used to track the depth of the algorithm so suffixes can be 
*          compared appropriately.  First call should always be 0.
***********************************************************************/
void recursiveBucketSort(size_t *bucket, const size_t bucketSize, 
            const u8 *source, const size_t sourceLength, size_t depth){

//DECLARATIONS//////////////////////////////////////////////////////////
  size_t *count, *ptrTrackerUnmod, *ptrTrackerMod;
  size_t i, *tmpBucket;
  const size_t nextDepth = depth+1;
  size_t overflowIndex;

//MEMORY ALLOCATION BUG HACK////////////////////////////////////////////
  do{
    for(i = 1; i < bucketSize; i++){
      if(bucket[i] + depth >= sourceLength) break;
      if(source[bucket[0] + depth] != source[bucket[i] + depth]) break;
    }
    if(i == bucketSize) depth++;
  }while(i == bucketSize);
  

//INITIALIZATIONS///////////////////////////////////////////////////////
  overflowIndex = 0;
  ptrTrackerUnmod = malloc(sizeof(size_t) * 256);
  ptrTrackerMod   = malloc(sizeof(size_t) * 256);
  count           = malloc(sizeof(size_t) * 256);
  tmpBucket       = malloc(sizeof(size_t) * bucketSize);
  
  memset(count, 0, sizeof(size_t) * 256);
  
//OPERATIONS////////////////////////////////////////////////////////////
  
  ptrTrackerUnmod[0] = 0;
  for(i = 0; i < bucketSize; i++){
    if(bucket[i] + depth >= sourceLength){
      ptrTrackerUnmod[0] = 1;
      overflowIndex = i;
      tmpBucket[0] = i;
      break;
    }
  }
  
  for(i = 0; i < bucketSize; i++) 
    if(ptrTrackerUnmod[0] == 0 || i != overflowIndex)
      count[source[bucket[i] + depth]]++;
  
  for(i = 1; i < 256; i++) ptrTrackerUnmod[i] = ptrTrackerUnmod[i-1] + 
                                                            count[i-1];
  memcpy(ptrTrackerMod, ptrTrackerUnmod, sizeof(size_t) * 256);
  
  for(i = 0; i < bucketSize; i++)
    if(ptrTrackerUnmod[0] == 0 || i != overflowIndex)
      tmpBucket[ptrTrackerMod[source[bucket[i]+depth]]++] = bucket[i];
  free(ptrTrackerMod); ptrTrackerMod = NULL;
  
  memcpy(bucket, tmpBucket, sizeof(size_t) * bucketSize);
  free(tmpBucket); tmpBucket = NULL;
  
  
  for(i = 0; i < 256; i++)
    if(count[i] > 1)
      recursiveBucketSort(&bucket[ptrTrackerUnmod[i]], count[i], source, 
                                              sourceLength, nextDepth);
  free(count);
  free(ptrTrackerUnmod);
  
}



/*Suffix Array Induced Sorting*****************************************/
/***********************************************************************
* Suffix Array Induced Sorting (SAIS) is a linear time/space suffix 
* array construction algorithm which competes with BPR2 for top 
* time/space requirements.  TODO: reference paper here.
* 
* Currently this code is being changes to enable testing of run-removal
* on the code.  Run removal enforces stricter conditions on the input 
* sequence which allows for some of the logic to be simplified.  This
* experimental approach is particularly useful for inputs with small
* alphabets in the general case, but definitionally saves on sequences 
* which are known to have many runs of identical values in the sequence.
* 
* @source  :The sequence to construct the suffix array on.
* @runsRem :
* 
* @alphabetSize : not actually alphabet size, but highest value seen in 
*                 alphabet.  Might change in future.
* 
* 
* Notes
* 0 = undefined, 1 = L, 2 = S, 3 = {LMS, M}
***********************************************************************/
sequence SAIS(const u8 *source, const size_t sourceLength, 
                        const sequence runsRem, const u8 alphabetSize){
//DECLARATIONS//////////////////////////////////////////////////////////
  sequence toReturn, sanityCheck;
  size_t **bucket;
  size_t **oldBucket;
  size_t i;
  size_t *bucketSize;
  size_t *bucketFrontCounter;
  size_t *bucketEndCounter;
  
  unsigned char *LMSandLS;

////INITIALIZATION//////////////////////////////////////////////////////
  LMSandLS  = malloc(sizeof(unsigned char) * runsRem.size);
  bucket    = malloc(sizeof(*bucket) * alphabetSize);
  oldBucket = malloc(sizeof(*oldBucket) * alphabetSize);
  bucketSize = calloc(sizeof(size_t)* alphabetSize, 1);
  bucketFrontCounter = calloc(sizeof(size_t)* alphabetSize, 1);
  bucketEndCounter = calloc(sizeof(size_t)* alphabetSize, 1);
  sanityCheck = initSequence(runsRem.size);
  memset(sanityCheck.S, 0, sizeof(*sanityCheck.S) * sanityCheck.size);
  sanityCheck.size = 0;

  /*prescan for buckets************************************************/
  //calculate bucket sizes
  
  for(i = 0; i < runsRem.size; i++) bucketSize[source[runsRem.S[i]]]++;

  //calculate bucket start and stops
  for(short i = 0; i < alphabetSize; i++){
    bucket[i] = calloc(sizeof(size_t), bucketSize[i]);
    oldBucket[i] = calloc(sizeof(size_t), bucketSize[i]);
  }

#ifdef DEBUG
  //first place where bucket data can be printed
  fprintf(stderr, "%lu\n", runsRem.size);
  printBucket(bucket, alphabetSize, bucketSize);
#endif


//OPERATION/////////////////////////////////////////////////////////////
  /*set up L, S, and LMS metadata**************************************/
  /*The paper stipulates an additional universally minimal character
   * which is definitionally LMS, but here it is simulated.*/

  //Assign characters' values right to left (end to beginning) for L, S,
  //and LMS
  size_t loopUntil = runsRem.size - 2;
  LMSandLS[runsRem.size-1] = _L_;
  for(i = loopUntil; i != ((size_t)0)-1; i--)
    LMSandLS[i] = source[runsRem.S[i]] > source[runsRem.S[i+1]] ? _L_ : _S_;
  
  i=0;
  while(1){
    while(i < loopUntil && LMSandLS[i] == _L_) i++;
    if(i >= loopUntil) break;
    LMSandLS[i++] = _LMS_;
    while(i < loopUntil && LMSandLS[i] == _S_) i++;
    if(i >= loopUntil) break;
  }

#ifdef DEBUG
  printLMSandLS(LMSandLS, runsRem.size);

  fprintf(stderr, "\n\nAdding to buckets\n\n");
#endif

/*PRIMEER***************************************Add entries to buckets*/
  //This is supposed to prepare the data to be induce sorted.

  memcpy(bucketEndCounter, bucketSize, sizeof(*bucketEndCounter) * alphabetSize);
  
  bucket[source[runsRem.S[runsRem.size-1]]][0] = runsRem.size-1;
  //bucketFrontCounter[bucketLocation]++;
  
  //LMS type right-to-left scan -- Add LMS entries to the ends of
  //various buckets going from right to left.  The result is partially
  //full buckets with LMS entries in acending order.
  for(size_t i = runsRem.size-1; i != ((size_t)0)-1; i--){
    if(LMSandLS[i] == _LMS_){
      const unsigned char target = source[runsRem.S[i]];
      bucket[target][--bucketEndCounter[target]] = i;
    }
  }
  

/*LOOP OVER UNTIL COMPLETE*********************************************/
//TODO: there's some really ugly ways to make this run faster.
  //L type left-to-right scan, not exactly a direct reasoning for this,
  //please refer to the paper.  Bounds checking was used in place of
  //checking for negative values so that -1 didn't have to be used,
  //allowing architentually maximal string length.
  char goOn;
  do{
    goOn = 0;
    memset(sanityCheck.S, 0, sizeof(*sanityCheck.S) * runsRem.size);
    sanityCheck.size = 0;

    //step 3 of setting up SA
    //L type right to left scan.
    memcpy(bucketEndCounter, bucketSize, sizeof(*bucketEndCounter) * alphabetSize);
    for(i = alphabetSize-1; i != ((size_t)0)-1 ; i--){
      for(size_t j = bucketSize[i]-1; j != ((size_t)0)-1; j--){
        if(!bucket[i][j]) continue;
        const size_t target = bucket[i][j]-1;

        if(LMSandLS[target] == _L_ || LMSandLS[target] == _LMS_){
          char KILLYOSELF = 0;
          if(source[runsRem.S[target]] == source[runsRem.S[runsRem.size-1]] && bucketEndCounter[source[runsRem.S[target]]]-1 == 0){
            KILLYOSELF = 1;
            printf("Trying to write over something you're blatently not supposed to write over.\n");
          }
          for(size_t k = 0; k < sanityCheck.size; k++)
            if(sanityCheck.S[k] == target){
              KILLYOSELF=1;
              printf("trying to write a %lu a second time.\n", target);
            }
          
          if(KILLYOSELF){
            printf("KILL YO SELF in r to l\n");
            exit(1);
          }
          const unsigned char target2 = source[runsRem.S[target]];
          bucket[target2][--bucketEndCounter[target2]] = target;
          sanityCheck.S[sanityCheck.size++] = target;
        }
      }
    }

#ifdef DEBUG
    printBucket(bucket, alphabetSize, bucketSize);
#endif
    
    //S type left to right scan.
    memset(bucketFrontCounter, 0, sizeof(*bucketFrontCounter) * alphabetSize);
    //bucket[source[runsRem.S[runsRem.size-1]]][0] = runsRem.size-1;
    bucketFrontCounter[source[runsRem.S[runsRem.size-1]]] = 1;//protect last index
  
    for(int i = 0; i < alphabetSize; i++){
      for(size_t j = 0; j < bucketSize[i]; j++){
        if(!bucket[i][j]) continue;
        const size_t target = bucket[i][j]-1;
        
        if(LMSandLS[target] == _S_){
          char KILLYOSELF = 0;
          if(source[runsRem.S[target]] == source[runsRem.S[runsRem.size-1]] && bucketEndCounter[source[runsRem.S[target]]]-1 == 0){
            KILLYOSELF = 1;
            printf("Trying to write over something you're blatently not supposed to write over.\n");
          }
          for(size_t k = 0; k < sanityCheck.size; k++)
            if(sanityCheck.S[k] == target){
              KILLYOSELF=1;
              printf("trying to write a %lu a second time.\n", target);
            }
          
          if(KILLYOSELF){
            printf("KILL YO SELF in l to r\n");
            exit(1);
          }
          const unsigned char target2 = source[runsRem.S[target]];
          bucket[target2][bucketFrontCounter[target2]++] = target;
          sanityCheck.S[sanityCheck.size++] = target;
        }
      }
    }

    for(i = 0; i < alphabetSize; i ++)
      if(memcmp(bucket[i], oldBucket[i], bucketSize[i] * sizeof(size_t))){
        for(size_t j = i; j < alphabetSize; j++)
          if(bucketSize[j])
            memcpy(oldBucket[j], bucket[j], sizeof(*bucket) * bucketSize[j]);
        goOn = 1;
        break;
      }

  }while(goOn);

#ifdef DEBUG
    printBucket(bucket, alphabetSize, bucketSize);
#endif

//CLEAN UP//////////////////////////////////////////////////////////////

  free(LMSandLS);
  
  toReturn = initSequence(runsRem.size);
  toReturn.size = 0;
  for(i = 0; i < alphabetSize; i++)
    for(size_t j = 0; j < bucketSize[i]; j++)
      toReturn.S[toReturn.size++] = bucket[i][j];
  return toReturn;
}


/***********************************************************************
 * tweaked BPR2 to handle some changes nessicary for run removal
***********************************************************************/
sequence bpr2dereferenced(const u8 *source, const size_t length, const sequence input){

  sequence toReturn = initSequence(input.size);
  
  memcpy(toReturn.S, input.S, toReturn.size * sizeof(*toReturn.S));
  
  recursiveBucketSort(toReturn.S, toReturn.size, source, length, 0);
  
  return toReturn;
}


/***********************************************************************
 * original BPR2
***********************************************************************/
sequence bpr2direct(const u8 *source, const size_t length){

  sequence toReturn = initSequence(length);
  
  for(size_t i = 0; i < length; i++) toReturn.S[i] = i;
  
  recursiveBucketSort(toReturn.S, toReturn.size, source, length, 0);
  
  return toReturn;
}


/***********************************************************************
 * Switching to BPR2 over SAIS because SAIS became unruley and BPR2 
 * seems to have better performance characteristics.  Still 
 * investigating.
***********************************************************************/
size_t* getSortedSuffixArray(const u8 *input, const size_t length){
  
//DECLARATIONS//////////////////////////////////////////////////////////
  sequence toReturn, intermediate, runsRem;
  
//INITIALIZATIONS///////////////////////////////////////////////////////
  runsRem = removeRuns(input, length);

//OPERATIONS////////////////////////////////////////////////////////////
  
  if((runsRem.size * 1.0) / length > DEREFERENCE_BREAK_EVEN){
    free(runsRem.S);
    toReturn = bpr2direct(input, length);
  }else{
    u8 alphabetSize = getAlphabetSize(input, length);
    intermediate = SAIS(input, length, runsRem, alphabetSize);
  
    toReturn = addRuns(input, length, intermediate);
    free(runsRem.S);
    free(intermediate.S);
  }
  
//CLEAN UP//////////////////////////////////////////////////////////////
  //free(intermediate.S);
  
  return toReturn.S;
}


////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS  //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


SuffixArray makeSuffixArray(const u8* inputSequence,
                                              const size_t inputLength){

#ifdef DEBUG
  assert(inputLength > 0);
  assert(inputSequence != NULL);
  fprintf(stderr, "Initializing Suffix Array\n"); fflush(stdout);
#endif

  SuffixArray toReturn = {inputSequence, false, inputLength,
                      getSortedSuffixArray(inputSequence, inputLength)};

#ifdef DEBUG
  fprintf(stderr, "Finished initializing Suffix Array\n"); fflush(stdout);
#endif

  return toReturn;
}


EnhancedSuffixArray makeEnhancedSuffixArray(const SuffixArray toProcess){

#ifdef DEBUG
  fprintf(stderr, "Initializing EnhancedSuffixArray\n"); fflush(stdout);
#endif

  EnhancedSuffixArray toReturn;// = {toProcess,
  //      AppendIdentInit(toProcess.sequence, toProcess.length,
  //                                                  toProcess.sa_data)};

#ifdef DEBUG
  fprintf(stderr, "Finished initializing EnhancedSuffixArray\n"); fflush(stdout);
#endif

  return toReturn;
}


SuffixArray copySequenceToLocal(const SuffixArray toMod){
  assert(false == toMod.doIOwnSequence);

  u8 *tmp = malloc(sizeof(size_t) * toMod.length);
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
