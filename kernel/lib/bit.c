#include "bit.h"
#include "console.h"

// 为了照顾到尽可能多的应用场景,以下均使用32位
// 获取[start,end)范围内的所有位
uint32_t testbits(uint32_t num, uint8_t start, uint8_t end)
{
    return (num >> start) & ((1 << (end - start)) - 1);
}

bool testbit(uint32_t num, uint8_t digit)
{
    return testbits(num, digit, digit + 1);
}

void setbit(uint32_t *p_num, int index, bool value)
{
    if (value)
        (*p_num) |= 1 << index;
    else
        (*p_num) &= ~(1 << index);
}