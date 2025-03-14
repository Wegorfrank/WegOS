#pragma once
#include "stdint.h"
#include "string.h"

#define VIDEO_MEM_TEXT_MODE_START 0xB8000
#define ROWS 25
#define COLS 80

#define CURSOR_CONTROL_PORT 0x3D4
#define CURSOR_DATA_PORT 0x3D5

// 光标位置的三个字段必须时刻保持同步!
// 必要时可以设置为原语
typedef struct cursor_state
{
    uint16_t *p_last_char;
    // 8位足以存放光标的行列
    uint8_t cursor_col, cursor_row;
} cursor_state;

int putchar(uint32_t c);
int printf(const char *fmt, ...);
void cls();
void println();
void print_back();
void keyboard_init();
void set_idt(uint8_t interrupt_num, uint32_t handler);
void tty_init();
void print_back();
void scroll_up();

void set_cursor_state(cursor_state *p_cursor_state, uint16_t *p_last_char); // 这是最终的函数
void cursor_state_init(cursor_state *p_cursor_state);
void cursor_state_inc(cursor_state *p_cursor_state);
void check_scroll_up();

void gotoxy(uint8_t col, uint8_t row);

#ifdef _DEBUG
#define VISUAL_DEBUG_DO_SOMETHING() (*((uint8_t *)VIDEO_MEM_TEXT_MODE_START))++
#define PRINT_HEX(I) (printf(get_itoa(I, 16)))
void emphasize(char *text);
#endif