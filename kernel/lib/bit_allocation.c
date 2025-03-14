#include "bit_allocation.h"
#include "console.h"

// 以下两函数使用时want一直传入0
int bit_alloc(bitmap_t *bitmap, bool want)
{
    // printf("bit_alloc,want=");
    // PRINT_HEX(want);
    // println();
    // printf("size=");
    // PRINT_HEX(bitmap->size);
    // println();
    for (int i = 0; i < bitmap->size / 8; i++) // 默认bitmap->size是能够被8整除的
    {
        // printf("i=");
        // PRINT_HEX(i);
        // printf(",");
        if ((want && bitmap->bits[i] != 0) || (!want && bitmap->bits[i] != 0xFF))
        {
            // printf("available found");
            // i是可以分配的
            // 从低到高,查看bits[i]的哪一位不等于want,然后将它取反
            for (int j = 0; j < 8; j++)
            {
                if (testbit(bitmap->bits[i], j) == want)
                {
                    setbit((uint32_t *)&bitmap->bits[i], j, !want);
                    return i * 8 + j;
                }
            }
        }
        // println();
    }
    return -1;
}

// 如果想指定把某个位设置为1,格式:bit_free(bitmap, index, 1);
int bit_free(bitmap_t *bitmap, int index, bool want)
{
    setbit((uint32_t *)&bitmap->bits[index >> 3], index & 7, want);
}

void bit_bind(bitmap_t *bitmap, char *bits, int size) // 又名:bit_init
{
    bitmap->bits = bits;
    bitmap->size = size;
}