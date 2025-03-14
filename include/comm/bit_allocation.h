#ifndef ALLOCATION_H
#define ALLOCATION_H

#include "stdint.h"
#include "bit.h"

typedef struct bitmap_t
{
    unsigned char *bits; // 实际数据
    int size;            // 一共有多少个分配项(单位:比特!)
} bitmap_t;

int bit_alloc(bitmap_t *bitmap, bool want);
int bit_free(bitmap_t *bitmap, int index, bool want);
void bit_bind(bitmap_t *bitmap, char *bits, int size);

#endif