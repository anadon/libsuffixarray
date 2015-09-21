/*
 * suftest.c for timesuftest.pl
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "archon.h"
#include "direct.h"

int memory = 0;

int
archonsort(unsigned char *T, int *SA, int n);

int
main(int argc, const char *argv[]) {
  FILE *fp;
  unsigned char *T;
  int *A;
  int len;

  fp = fopen(argv[1], "rb");

  fseek(fp, 0, SEEK_END);
  len = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  T = calloc((len + DEAD + 1), sizeof(unsigned char));
  A = malloc((len + 1) * sizeof(int));

  fread(T + DEAD, sizeof(unsigned char), len, fp);
  fclose(fp);

  archonsort(T, A, len);

  free(A);
  free(T);

  return 0;
}
