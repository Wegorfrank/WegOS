#include "disk.h"

void load_kernel()
{
    // 进进进进进进来力(喜)
    // 把内核读取到一个固定的内存地址,然后跳过去就可以了
    // 跳转目标恐怕又需要使用call的形式
    // 放到什么位置最合适呢?Linux选择了0地址.我也选择0地址.
    // 中断已经被关掉了,无需担心破坏中断向量表
    read_disk(KERNEL_DEST, KERNEL_MAX_SIZE, (uint8_t *)0x100000);
}