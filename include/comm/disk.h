// 磁盘有关的操作到底应该放在comm下,还是一个设置为文件系统的一部分呢?
// 这不关键,之后如果文件系统要大量使用,就放到文件系统当中
#pragma once

#include "stdint.h"
#include "asm.h"
#ifndef DISK_H
#define DISK_H

// 单个扇区大小
#define SECTOR_SIZE 0x200
// 把内核需要加载到的目标扇区放在disk.h或许是一个败笔...
#define KERNEL_DEST 128
#define KERNEL_MAX_SIZE 512

// static void read_disk(int sector, int sector_count, uint8_t * buf) {
//     outb(0x1F6, (uint8_t) (0xE0));

// 	outb(0x1F2, (uint8_t) (sector_count >> 8));
//     outb(0x1F3, (uint8_t) (sector >> 24));		// LBA参数的24~31位
//     outb(0x1F4, (uint8_t) (0));					// LBA参数的32~39位
//     outb(0x1F5, (uint8_t) (0));					// LBA参数的40~47位

//     outb(0x1F2, (uint8_t) (sector_count));
// 	outb(0x1F3, (uint8_t) (sector));			// LBA参数的0~7位
// 	outb(0x1F4, (uint8_t) (sector >> 8));		// LBA参数的8~15位
// 	outb(0x1F5, (uint8_t) (sector >> 16));		// LBA参数的16~23位

// 	outb(0x1F7, (uint8_t) 0x24);

// 	// 读取数据
// 	uint16_t *data_buf = (uint16_t*) buf;
// 	while (sector_count-- > 0) {
// 		// 每次扇区读之前都要检查，等待数据就绪
// 		while ((inb(0x1F7) & 0x88) != 0x8) {}

// 		// 读取并将数据写入到缓存中
// 		for (int i = 0; i < SECTOR_SIZE / 2; i++) {
// 			*data_buf++ = inw(0x1F0);
// 		}
// 	}
// }

// 保护模式下只是BIOS用不了了而已!输入输出指令在特权级足够的情况下仍然可以正常使用
// sector->起始扇区
// sector_count->扇区数目
// buf->读取到内存当中哪个位置(这是一个逻辑地址)
// 如果后面出现了物理地址的需要,再重新定义一个函数
static void read_disk(uint32_t sector, uint32_t sector_count, uint8_t *buf)
{
    // 既然是读取扇区,为什么需要使用outb指令呢?
    // 因为需要用输出指令向IO端口发送命令和地址
    outb(0x1F6, 0xE0);
    // 扇区的高8位
    outb(0x1F2, (uint8_t)(sector_count >> 8)); // 需要读取的扇区数,高位部分
    outb(0x1F3, (uint8_t)(sector >> 24));
    outb(0x1F4, 0);
    outb(0x1F5, 0);
    // 地址
    outb(0x1F2, (uint8_t)sector_count);
    outb(0x1F3, (uint8_t)sector);
    outb(0x1F4, (uint8_t)(sector >> 8));
    outb(0x1F5, (uint8_t)(sector >> 16));
    // 控制(命令寄存器)
    outb(0x1F7, 0x24);
    // 状态检查
    uint16_t *data_buf = (uint16_t *)buf;
    while (sector_count--)
    {
        // 计组教材的经典代码,在这里体现
        while ((inb(0x1F7) & 0x88) != 0x8)
            ;
        // 为了方便,不检查读出来的数据有没有错误
        for (int i = 0; i < SECTOR_SIZE / 2; i++)
        {
            *(data_buf++) = inw(0x1F0);
        }
    }
}

static void write_disk(uint32_t sector, uint32_t sector_count, uint8_t *buf)
{
    outb(0x1F6, 0xE0); // 0xE0表示主驱动器
    outb(0x1F2, (uint8_t)(sector_count >> 8));
    // 写入扇区号的高位部分
    outb(0x1F3, (uint8_t)(sector >> 24));
    outb(0x1F4, 0);
    outb(0x1F5, 0);
    // 写入需要写入的扇区数（低位部分）
    outb(0x1F2, (uint8_t)sector_count);
    // 写入扇区号的低位部分
    outb(0x1F3, (uint8_t)sector);
    outb(0x1F4, (uint8_t)(sector >> 8));
    outb(0x1F5, (uint8_t)(sector >> 16));
    // 写入命令到控制寄存器（命令寄存器）
    outb(0x1F7, 0x34); // 0x34 是写扇区的命令
    // 将数据从缓冲区写入磁盘
    uint16_t *data_buf = (uint16_t *)buf;
    while (sector_count--)
    {
        // 等待磁盘准备好接受数据
        while ((inb(0x1F7) & 0x88) != 0x8)
            ;
        // 将数据写入磁盘数据端口
        for (int i = 0; i < SECTOR_SIZE / 2; i++)
        {
            outw(0x1F0, *(data_buf++));
        }
    }
    // 发送缓存刷新命令，确保数据写入磁盘
    outb(0x1F7, 0xE7);
}

#endif