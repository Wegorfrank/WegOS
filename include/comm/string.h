#pragma once
#ifndef STRING_H
#define STRING_H

int strlen(const char *text);
char *strcpy(char *dest, const char *src);
// 格式串解析函数
int vsprintf(char *buf, const char *fmt, ...);
void itoa(int32_t num, uint8_t radix, char text[]);
void *memset(void *s, uint32_t c, uint32_t n);
void *memcpy(void *dest, void *source, unsigned int n);
int strcmp(const char *str1, const char *str2);
void get_parent_dir_name(const char *path, char *parent_dir);

// 调试专用的函数
char *get_itoa(uint32_t num, uint8_t radix);

#endif