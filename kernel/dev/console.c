// 我知道这堆函数该放哪里了!
#include "stdint.h"
#include "string.h"
#include "interrupt.h"
#include "asm.h"
#include "console.h"

static cursor_state current_cursor_state;

// Linux简单粗暴,直接设置了一个静态全局缓冲区用于存格式串转换之后的新串
static char buf[1024];

void write(const char *buf, int length)
{
    // write的对象还可以是文件,但是我还没有实现文件,所以暂时先选择打印操作
    // 搞那么复杂干什么?有什么需要改的之后再改也不迟!
    for (int i = 0; i < length; i++)
    {
        putchar(buf[i]);
    }
}

// 参数设置为uint32_t也是为了避免编译警告
int putchar(uint32_t c)
{
    // 换行符的显示也需要操作系统的支持
    if (c == '\n')
    {
        println();
    }
    else if (c == '\b')
    {
        print_back();
    }
    else
    {
        // 普通字符
        *((uint8_t *)(current_cursor_state.p_last_char)) = c;
        cursor_state_inc(&current_cursor_state);
        check_scroll_up();
    }
}

int printf(const char *fmt, ...)
{
    write(fmt, strlen(fmt));
    // 返回值屁用没有
    return 0;
}

void cls()
{
    cursor_state_init(&current_cursor_state);
    for (int i = 0; i < ROWS * COLS; i++)
    {
        *((uint8_t *)((uint16_t *)VIDEO_MEM_TEXT_MODE_START + i)) = ' ';
    }
}

void println()
{
    // 没有明显的位数要求的整数一律直接使用int类型
    int row = ((uint32_t)current_cursor_state.p_last_char - (uint32_t)VIDEO_MEM_TEXT_MODE_START) / (sizeof(uint16_t) * COLS);
    set_cursor_state(&current_cursor_state, ((uint16_t *)VIDEO_MEM_TEXT_MODE_START) + (row + 1) * COLS);
    // 滚屏的处理
    check_scroll_up();
}

void print_back()
{
    // 很多操作系统关于\b的打印不会覆盖已经打印出来的内容
    // 这怎么合理呢?要构建我自己的操作系统体系
    set_cursor_state(&current_cursor_state, current_cursor_state.p_last_char - 1);
    *((uint8_t *)current_cursor_state.p_last_char) = ' ';
}

void tty_init()
{
    cls();
    keyboard_init();
}

// 控制台的滚动操作
void scroll_up()
{
    // 滚屏的实质是把每一行的字符复制到上一行
    for (uint16_t *ptr = (uint16_t *)VIDEO_MEM_TEXT_MODE_START; (uint32_t)ptr < VIDEO_MEM_TEXT_MODE_START + sizeof(uint16_t) * ROWS * COLS; ptr++)
        *ptr = *(ptr + COLS);
    // 输出位置也需要跟着移动
    set_cursor_state(&current_cursor_state, current_cursor_state.p_last_char - COLS);
}

void check_scroll_up()
{
    // 我居然被这种弱智bug困住了...
    while ((uint32_t)current_cursor_state.p_last_char > (uint32_t)((uint16_t *)VIDEO_MEM_TEXT_MODE_START + ROWS * COLS))
        scroll_up();
}

// 为了保证数据一致性,这个函数是关键
void set_cursor_state(cursor_state *p_cursor_state, uint16_t *p_last_char)
{
    p_cursor_state->p_last_char = p_last_char;
    // 计算相应的行和列
    uint32_t offset = p_last_char - (uint16_t *)VIDEO_MEM_TEXT_MODE_START;
    p_cursor_state->cursor_col = offset % COLS;
    p_cursor_state->cursor_row = offset / COLS;
    gotoxy(p_cursor_state->cursor_col, p_cursor_state->cursor_row);
}

void cursor_state_init(cursor_state *p_cursor_state)
{
    set_cursor_state(p_cursor_state, (uint16_t *)VIDEO_MEM_TEXT_MODE_START);
}

void cursor_state_inc(cursor_state *p_cursor_state)
{
    p_cursor_state->p_last_char++;
    set_cursor_state(p_cursor_state, p_cursor_state->p_last_char);
}

void gotoxy(uint8_t col, uint8_t row)
{
    // cli();
    uint16_t pos = row * COLS + col;
    // 直接操作光标位置的函数
    outb(CURSOR_CONTROL_PORT, 0xE);   // 0xE->高字节
    outb(CURSOR_DATA_PORT, pos >> 8); // sizeof(uint8_t)!=8
    outb(CURSOR_CONTROL_PORT, 0xF);   // 0xF->低字节
    outb(CURSOR_DATA_PORT, 0xFF & pos);
    // sti();
}

#ifdef _DEBUG
void emphasize(char *text)
{
    while (true)
        printf(text);
}
#endif