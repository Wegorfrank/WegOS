// 为了更好的可读性,需要定义相关类型
#pragma once

#ifndef STDINT_H
#define STDINT_H

#define UINT32_MAX 0xFFFFFFFF
#define INT32_MAX 0x7FFFFFFF

// 这让我想起来我408考试中C语言类型转换基础题居然做错的事情...
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
// 64位的东西不要轻易使用,免得出现难以调试的Bug
typedef unsigned long long uint64_t;

// 把signed加上
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef _Bool bool;

// 类型问题还有点复杂..暂时把布尔类型也放在stdint里面
#define true (bool)1
#define false (bool)0

#define NOT_FOUND ((uint16_t)(-1))

#endif