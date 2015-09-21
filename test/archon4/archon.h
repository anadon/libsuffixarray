#ifndef _ARCHON_H_
#define _ARCHON_H_

#include <stdio.h>
//#define VERBOSE

int geninit(FILE*);
int gencode();
void genprint();
void genexit();

int compute();
int verify();
int encode(FILE*);
int decode(FILE*);

#endif /* _ARCHON_H_ */
