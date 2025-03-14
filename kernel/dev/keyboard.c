#include "interrupt.h"
#include "keyboard.h"
#include "stdint.h"
#include "console.h"
#include "asm.h"
#include "string.h"

static keyboard_state current_keyboard_state;

void keyboard_interrupt_handler();

void keyboard_init()
{
    keyboard_state_init(&current_keyboard_state);
    // printf("keyoard_init:current_keyboard_state.left_shift_pressed=");
    // printf(get_itoa(current_keyboard_state.left_shift_pressed, 10));
    // println();
    set_idt(KEYBOARD_INTERRUPT, (uint32_t)keyboard_interrupt_handler);
    interrupt_enable(KEYBOARD_INTERRUPT);
}

void keyboard_state_init(keyboard_state *p_keyboard_state)
{
    p_keyboard_state->left_alt_pressed = false;
    p_keyboard_state->right_alt_pressed = false;
    p_keyboard_state->caps_pressed = false;
    p_keyboard_state->left_shift_pressed = false;
    p_keyboard_state->right_shift_pressed = false;
    p_keyboard_state->ctrl_pressed = false;
}

// 李老师的这张表很好用,直接拿过来了
static const ascii_case_t scan_to_ascii[256] = {
    {false, ' ', ' '},             // 空
    {false, ' ', ' '},             // 空
    {true, '1', '!'},              // 2
    {true, '2', '@'},              // 3
    {true, '3', '#'},              // 4
    {true, '4', '$'},              // 5
    {true, '5', '%'},              // 6
    {true, '6', '^'},              // 7
    {true, '7', '&'},              // 8
    {true, '8', '*'},              // 9
    {true, '9', '('},              // 0xA
    {true, '0', ')'},              // 0xB
    {true, '-', '_'},              // 0xC
    {true, '=', '+'},              // 0xD
    {false, ASCII_DEL, ASCII_DEL}, // 0xE(本人电脑上,认为退格键属于特殊按键)
    {true, '\t', '\t'},            // 0xF
    {true, 'q', 'Q'},              // 0x10
    {true, 'w', 'W'},              // 0x11
    {true, 'e', 'E'},              // 0x12
    {true, 'r', 'R'},              // 0x13
    {true, 't', 'T'},              // 0x14
    {true, 'y', 'Y'},              // 0x15
    {true, 'u', 'U'},              // 0x16
    {true, 'i', 'I'},              // 0x17
    {true, 'o', 'O'},              // 0x18
    {true, 'p', 'P'},              // 0x19
    {true, '[', '{'},              // 0x1A
    {true, ']', '}'},              // 0x1B
    {true, '\n', '\n'},            // 0x1C
    {false, ' ', ' '},             // 空
    {true, 'a', 'A'},              // 0x1E
    {true, 's', 'B'},              // 0x1F
    {true, 'd', 'D'},              // 0x20
    {true, 'f', 'F'},              // 0x21
    {true, 'g', 'G'},              // 0x22
    {true, 'h', 'H'},              // 0x23
    {true, 'j', 'J'},              // 0x24
    {true, 'k', 'K'},              // 0x25
    {true, 'l', 'L'},              // 0x26
    {true, ';', ':'},              // 0x27
    {true, '\'', '"'},             // 0x28
    {true, '`', '~'},              // 0x29
    {false, ' ', ' '},             // 空
    {true, '\\', '|'},             // 0x2B
    {true, 'z', 'Z'},              // 0x2C
    {true, 'x', 'X'},              // 0x2D
    {true, 'c', 'C'},              // 0x2E
    {true, 'v', 'V'},              // 0x2F
    {true, 'b', 'B'},              // 0x30
    {true, 'n', 'N'},              // 0x31
    {true, 'm', 'M'},              // 0x32
    {true, ',', '<'},              // 0x33
    {true, '.', '>'},              // 0x34
    {true, '/', '?'},              // 0x35
    {false, ' ', ' '},             // 空
    {false, ' ', ' '},             // 空
    {false, ' ', ' '},             // 空
    {true, ' ', ' '},              // 0x39(这个不是空值!)
    {false, ' ', ' '},             // 空
    {false, ' ', ' '},             // 空
    {false, ' ', ' '},             // 空
    {false, ' ', ' '},             // 空
    {false, ' ', ' '},             // 空
    {false, ' ', ' '},             // 空
};

void on_keyboard()
{
    // VISUAL_DEBUG_DO_SOMETHING();
    // 例行判断是否真的有数据
    if (inb(0x64) & 1)
    {
        // 有数据
        on_key(inb(0x60));
    }
    pic_send_eoi(KEYBOARD_INTERRUPT);
}

void on_usual_key_pressed(char ascii_code)
{
    putchar(ascii_code);
}

void on_usual_key_released(char ascii_code)
{
}

// 应该定义一个总的扫描码的处理函数
void on_key(uint8_t scan_code)
{
    // printf("%scancode:");
    // printf(get_itoa(scan_code, 16));
    // println();
    // 去除最高位之后的扫描码
    uint8_t scan_code_with_digit7_stripped = scan_code & 0x7F;
    char real_key = should_enable_upper_case() ? scan_to_ascii[scan_code_with_digit7_stripped].caps_ascii_code : scan_to_ascii[scan_code_with_digit7_stripped].normal_ascii_code;
    // 根据通码断码进行相应的处理
    if (scan_code & 0x80)
    {
        if (is_usual_key(scan_code)) // 断码由用户自行定义并处理
            on_usual_key_released(real_key);
        else
        {
            switch (scan_code_with_digit7_stripped)
            {
            case KEY_LSHIFT:
                NOT(current_keyboard_state.left_shift_pressed);
                break;
            default:
                break;
            }
        }
    }
    else
    {
        // printf("%intotongma\n");
        // 通码
        if (is_usual_key(scan_code))
            on_usual_key_pressed(real_key);
        else
        {
            // printf("%special key\n");
            // 只对"按下"特殊键进行处理
            // 一定要把最高位去掉然后再做判断!
            switch (scan_code_with_digit7_stripped)
            {
            case KEY_BACKSPACE:
                // printf("%into backspace\n");
                print_back();
                break;
            case KEY_CAPS:
                NOT(current_keyboard_state.caps_pressed);
                break;
            case KEY_LSHIFT: // 右shift键存在一些争议,就先不做吧
                NOT(current_keyboard_state.left_shift_pressed);
                break;
            }
        }
    }
}

static inline bool is_usual_key(uint8_t scan_code)
{
    uint8_t scan_code_with_digit7_stripped = scan_code & 0x7F;
    return scan_to_ascii[scan_code_with_digit7_stripped].is_valid;
}

static inline bool should_enable_upper_case()
{
    // printf("current_keyboard_state.left_shift_pressed=");
    // printf(get_itoa(current_keyboard_state.left_shift_pressed, 10));
    // println();
    return current_keyboard_state.caps_pressed ^ current_keyboard_state.left_shift_pressed;
}
