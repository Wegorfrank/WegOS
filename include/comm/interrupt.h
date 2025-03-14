#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "gdt.h"

// 中断相关的宏定义

#define PIC0_ICW1 0x20
#define PIC0_ICW2 0x21
#define PIC0_ICW3 0x21
#define PIC0_ICW4 0x21
#define PIC0_OCW2 0x20
#define PIC0_IMR 0x21

#define PIC1_ICW1 0xa0
#define PIC1_ICW2 0xa1
#define PIC1_ICW3 0xa1
#define PIC1_ICW4 0xa1
#define PIC1_OCW2 0xa0
#define PIC1_IMR 0xa1

#define PIC_ICW1_ICW4 (1 << 0)     // 1 - 需要初始化ICW4
#define PIC_ICW1_ALWAYS_1 (1 << 4) // 总为1的位
#define PIC_ICW4_8086 (1 << 0)     // 8086工作模式

#define PIC_OCW2_EOI (1 << 5) // 1 - 非特殊结束中断EOI命令

#define PIC_START 0x20 // PIC中断起始号

#define INTEL_INTRRUPT_TYPES 256

#define PAGE_FAULT_INTERRUPT 0xE
#define TIMER_INTERRUPT 0x20
#define KEYBOARD_INTERRUPT 0x21

void ignore_int();
void set_idt(uint8_t interrupt_num, uint32_t handler);
void make_idt_desc(idt_desc *p_desc, uint16_t selector, uint32_t offset);
void interrupt_init();
void set_idt(uint8_t interrupt_num, uint32_t handler);
void pic_send_eoi(int interrupt_num);
void pic_init(void);
void interrupt_enable(int interrupt_num);
void interrupt_disable(int interrupt_num);

#endif