//GPLv3 this properly at some point

#ifndef SUFFIX_ARRAY_H
#define SUFFIX_ARRAY_H


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



void BWTRadixSort(suffixArray toSetup);


void AppendIdentInit(suffixArray toSetup);
	
	
suffixArray makeSuffixArray(const unsigned char* inputSequence, 
																							const size_t inputLength);
	
void freeSuffixArray(suffixArray toFree);
	

#endif