#include "v_memory.h"
#include "memory.h"
#include "console.h"
#include "disk.h"
#include "asm.h"
#include "bit.h"

// 功能:把相应的页换出到磁盘交换区
void swap_out(unsigned int cr3, unsigned int useless)
{
}

// 考虑其他进程的页真是麻烦,所以我就把cr3参数取消了
// 调用前要保证相应的外存地址已经正确设置
void swap_in(unsigned int useful, unsigned short flags)
{
    unsigned int cr3 = read_cr3();
#ifdef _DEBUG
    if (has_addr(cr3, useful))
        emphasize("The page to swap in already exists!\n");
#endif
    int swap_sector = swap_sector_of(cr3, useful);
    load_memory(cr3, useful, flags);
    // 相应的页面已经装入内存,接下来需要把交换区的数据写进去
    read_disk(swap_sector, 8, (char *)useful); // 1页=8扇区
}

// 返回值:扇区号
int swap_alloc(void)
{
    // 我的磁盘很大!所以分配交换区空间时使用最原始的方法.
    static int sector = DISK_SWAP_START_SECTOR;
    int avl = sector;
    sector += 8;
    return avl;
}

void swap_free(void)
{
}

int swap_sector_of(unsigned int cr3, unsigned int virtual)
{
#ifdef _DEBUG
    if (has_addr(cr3, virtual))
        emphasize("swap_sector-of already exists\n");
#endif
    page_table_entry_t *page_dir_table = (page_table_entry_t *)cr3;
    unsigned int pte_desc_in_pde = page_dir_table[testbits(virtual, 12 + 10, 12 + 10 + 10)];
    if (!testbit(pte_desc_in_pde, PAGE_TABLE_P))
        return false;
    page_table_entry_t *page_table = (page_table_entry_t *)(pte_desc_in_pde & 0xFFFFF000);
    return SWAP_SECTOR_GET(page_table[testbits(virtual, 12, 12 + 10)]);
}

// 调用此函数之前,要保证页表存在
void swap_sector_set(unsigned int cr3, unsigned int virtual, int sector)
{
    page_table_entry_t *page_dir_table = (page_table_entry_t *)cr3;
    unsigned int pte_desc_in_pde = page_dir_table[testbits(virtual, 12 + 10, 12 + 10 + 10)];
    page_table_entry_t *page_table = (page_table_entry_t *)(pte_desc_in_pde & 0xFFFFF000);
    SWAP_SECTOR_SET(page_table[testbits(virtual, 12, 12 + 10)], sector);
}

void on_page_fault(unsigned int virtual, unsigned int error_code)
{
    swap_in(virtual, PAGE_TABLE_FLAG_KERNEL_READ_WRITE);
}
