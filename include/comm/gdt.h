#pragma once
#ifndef DESC_H
#define DESC_H

#include "stdint.h"

// 低位部分
// #define GDT_LIMIT0_15 0xFFFF
// #define GDT_BASE0_15 0xFFFF0000

// 高位部分
#define GDT_BASE16_23 0xFF

#define GDT_TYPE_START /*0xF00*/ 8
#define GDT_TYPE_END 12

#define GDT_S_START 12
#define GDT_S_END 13
// #define GDT_DPL 0x6000
// #define GDT_P 0x8000
// #define GDT_LIMIT16_19 0xF0000
// #define GDT_AVL 0x100000 // 本宏不会被使用
// #define GDT_L 0x200000   // 本宏也不会被使用
// #define GDT_D_B 0x400000
// #define GDT_G 0x800000
// #define GDT_BASE24_31 0xFF000000

// // 高位部分的一些更细的东西
// #define GDT_TYPE_X 0x800
// #define GDT_TYPE_E 0x400 // 仅用于数据段
// #define GDT_TYPE_C 0x400 // 仅用于代码段
// #define GDT_TYPE_W 0x200 // 仅用于数据段
// #define GDT_TYPE_R 0x200 // 仅用于代码段
// #define GDT_TYPE_A 0x100

#define GDT_KERNEL_CODE_SEG 0x8
#define GDT_KERNEL_DATA_SEG 0x10
#define GDT_USER_CODE_SEG (0x18 | 3) // |3是为了避免RPL错误
#define GDT_USER_DATA_SEG (0x20 | 3)

#define DPL_KERNEL 0
#define DPL_USER 3

#define MAX_GDT_TERMS 8192

// 全局(局部?)描述符表总体结构
typedef struct
{
    uint32_t low, high;
} gdt_desc, ldt_desc, idt_desc;

#endif