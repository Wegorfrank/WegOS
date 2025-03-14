#pragma once
#ifndef TASK_H
#define TASK_H

#include "stdint.h"
#include "gdt.h"
#include "asm.h"
#include "bit.h"

#define TASK_MAX_PAGE_COUNT 8 // 每个进程最多分配8个物理页
#define INIT_EFLAGS 0x202
#define TASK_FLAG_KERNEL 0
#define TASK_FLAG_USER 1
#define IS_TSS(type) ((type == 0b1001) || (type == 0b1011))

// 以下字段全是硬件的要求
typedef struct tss_t
{
    struct tss_t *prev;                                  // 高16位0
    unsigned int esp0, ss0, esp1, ss1, esp2, ss2;        // 各个ss的高16位0
    unsigned int cr3;                                    // 页表基址寄存器
    unsigned int eip, eflags;                            //"断点信息"
    unsigned int eax, ecx, edx, ebx, esp, ebp, esi, edi; // 通用寄存器
    unsigned int es, cs, ss, ds, fs, gs;                 // 全是高16位保留为0
    unsigned int ldt_seg;                                // 高16位保留
    unsigned int io_map_associated;
} tss_t;

// 暂时只考虑进程的五状态模型
typedef enum task_state_t
{
    STATE_READY,
    STATE_RUNNING,
    STATE_BLOCKED,
    STATE_SLEEPING,
    STATE_JUST_INITIALIZED,
    STATE_DYING
} task_state_t;

// PCB
typedef struct task_t
{
    uint16_t pid; // 最多8192个进程,16位pid完全够用
    task_state_t state;
    tss_t tss;                   // 上下文
    unsigned int wakeup_counter; // 进程处于正常状态时,此字段无效
    unsigned int ddl;            // WegOS的特色机制:支持传入一个截止时间参数,进程如果在这个时间还没有运行完毕会被强行杀掉
} task_t;

void task_init(uint16_t pid, unsigned int flag, unsigned int eip, unsigned int esp, unsigned int ddl);
void make_gdt_desc_for_tss(gdt_desc *p_desc, unsigned int base_addr, uint8_t DPL);
uint16_t get_pid_empty();
uint16_t create_task(unsigned int flag, unsigned int eip, unsigned int esp, unsigned int ddl); // 即使设置flag为bool类型,最终也会按照4字节压栈...并不会节省空间
// unsigned int get_default_eflags();
void set_task_state(uint16_t pid, task_state_t state);

static inline bool is_a_tss_descriptor(gdt_desc desc)
{
    return testbits(desc.high, GDT_S_START, GDT_S_END) == 0 && IS_TSS(testbits(desc.high, GDT_TYPE_START, GDT_TYPE_END));
}

#endif
