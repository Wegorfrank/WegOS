#include "stdint.h"
#include "string.h"
#include "console.h"

int printf(const char *fmt, ...);

// 遇到\0停止
int strlen(const char *text)
{
    int i = 0;
    for (const char *p_c = text; (*p_c) != 0; p_c++, i++)
        ;
    return i;
}

char *strcpy(char *dest, const char *src)
{
    int length = strlen(src);
    for (int i = 0; i <= length; i++) // 是<=,这是为了顺便把结束符也加上
        dest[i] = src[i];
}

// 丐版strcmp函数.相等->0,不相等->0
int strcmp(const char *str1, const char *str2)
{
    if (strlen(str1) != strlen(str2))
        return 1;
    int length = strlen(str1);
    for (int i = 0; i < length; i++)
        if (str1[i] != str2[i])
            return 1;
    return 0;
}

void *memcpy(void *dest, void *source, unsigned int n)
{
    // printf("memcpy\n");
    for (int i = 0; i < n; i++)
    {
        // printf("memcpy,source[i]=");
        // putchar(((char *)source)[i]);
        ((char *)dest)[i] = ((char *)source)[i];
    }
}

void get_parent_dir_name(const char *path, char *parent_dir_name)
{
    int len = strlen(path);
    if (len == 0)
    {
        parent_dir_name[0] = '\0';
        return;
    }
    int new_len = len - 1; // 去掉末尾的'/'
    int pos = -1;
    // 从后往前查找最后一个'/'
    for (int i = new_len - 1; i >= 0; --i)
    {
        if (path[i] == '/')
        {
            pos = i;
            break;
        }
    }
    if (pos == -1)
    {
        parent_dir_name[0] = '\0';
    }
    else
    {
        memcpy((void *)parent_dir_name, (void *)path, pos + 1);
        parent_dir_name[pos + 1] = '\0';
    }
}

#ifdef DEPRECATED
// 对格式串进行解析
int vsprintf(char *buf, const char *fmt, ...)
{
    // 这需要一点小小的算法
    int length = strlen(fmt);
    int buf_index = 0, args_index = 0, fmt_index;
    bool fmt_symbol_encountered = false;
    uint32_t *args = ((uint32_t)&fmt) + 1;
    // 最多32位数,够用了
    char num_text[32];
    for (fmt_index = 0; fmt_index < length; fmt_index++)
    {
        char c = fmt[fmt_index];
        if (c == '%')
        {
            fmt_symbol_encountered = true;
        }
        else if (fmt_symbol_encountered)
        {
            fmt_symbol_encountered = false;
            switch (c)
            {
            case 'x':
                itoa(args[args_index], 0x10, num_text);
                // 将数字对应的文本复制到目标位置
                char *src = num_text, *dest = buf + buf_index;
                int i = 0;
                for (; (*src) != '\0'; src++, dest++, i++, buf_index++)
                    dest[i] = src[i];
                break;
            }
        }
        else
        {
            // 是普通字符,正常输出
            buf[buf_index++] = c;
        }
    }
}
#endif

void reverse(char str[], int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end)
    {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// itoa function: num -> integer, radix -> base, text -> result string
void itoa(int32_t num, uint8_t radix, char text[]) // 基数参数使用8位是因为目前的计算机顶多能表示十六进制
{
    if (num == 0)
    {
        text[0] = '0';
        text[1] = '\0';
        return;
    }
    int i = 0;
    int isNegative = 0;
    if (num < 0)
    {
        isNegative = 1;
        num = -num;
    }
    char buffer[32];
    while (num != 0)
    {
        int remainder = num % radix;
        buffer[i++] = (remainder > 9) ? (remainder - 10) + 'a' : remainder + '0';
        num = num / radix;
    }
    if (isNegative)
    {
        buffer[i++] = '-';
    }
    buffer[i] = '\0';
    reverse(buffer, i);
    for (int j = 0; j < i; j++)
    {
        text[j] = buffer[j];
    }
    text[i] = '\0';
}

// 参数c会被强转为char
// 之所以这样定义完全是为了防止conflict...built-in编译警告
void *memset(void *s, uint32_t c, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++)
        *((uint8_t *)(s + i)) = c;
}

#ifdef _DEBUG

static char text[32];

// 这是一个纯粹的调试方便的函数
char *get_itoa(uint32_t num, uint8_t radix)
{
    itoa(num, radix, text);
    return text;
}
#endif