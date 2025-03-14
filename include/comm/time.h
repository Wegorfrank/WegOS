#pragma once

#include "stdint.h"

void CMOS_RTC_hardware_init(uint8_t interrupt_num, uint32_t interval);
void set_idt(uint8_t interrupt_num, uint32_t handler);
void timer_interrupt_handler();
void on_timer(void);
void time_init(void);