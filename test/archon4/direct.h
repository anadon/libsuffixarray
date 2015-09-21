#ifndef _ARCHON_DIRECT_H_
#define _ARCHON_DIRECT_H_

#include <memory.h>
//special case for BSD
#if HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdlib.h>

//#define NDEBUG // defined in Makefile
#define DEAD	10

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

// Byte swap
static __inline
word
byteswap_16p(const word *n) {
#if defined(__i386__)  && defined(__GNUC__)
  word r = *n;
  __asm__("xchgb %b0, %h0" : "+q" (r));
  return r;
#elif defined(__ppc__) && defined(__GNUC__)
  word r;
  __asm__("lhbrx %0, 0, %1" : "=r" (r) : "r"  (n), "m" (*n));
  return r;
#else
  return (((*n) << 8) & 0xff00) | (((*n) >> 8) & 0xff);
#endif
}
static __inline
dword
byteswap_32p(const dword *n) {
#if defined(__i386__) && defined(__GNUC__)
  dword r = *n;
  __asm__("bswap %0" : "+r" (r));
  return r;
#elif defined(__ppc__) && defined(__GNUC__)
  dword r;
  __asm__("lwbrx %0, 0, %1" : "=r" (r) : "r"  (n), "m" (*n));
  return r;
#else
  return (((*n) & 0xff) << 24) | (((*n) & 0xff00) << 8) |
         (((*n) >> 8) & 0xff00) | (((*n) >> 24) & 0xff);
#endif
}

#if defined(__BIG_ENDIAN__)
# define CPU2LE16p(_a) byteswap_16p((word *)(_a))
# define CPU2LE32p(_a) byteswap_32p((dword *)(_a))
# warning Big Endian
//#elif defined(__LITTLE_ENDIAN__)
#else /* little endian or unknown endian... */
# define CPU2LE16p(_a) (*((word *)(_a)))
# define CPU2LE32p(_a) (*((dword *)(_a)))
# warning Little Endian
#endif


int ankinit(int);
void ankexit();
void ankprint(void(*)(char*,int,int,int,int));

int compare(int,int,int*);
void ray(int*,int,int);
int sufcheck(int*,int,char);
void getbounds(int,int**,int**);

// caution: REVER uses 'x' & 'z' values
#define REVER(px,pz,tm)	{ x=px,z=pz;		\
	while(x+1 < z) tm=*x,*x++=*--z,*z=tm;	\
}
#define GETMEM(num,type)			\
	(memory += (num)*sizeof(type),		\
	(type*)malloc((num)*sizeof(type)))
#define FREE(ptr) if(ptr) free(ptr)

#endif /* _ARCHON_DIRECT_H_ */
