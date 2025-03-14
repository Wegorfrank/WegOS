#include "asm.h"
#include "memory.h"
#include "disk.h"
#include "console.h"
#include "bit.h"
#include "bit_allocation.h"
#include "interrupt.h"

page_table_entry_t kernel_pd[PAGE_SIZE / sizeof(page_table_entry_t)] __attribute__((aligned(PAGE_SIZE)));

char memory_alloc_table[(1 << 20) / 8]; // 一共有多少个物理页?1<<20
bitmap_t memory_alloc_bitmap;
void page_fault_handler(void);

void memory_init(void)
{
    memory_alloc_init();                                                                        // 如果想调用p_malloc或p_free,在此之前一定要保证内存分配的数据结构已经被初始化
    add_mmap_to_itself((unsigned int)kernel_pd, ITSELF_END, PAGE_TABLE_FLAG_KERNEL_READ_WRITE); // 从0到0x10000000全部内存映射到自身
    write_cr3((unsigned int)kernel_pd);
    // printf("0xb8000->");
    // PRINT_HEX(addr_translate((unsigned int)kernel_pd, 0xB8000));
    enable_page();
    set_idt(PAGE_FAULT_INTERRUPT, (unsigned int)page_fault_handler);
}

void enable_page(void)
{
    write_cr0(read_cr0() | 0x80000000);
}

void memory_alloc_init(void)
{
    bit_bind(&memory_alloc_bitmap, memory_alloc_table, 1 << 20);
    // 将0到0x800000的所有地址预先设置为已分配
    for (int i = 0; i * PAGE_SIZE < 0x800000; i++) // 0x800000是一个临时的值,它应该被替换为内核代码段以及使用C语言定义的全局数据的结束位置
        bit_free(&memory_alloc_bitmap, i, true);
}

// 返回值:true(原本页表就已经存在,只是对它进行了改写),false(页表本来不存在,新创建了一张)
// 此返回值经常被忽略
// 无论当前有没有开启分页,页目录表是哪一个,此函数都能够正常工作!
bool set_mmap(unsigned int cr3,      // 页目录表的地址必须传入,这可不兴动态分配
              unsigned int virtual,  // (如果是C++这个变量名用不了)低12位会被无视
              unsigned int physical, // 低12位会被无视
              unsigned short flags)  // 分行写
{
#ifdef _DEBUG
    // if ((unsigned int)virtual & 0xFFF || (unsigned int)physical & 0xFFF)
    //     while (true)
    //         printf("Page not multiple of 4096");
#endif
    bool result;
    int index_in_pde = testbits(virtual, 12 + 10, 12 + 10 + 10);
    if (!(result = testbit(((page_table_entry_t *)cr3)[index_in_pde], PAGE_TABLE_P)))
        // 读写权限控制应当放在页表项而不是页目录项进行,所以页目录项的读写权限永远是任何进程可读可写
        // 实际上这里并没有考虑到p_malloc分配失败的情况...
        ((page_table_entry_t *)cr3)[index_in_pde] = p_malloc() | PAGE_TABLE_FLAG_USER_READ_WRITE;                    // 分配出来的物理地址一定要和某个虚拟地址一对一吗?我看不然.每个页表刚好占1页,这是一个很好的特性
    page_table_entry_t *pte_addr = (page_table_entry_t *)((((page_table_entry_t *)cr3)[index_in_pde]) & 0xFFFFF000); // 对应的页表地址
    int index_in_pte = testbits(virtual, 12, 12 + 10);
    pte_addr[index_in_pte] = ((unsigned int)physical) | flags;
    return result;
}

unsigned int load_memory(unsigned int cr3, unsigned int virtual, unsigned short flags)
{
#ifdef _DEBUG
    // if ((unsigned int)virtual & 0xFFF)
    //     while (true)
    //         printf("Page not multiple of 4096");
#endif
    unsigned int physical = p_malloc(); // p_malloc()的返回值可能是-1(物理内存满了)
    set_mmap(cr3, virtual, physical, flags);
    return physical;
}

// 0-upper_limit的内存地址全部映射到自身
void add_mmap_to_itself(unsigned int cr3, unsigned int upper_limit, unsigned short flags)
{
    for (int addr = 0; addr < upper_limit; addr += PAGE_SIZE) // 0到0x800000的地址全部映射到自身
        set_mmap((unsigned int)cr3, addr, addr, PAGE_TABLE_FLAG_KERNEL_READ_WRITE);
}

// 手动对虚拟地址进行翻译(函数不负责页不存在的错误处理!)
unsigned int addr_translate(unsigned int cr3, unsigned int virtual)
{
    page_table_entry_t *page_dir_table = (page_table_entry_t *)cr3;
    page_table_entry_t *page_table = (page_table_entry_t *)(page_dir_table[testbits(virtual, 12 + 10, 12 + 10 + 10)] & 0xFFFFF000);
    return (page_table[testbits(virtual, 12, 12 + 10)] & 0xFFFFF000) + (virtual & 0xFFF);
}

bool has_addr(unsigned int cr3, unsigned int virtual)
{
    page_table_entry_t *page_dir_table = (page_table_entry_t *)cr3;
    unsigned int pte_desc_in_pde = page_dir_table[testbits(virtual, 12 + 10, 12 + 10 + 10)];
    if (!testbit(pte_desc_in_pde, PAGE_TABLE_P))
        return false;
    page_table_entry_t *page_table = (page_table_entry_t *)(pte_desc_in_pde & 0xFFFFF000);
    return testbit(page_table[testbits(virtual, 12, 12 + 10)], PAGE_TABLE_P);
}

// 返回值是物理地址
unsigned int p_malloc(void) // 默认只分配一页(我想应该不会出现必须要求连续的好几页的需求吧)
{
    // 有了位图之后,物理内存的分配变得如此简单!
    int page_index = bit_alloc(&memory_alloc_bitmap, false);
    return (page_index == -1) ? (unsigned int)-1 : page_index * PAGE_SIZE;
}

// 参数类型是物理地址
void p_free(int p_addr)
{
#ifdef _DEBUG
    if (p_addr & 0xFFF)
        while (true)
            printf("p_free:p_addr not multiple of 4096\n");
#endif
    bit_free(&memory_alloc_bitmap, p_addr / PAGE_SIZE, false);
}

// 致敬Linus的copy_page_tables函数!
// 必须认识到复制页表是一个具有巨大风险的事情:进程A的页表复制给了进程B之后,由于页表项并没有跟着复制过去,如果进程A修改自己的页表项,那么进程B也会遭殃,反之亦然
void copy_pts(unsigned int from_pd, unsigned int to_pd)
{
    // Linus专门提供了一个size参数,我觉得不需要这个参数
    memcpy((void *)to_pd, (void *)from_pd, PAGE_SIZE); // 一个页表刚好占一页
}