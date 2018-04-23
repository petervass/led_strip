#ifndef _UTIL_H

#define _UTIL_H

#include "libws2811.h" /* packColor */



void animPattern(uint32_t pal,uint32_t * col_ptr, uint32_t len_t);
void animSolid(uint32_t pal,uint32_t * col_ptr, uint32_t len_t);
void animSin(uint32_t pal,uint32_t * col_ptr, uint32_t len_t);


void anim(uint8_t mod,uint32_t pal,uint32_t * col_ptr, uint32_t len_t);

#endif
