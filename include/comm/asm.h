#pragma once

#ifndef ASM_H
#define ASM_H

#include "stdint.h"

#define AV __asm__ __volatile__

#define EFLAGS_CF 0
#define EFLAGS_DEFAULT_ONE 1
#define EFLAGS_PF 2
#define EFLAGS_AF 4
#define EFLAGS_ZF 6
#define EFLAGS_SF 7
#define EFLAGS_TF 8
#define EFLAGS_IF 9
#define EFLAGS_DF 10
#define EFLAGS_OF 11

// 0x23->SS段寄存器以及各个数据段寄存器
// 0x1b->CS段寄存器
#define move_to_user_mode()           \
    __asm__("movl %%esp,%%eax\n\t"    \
            "pushl $0x23\n\t"         \
            "pushl %%eax\n\t"         \
            "pushfl\n\t"              \
            "pushl $0x1b\n\t"         \
            "pushl $1f\n\t"           \
            "iret\n"                  \
            "1:\tmovl $0x23,%%ax\n\t" \
            "movw %%ax,%%ds\n\t"      \
            "movw %%ax,%%es\n\t"      \
            "movw %%ax,%%fs\n\t"      \
            "movw %%ax,%%gs" ::: "ax")

#define REP(n, statement)       \
    for (int i = 0; i < n; i++) \
    statement

// gcc逆天特性:识别不了inline
// inline前面要加上static
static inline void cli()
{
    AV("cli");
}

static inline void sti()
{
    AV("sti");
}

static inline uint32_t read_cr0(void)
{
    uint32_t cr0;
    AV("mov %%cr0,%%eax" : "=a"(cr0));
    return cr0;
}

static inline void write_cr0(uint32_t cr0)
{
    AV("mov %%eax,%%cr0" ::"a"(cr0));
}

static inline uint32_t read_cr3(void)
{
    uint32_t cr3;
    AV("mov %%cr3,%%eax" : "=a"(cr3));
    return cr3;
}

static inline void write_cr3(uint32_t cr3)
{
    AV("mov %%eax,%%cr3" ::"a"(cr3));
}

static inline uint32_t read_eflags(void)
{
    uint32_t eflags;

    AV("pushfl\n\tpopl %%eax" : "=a"(eflags));
    return eflags;
}

static inline void write_eflags(uint32_t eflags)
{
    AV("pushl %%eax\n\tpopfl" ::"a"(eflags));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t rv;
    __asm__ __volatile__("inb %[p], %[v]" : [v] "=a"(rv) : [p] "d"(port));
    return rv;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t rv;
    __asm__ __volatile__("in %1, %0" : "=a"(rv) : "dN"(port));
    return rv;
}

static inline void outb(uint16_t port, uint8_t data)
{
    __asm__ __volatile__("outb %[v], %[p]" : : [p] "d"(port), [v] "a"(data));
}

static inline void outw(uint16_t port, uint16_t data)
{
    __asm__ __volatile__("out %[v], %[p]" : : [p] "d"(port), [v] "a"(data));
}

// 本函数执行之前,应当保证已经设置了正确的数据段寄存器
static inline void lgdt(uint32_t base_addr, uint16_t size) // size设置为16位,但是机器仍然会按照32位压栈(即使是16位代码编译)
{
#pragma pack(1)
    // 内存对齐是问题出现的根源!
    struct
    {
        uint16_t limit;
        uint32_t base_addr;
    } gdt;
#pragma pack()

    gdt.base_addr = base_addr;
    gdt.limit = size - 1;

    // 直接用李老师的代码
    AV("lgdt %[g]" ::[g] "m"(gdt));
}

static inline void lidt(uint32_t base_addr, uint16_t size) // size设置为16位,但是机器仍然会按照32位压栈(即使是16位代码编译)
{
#pragma pack(1)
    // 内存对齐是问题出现的根源!
    struct
    {
        uint16_t limit;
        uint32_t base_addr;
    } idt;
#pragma pack()

    idt.base_addr = base_addr;
    idt.limit = size - 1;

    // 直接用李老师的代码
    AV("lidt %[g]" ::[g] "m"(idt));
}

static inline void jmpf(uint32_t cs, uint32_t ip)
{
    uint32_t addr[] = {ip, cs};
    AV("ljmpl *(%[a])" ::[a] "r"(addr));
}

#endif