/* suffixarray header for programs to link against
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


#ifndef SUFFIX_ARRAY_H
#define SUFFIX_ARRAY_H

#ifndef __cplusplus
#include <stdbool.h>
#endif

//Add alternate naming schemes so that naming conventions can be
//consistant for different style guides.
#define suffixArray SuffixArray
#define SA SuffixArray
#define sa SuffixArray

#define enhancedSuffixArray EnhancedSuffixArray
#define ESA EnhancedSuffixArray
#define esa EnhancedSuffixArray


#ifdef __cplusplus
extern "C" {
#endif

typedef struct SuffixArray{
  /*This points to the sequence in memory.  The memory the sequence
   * is in may not belong to this object, but there is a function
   * available to make a  copy in local memory*/
  const unsigned char *sequence;
  const bool doIOwnSequence;
  const size_t length;

  /*Since the Burrow-Wheeler transformation table can be derived from
   * the suffixArray here, a memory-using version is not used.  Instead,
   * all attempts to get a given BWT value should be as follows:
   *
   * BWTArray[i] = source[(sa_data[i] + length - 1)%length];
   *
   * This is not inclused as a function because the nature of this
   * program requires minimalistic data and a function call could
   * increase CPU-overhead for something not everyone needs and can be
   * extrapolated from data that already exists.
   * */
  const size_t *sa_data;

}SuffixArray;


typedef struct EnhancedSuffixArray{

  SuffixArray sa_struct;
  /*The LCPArray defined the number of same continuous characters in
   * sequence[sa_data[i]] and sequence[sa_data[i-1]] for LCP[i].
   * */
  const size_t *LCPArray;
}EnhancedSuffixArray;


/***********************************************************************
 * This is to help with assignment and more typical usage, but is not
 * as proper.  Code using this may be slower.
 **********************************************************************/
typedef struct SuffixArrayCaster{
  unsigned char *sequence;
  bool doIOwnSequence;
  size_t length, *sa_data;
}SuffixArrayCaster;


typedef struct EnhancedSuffixArrayCaster{
  SuffixArrayCaster sa_struct;
  size_t *LCPArray;
}EnhancedSuffixArrayCaster;


/***********************************************************************
 * Create a suffixArray from a passed suffixArray, effectively being a
 * copy, except that it now owns the original sequence memory.  This is
 * included for better memory paradeigm corectness.
 **********************************************************************/
SuffixArray copySequenceToLocal(SuffixArray toMod);


/***********************************************************************
 * Create and initialize a suffixArray structure.
 *
 * It does this in the most memory efficient manner possible, so it does
 * not copy the original sequence array.
 **********************************************************************/
SuffixArray makeSuffixArray(const unsigned char* inputSequence,
                                              const size_t inputLength);


/***********************************************************************
 * Create and initialize a suffixArray structure.
 *
 * It does this in the most memory efficient manner possible, so it does
 * not copy the original sequence array.
 **********************************************************************/
EnhancedSuffixArray makeEnhancedSuffixArray(SuffixArray toProcess);


/***********************************************************************
 * Destrory and free resources held by a suffixArray.
 **********************************************************************/
void freeSuffixArray(SuffixArray *toFree);

void freeEnhancedSuffixArray(EnhancedSuffixArray *toFree);


#ifdef DEBUG
/***********************************************************************
 * Dump table contents for debugging purposes
 **********************************************************************/
void printSuffixArrayContainer(EnhancedSuffixArray toDump);
#endif


#ifdef __cplusplus
}
#endif

#endif