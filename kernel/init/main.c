#include "gdt.h"
// 中断既包括内中断又包括外中断,怎么能和Trap等同呢?
// Linus的函数名的可读性就很不好.我觉得就应该统一叫做中断
#include "interrupt.h"
#include "keyboard.h"
#include "console.h"
#include "time.h"
#include "string.h"
#include "task.h"
#include "sched.h"
#include "semaphore.h"
#include "memory.h"
#include "fs.h"
#include "bit_allocation.h"
#include "execve.h"

// 这是照抄了Linus的栈空间大小
// Linus的实践说明这个栈空间肯定是够用的
#define KERNEL_STACK_SIZE 4096

uint32_t kernel_stack[KERNEL_STACK_SIZE / sizeof(uint32_t)];
gdt_desc gdt[MAX_GDT_TERMS] = {{0, 0},
                               {0x0000FFFF, 0x00CF9A00},  // 内核代码段(8)
                               {0x0000FFFF, 0x00CF9200},  // 内核数据段(0x10)
                               {0x0000FFFF, 0x00CFFA00},  // 用户代码段(0x18)
                               {0x0000FFFF, 0x00CFF200}}; // 用户数据段(0x20)

extern task_t tasks[MAX_GDT_TERMS];
extern bitmap_t file_alloc_bitmap;

// 不方便写在头文件里面的声明
void reload_gdt();

#ifdef _DEBUG
// 经典卖票问题
void sell_ticket_A();
void sell_ticket_B();
void sell_ticket_C();
#endif

#ifdef _DEBUG
char buf[4096];
void fs_test(void);
#endif

extern task_manager_t task_manager;
void idle(void);

void main(void)
{
    reload_gdt();
    interrupt_init();
    time_init();
    tty_init();
    memory_init();
    sched_init(); // 和408考纲的先后顺序不同,进程管理依赖于内存管理,所以应当先初始化内存管理的数据结构
    fs_init();
#ifdef _DEBUG
    fs_test();
#endif
    sti(); // 开中断操作放最后,不然老是有莫名其妙的错误
    // move_to_user_mode(); // Linux风格

#ifdef _DEBUG
    printf("MAIN END\n");
#endif
    for (;;)
    {
    }
}

void reload_gdt()
{
    lgdt((uint32_t)gdt, (uint16_t)sizeof(gdt));
}

#ifdef _DEBUG
int32_t ticket_num = 10000; // 票数要和0做大小比较,选用有符号数
semaphore_t s;

void sell_ticket_A()
{
    while (ticket_num > 0)
    {
        ticket_num--;
        printf("A sold out a ticket,remaining tickets:");
        printf(get_itoa(ticket_num, 10));
        println();
    }
    sys_exit(0);
}

void sell_ticket_B()
{
    while (ticket_num > 0)
    {
        ticket_num--;
        printf("B sold out a ticket,remaining tickets:");
        printf(get_itoa(ticket_num, 10));
        println();
    }
    sys_exit(0);
}

void sell_ticket_C()
{
    while (ticket_num > 0)
    {
        ticket_num--;
        printf("C sold out a ticket,remaining tickets:");
        printf(get_itoa(ticket_num, 10));
        println();
    }
    sys_exit(0);
}
#endif

// #ifdef _DEBUG
void fs_test(void)
{
    block_t block_1025;
    strcpy(((file_t *)block_1025.data)[0].file_name, "usr");
    ((file_t *)block_1025.data)[0].file_size = VALID_DATA_SIZE_IN_BLOCK;
    ((file_t *)block_1025.data)[0].first_sector = 1026;
    ((file_t *)block_1025.data)[0].last_sector = 1026;
    ((file_t *)block_1025.data)[0].sector_count = 1;
    ((file_t *)block_1025.data)[0].file_type = FILE_TYPE_NORMAL;
    block_1025.next_sector = 0xFFFFFFFF;
    write_disk(1025, 1, (uint8_t *)&block_1025);

    bit_free(&file_alloc_bitmap, 1026, true);
}
// #endif