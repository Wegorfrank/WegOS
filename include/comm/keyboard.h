#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "interrupt.h"
#include "stdint.h"
#include "asm.h"

#define KBD_PORT_DATA 0x60
#define KBD_PORT_STAT 0x64
#define KBD_PORT_CMD 0x64

#define KBD_STAT_RECV_READY (1 << 0)
#define KBD_STAT_SEND_FULL (1 << 1)

// https://wiki.osdev.org/PS/2_Keyboard
#define KBD_CMD_RW_LED 0xED // 写按键

#define KEY_CAPS 0x3A

#define KEY_E0 0xE0 // E0编码
#define KEY_E1 0xE1 // E1编码
#define ASCII_ESC 0x1b
#define ASCII_DEL 0x7F

#define KEY_CTRL 0x1D            // E0, 1D或1d
#define KEY_RSHIFT /*0x36*/ 0x1D // 经过实测,右shift的扫描码也是0x1D
#define KEY_LSHIFT 0x2A
#define KEY_ALT 0x38 // E0, 38或38

#define KEY_FUNC 0x8000
#define KEY_F1 (0x3B)
#define KEY_F2 (0x3C)
#define KEY_F3 (0x3D)
#define KEY_F4 (0x3E)
#define KEY_F5 (0x3F)
#define KEY_F6 (0x40)
#define KEY_F7 (0x41)
#define KEY_F8 (0x42)
#define KEY_F9 (0x43)
#define KEY_F10 (0x44)
#define KEY_F11 (0x57)
#define KEY_F12 (0x58)

#define KEY_SCROLL_LOCK (0x46)
#define KEY_HOME (0x47)
#define KEY_END (0x4F)
#define KEY_PAGE_UP (0x49)
#define KEY_PAGE_DOWN (0x51)
#define KEY_CURSOR_UP (0x48)
#define KEY_CURSOR_DOWN (0x50)
#define KEY_CURSOR_RIGHT (0x4D)
#define KEY_CURSOR_LEFT (0x4B)
#define KEY_INSERT (0x52)
#define KEY_DELETE (0x53)
#define KEY_BACKSPACE 0xE

#define NOT(var) (var) = !(var)

typedef struct keyboard_state
{
    bool caps_pressed, left_shift_pressed, right_shift_pressed, ctrl_pressed, left_alt_pressed, right_alt_pressed;
} keyboard_state;

typedef struct ascii_case_t
{
    bool is_valid;
    char normal_ascii_code, caps_ascii_code;
} ascii_case_t;

// 声明放在头文件是一个很好的编程习惯
void keyboard_interrupt_handler();
void keyboard_init();
void on_keyboard();
void keyboard_state_init(keyboard_state *the_keyboard_state);

void on_usual_key_pressed(char key);
void on_usual_key_released(char key);
void on_key(uint8_t scan_code);

static inline bool is_usual_key(uint8_t scan_code);
static inline bool should_enable_upper_case();

#endif