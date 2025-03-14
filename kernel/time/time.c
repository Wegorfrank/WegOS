#include "stdint.h"
#include "asm.h"
#include "interrupt.h"
#include "time.h"
#include "console.h"
#include "sched.h"

// CMOS RTC的工作频率
#define LATCH 1193180

// 开机时间
uint32_t ticks = 0;

void time_init(void)
{
    CMOS_RTC_hardware_init(0x20, 100); // 我之后想办法测一下到底是隔多长时间来一次时钟中断
    set_idt(TIMER_INTERRUPT, (uint32_t)timer_interrupt_handler);
}

// interrupt_num->中断号(为了避免莫名其妙的问题,在使用本函数时默认为0x20)
// interval->每隔多少毫秒触发一次中断
/*一次定时器中断就是一个时间单位.定时器中断周期是10毫秒,那么一切时间函数的单位就是10毫秒;定时器中断周期是100毫秒,那么一切时间函数的单位就是100毫秒*/
void CMOS_RTC_hardware_init(uint8_t interrupt_num, uint32_t interval)
{
    // 时钟初始化的首要任务是正确设置时钟中断
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    // 设置定时器中断号为0x20
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x4);
    outb(0xA1, 0x2);
    outb(0x21, 0x1);
    outb(0xA1, 0x1);
    outb(0x21, 0xFE);
    outb(0xA1, 0xFF);
    outb(0x43, 0x36);
    // 计算定时器的初值
    uint16_t tmo = LATCH / interval;
    outb(0x40, tmo & 0xFF);
    outb(0x40, tmo >> 8);
}

void on_timer(void)
{
    ticks++;
    pic_send_eoi(TIMER_INTERRUPT); // 固定代码,删除之后会导致定时器无法正常工作
    check_task_affairs();
}