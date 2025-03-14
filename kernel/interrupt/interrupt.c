#include "asm.h"
#include "gdt.h"
#include "interrupt.h"

// 这个数组是永久性的,无需加temp
idt_desc idt[INTEL_INTRRUPT_TYPES];

// int is short for interrupt, not integer
void ignore_int();
void set_idt(uint8_t interrupt_num, uint32_t handler);

// 打开时钟中断之前需要做什么?肯定要先学会设置中断描述符啊!
// 默认此函数工作在保护模式
// 其他参数:默认DPL=0,P=1,D=1
// 用指针来返回!我不熟悉结构体返回值的汇编码,所以要选择我熟悉的技术
void make_idt_desc(idt_desc *p_desc, uint16_t selector, uint32_t offset)
{
    uint16_t offset_16_31 = offset >> 16;
    uint16_t offset_0_15 = offset % (1 << 16);
    p_desc->high = (offset_16_31 << 16) + 0x8E00; // 0x8E00是那堆默认参数产生的magic number
    p_desc->low = (selector << 16) + offset_0_15;
}

void interrupt_init()
{
    lidt((uint32_t)idt, sizeof(idt_desc) * INTEL_INTRRUPT_TYPES);
    pic_init();
    for (int i = 0; i < INTEL_INTRRUPT_TYPES; i++)
    {
        set_idt(i, (uint32_t)ignore_int);
    }
}

void set_idt(uint8_t interrupt_num, uint32_t handler)
{
    idt_desc desc;
    make_idt_desc(&desc, GDT_KERNEL_CODE_SEG, handler);
    idt[interrupt_num] = desc;
}

void pic_send_eoi(int interrupt_num)
{
    interrupt_num -= PIC_START;
    // 从片也可能需要发送EOI
    if (interrupt_num >= 8)
    {
        outb(PIC1_OCW2, PIC_OCW2_EOI);
    }
    outb(PIC0_OCW2, PIC_OCW2_EOI);
}

// 我只是代码的搬运工
void pic_init(void)
{
    // 边缘触发，级联，需要配置icw4, 8086模式
    outb(PIC0_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
    // 对应的中断号起始序号0x20
    outb(PIC0_ICW2, PIC_START);
    // 主片IRQ2有从片
    outb(PIC0_ICW3, 1 << 2);
    // 普通全嵌套、非缓冲、非自动结束、8086模式
    outb(PIC0_ICW4, PIC_ICW4_8086);
    // 边缘触发，级联，需要配置icw4, 8086模式
    outb(PIC1_ICW1, PIC_ICW1_ICW4 | PIC_ICW1_ALWAYS_1);
    // 起始中断序号，要加上8
    outb(PIC1_ICW2, PIC_START + 8);
    // 没有从片，连接到主片的IRQ2上
    outb(PIC1_ICW3, 2);
    // 普通全嵌套、非缓冲、非自动结束、8086模式
    outb(PIC1_ICW4, PIC_ICW4_8086);
    // 禁止所有中断, 允许从PIC1传来的中断
    outb(PIC0_IMR, 0xFF & ~(1 << 2));
    outb(PIC1_IMR, 0xFF);
}

// 本函数说明了:中断屏蔽字确实是可以通过软件设置或保存的
void interrupt_enable(int interrupt_num)
{
    if (interrupt_num < PIC_START)
    {
        return;
    }

    interrupt_num -= PIC_START;
    if (interrupt_num < 8)
    {
        uint8_t mask = inb(PIC0_IMR) & ~(1 << interrupt_num);
        outb(PIC0_IMR, mask);
    }
    else
    {
        interrupt_num -= 8;
        uint8_t mask = inb(PIC1_IMR) & ~(1 << interrupt_num);
        outb(PIC1_IMR, mask);
    }
}

void interrupt_disable(int interrupt_num)
{
    if (interrupt_num < PIC_START)
    {
        return;
    }

    interrupt_num -= PIC_START;
    if (interrupt_num < 8)
    {
        uint8_t mask = inb(PIC0_IMR) | (1 << interrupt_num);
        outb(PIC0_IMR, mask);
    }
    else
    {
        interrupt_num -= 8;
        uint8_t mask = inb(PIC1_IMR) | (1 << interrupt_num);
        outb(PIC1_IMR, mask);
    }
}