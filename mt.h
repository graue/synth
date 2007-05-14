#ifndef MT_H
#define MT_H

#include <stdint.h>

void mt_init(uint32_t seed);
uint32_t mt_urand(void);
int32_t mt_rand(void);
double mt_frand(void);

#ifdef MT_IMPLEMENT
#include "mt.c"
#endif

#endif
