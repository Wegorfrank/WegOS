// 以下开始设计需要封装的进程有关函数
#include "task.h"
#include "sched.h"
#include "gdt.h"
#include "console.h"
#include "bit.h"
#include "memory.h"

extern gdt_desc gdt[MAX_GDT_TERMS];
extern page_table_entry_t kernel_pd[PAGE_SIZE / sizeof(page_table_entry_t)];

// 所有进程的pcb集中存放在一起
task_manager_t task_manager;

// 创建新进程需要哪些参数呢?程序入口地址肯定是需要的.
// 由于采用平坦模型,可以不提供相应的段选择子
// 一个重要事实是,栈指针也是必须的,不然会出现函数调用的莫名其妙的数据冲突
void task_init(uint16_t pid, unsigned int flag, unsigned int eip, unsigned int esp, unsigned int ddl)
{
    // 调用此函数的前提是:已经找到了一个空白的存放pcb的位置pid并且该pcb已经被放在了tasks数组当中的正确位置
    task_t *p_task = task_manager.tasks + pid;
    // 先安装TSS描述符
    gdt_desc desc;
    make_gdt_desc_for_tss(&desc, (unsigned int)(&(p_task->tss)), DPL_KERNEL);
    gdt[pid] = desc;
    // 入口地址和堆栈
    p_task->tss.esp = esp;
    p_task->tss.eip = eip;
    p_task->tss.eflags = INIT_EFLAGS;
    // 现在完成特权级保护问题!
    if (flag == TASK_FLAG_KERNEL)
    {
        p_task->tss.cs = GDT_KERNEL_CODE_SEG;
        // 数据段也直接沿用系统的段
        p_task->tss.ss = GDT_KERNEL_DATA_SEG;
        p_task->tss.ds = GDT_KERNEL_DATA_SEG;
    }
    else
    {
        p_task->tss.cs = GDT_USER_CODE_SEG;
        p_task->tss.ss = GDT_USER_DATA_SEG;
        p_task->tss.ds = GDT_USER_DATA_SEG;
    }

    unsigned int cr3 = p_malloc();
    // add_mmap_to_itself(cr3, ITSELF_END, PAGE_TABLE_FLAG_KERNEL_READ_WRITE);
    // 每创建一个新进程,都需要创建新的公共部分的映射,这是很麻烦的事情.
    // 为什么学习Linus,直接抄已经创建完毕的页表kernel_pd的作业呢?
    copy_pts((unsigned int)kernel_pd, (unsigned int)cr3); // 我有充足的信心认为kernel_pd之后不会修改
    p_task->tss.cr3 = cr3;
    p_task->ddl = ddl;
    // 本函数只负责进程最开始的初始化,不负责进程中途设置esp和eip
    // (什么进程需要在中途使用系统调用的方式来设置esp和eip啊?)
    // 认为:进程一经建立即可运行,无需像java那样手动start()
    set_task_state(pid, STATE_READY);
}

// pid就是任务状态段在GDT当中的索引(同时也是在tasks数组当中的索引)
// 必须保持数据的长久一致性
uint16_t get_pid_empty()
{
    // 直接遍历GDT以寻找需要的位置
    // 因为GDT的0号位置是不可用的,所以要专门绕开
    for (uint16_t i = 1; i < MAX_GDT_TERMS; i++)
        if (gdt[i].low == 0 && gdt[i].high == 0) // 思路:寻找空描述符,空描述符肯定是没有相应进程的
            return i;
    return NOT_FOUND;
}

// 其他字段:段界限默认达到最大值,TYPE=1001
// DPL只有低4位有效
void make_gdt_desc_for_tss(gdt_desc *p_desc, unsigned int base_addr, uint8_t DPL)
{
    unsigned int base_addr0_15 = base_addr & 0xFFFF;
    unsigned int base_addr16_23 = base_addr & 0xFF0000, base_addr24_31 = base_addr & 0xFF000000;
    p_desc->high = base_addr24_31 | 0xF8900 | (base_addr16_23 >> 16) | (DPL << 12);
    p_desc->low = (base_addr0_15 << 16) | 0xFFFF;
}

// 每一次创建进程都需要依次申请空白PCB,初始化PCB,太麻烦
// WegOS是面向pid的,所以这个函数直接返回相应的pid
uint16_t create_task(unsigned int flag, unsigned int eip, unsigned int esp, unsigned int ddl)
{
    uint16_t pid = get_pid_empty();
    task_init(pid, flag, eip, esp, ddl);
    return pid;
}

// 00000010 00000010
// unsigned int get_default_eflags()
// {
//     unsigned int eflags = 0;
//     setbit(&eflags, EFLAGS_IF, true);
//     setbit(&eflags, EFLAGS_DEFAULT_ONE, true);
//     return eflags;
// }

// 本函数会设置进程状态并且完成相应的必须的操作
// 特别的,如果设置为就绪态,会直接跳转到该进程处执行
void set_task_state(uint16_t pid, task_state_t state)
{
    // printf("set_task_state,pid=");
    // PRINT_HEX(pid);
    // printf(",state=");
    // PRINT_HEX(state);
    // println();
    task_manager.tasks[pid].state = state;
    switch (state)
    {
    case STATE_READY:
        pid_enqueue(&task_manager.pid_queue_ready, pid);
        break;
    case STATE_RUNNING:
#ifdef _DEBUG
        // if (pid == 7)
        // {
        //     int x = 0x1145;
        // }
#endif
        task_manager.pid_running = pid;
        jmpf(pid << 3, 0);
        break;
    case STATE_BLOCKED:
        pid_enqueue(&task_manager.pid_queue_blocked, pid);
        break;
    case STATE_SLEEPING:
        pid_enqueue(&task_manager.pid_queue_sleeping, pid);
        break;
    }
}