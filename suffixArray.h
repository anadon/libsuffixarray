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

#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })



typedef struct suffixArray{
    /*This points to the sequence in memory.  The memory the sequence
   * is in may not belong to this object, but there is a function
   * available to make a  copy in local memory*/
  const unsigned char *sequence;
  unsigned char *internalSequence;
  const size_t length;

  size_t *bwtArray;
  size_t *appendIdent;
}suffixArray;


/***********************************************************************
 * Create a suffixArray from a passed suffixArray, effectively being a
 * copy, except that it now owns the original sequence memory.
 **********************************************************************/
suffixArray copySequenceToLocal(suffixArray toMod);


/***********************************************************************
 * Create and initialize a suffixArray structure.
 *
 * It does this in the most memory efficient manner possible, so it does
 * not copy the original sequence array.
 **********************************************************************/
suffixArray makeSuffixArray(const unsigned char* inputSequence,
                                              const size_t inputLength);


/***********************************************************************
 * Destrory and free resources held by a suffixArray.
 **********************************************************************/
void freeSuffixArray(suffixArray *toFree);


#endif