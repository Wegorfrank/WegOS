#ifndef MEMORY_H
#define MEMORY_H

#include "stdint.h"

#define KERNEL_MEMORY_START 0x100000 // 之后如果内核加载地址发生变化,那么这个地址也会变化

#define PAGE_TABLE_P 0
#define PAGE_TABLE_R_W 1
#define PAGE_TABLE_U_S 2

#define PAGE_TABLE_FLAG_USER_READ_WRITE 7 // 允许所有特权级访问
#define PAGE_TABLE_FLAG_USER_READ_ONLY 5
#define PAGE_TABLE_FLAG_KERNEL_READ_WRITE 3 // 页表项或页目录项只允许内核访问
#define PAGE_TABLE_FLAG_KERNEL_READ_ONLY 1

#define PAGE_SIZE 4096

#define INIT_ESP_VMA 0xFFFFEFFC
#define ITSELF_END 0x10000000

// 页表项或者页目录项的结构
typedef uint32_t page_table_entry_t;

void memory_init(void);
void memory_alloc_init(void);
void enable_page(void);
void load_kernel_page_table(void);
// 返回物理地址(返回值也经常被忽略)
unsigned int load_memory(unsigned int cr3, unsigned int virtual, unsigned short flags); // 根据指定的虚拟地址,将它映射到存在的物理地址(只处理单个页面)
void add_mmap_to_itself(unsigned int cr3, unsigned int upper_limit, unsigned short flags);
bool has_addr(unsigned int cr3, unsigned int virtual);
// 各个参数的含义:cr3->页目录表地址
// flags->访问信息
bool set_mmap(unsigned int cr3, unsigned int virtual, unsigned int physical, unsigned short flags);
unsigned int p_malloc(void);
unsigned int addr_translate(unsigned int cr3, unsigned int virtual);
void copy_pts(unsigned int from_pd, unsigned int to_pd);

#endif