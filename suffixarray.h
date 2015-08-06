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

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct suffixArrayContainer{
    /*This points to the sequence in memory.  The memory the sequence
   * is in may not belong to this object, but there is a function
   * available to make a  copy in local memory*/
  const unsigned char *sequence;
  unsigned char *internalSequence;
  const size_t length;

	/*Since the Burrow-Wheeler transformation table can be derived from
	 * the suffixArray here, a memory-using version is not used.  Instead,
	 * all attempts to get a given BWT value should be as follows:
	 * 
	 * BWTArray[i] = (suffixArray[i] + length - 1)%length
	 * 
	 * This is not inclused as a function because the nature of this 
	 * program requires minimalistic data and a function call could 
	 * increase CPU-overhead for something not everyone needs and can be
	 * extrapolated from data that already exists.
	 * */
  size_t *suffixArray;
	
	/*The LCPArray defined the number of same continuous characters in 
	 * sequence[suffixArray[i]] and sequence[suffixArray[i-1]] for LCP[i].
	 * */
  size_t *LCPArray;
}suffixArrayContainer;


/***********************************************************************
 * Create a suffixArray from a passed suffixArray, effectively being a
 * copy, except that it now owns the original sequence memory.
 **********************************************************************/
suffixArrayContainer copySequenceToLocal(suffixArrayContainer toMod);


/***********************************************************************
 * Create and initialize a suffixArray structure.
 *
 * It does this in the most memory efficient manner possible, so it does
 * not copy the original sequence array.
 **********************************************************************/
suffixArrayContainer makeSuffixArray(const unsigned char* inputSequence,
                                              const size_t inputLength);


/***********************************************************************
 * Destrory and free resources held by a suffixArray.
 **********************************************************************/
void freeSuffixArray(suffixArrayContainer *toFree);


#ifdef DEBUG
/***********************************************************************
 * Dump table contents for debugging purposes
 **********************************************************************/
void printSuffixArrayContainer(suffixArrayContainer toDump);
#endif

#ifdef __cplusplus
}
#endif

#endif