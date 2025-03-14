#include "asm.h"
#include "gdt.h"

/**
 * 我的内存规划:
 */

static inline void open_A20();
static inline void enter_protected_mode();

void kernel_datasg_init_16();

//                                    代码段,base=0,DPL=0        数据段,base=0,DPL=0
static gdt_desc gdt_temp[] = {{0, 0}, {0x0000FFFF, 0x00CF9A00}, {0x0000FFFF, 0x00CF9200}};

// 本函数是跳进来的,千万不能返回
void loader_entry()
{
    // 现在,必须分析一下loader需要干什么事情
    // 主要工作就是:关中断,设置全局描述符表,开启地址线,进入保护模式
    cli();
    lgdt((uint32_t)gdt_temp, (uint16_t)sizeof(gdt_temp));
    open_A20();
    enter_protected_mode();
    // 逆天编译器居然在16位下使用4字节压栈的call-ret指令,还没法让loader_entry自然返回
    // 就只能用这种不伦不类的方式来实现控制转移了
    kernel_datasg_init_16();
}

static inline void enter_protected_mode()
{
    write_cr0(read_cr0() | 1);
}

static inline void open_A20()
{
    outb(0x92, inb(0x92) | 0x2);
}