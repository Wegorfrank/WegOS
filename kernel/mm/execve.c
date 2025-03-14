#include "execve.h"
#include "memory.h"
#include "v_memory.h"
#include "stdint.h"
#include "console.h"
#include "fs.h"
#include "disk.h"
#include "task.h"
#include "sched.h"

#define TEMP_CODE_SIZE_MAX 4096
#define TEMP_CODE_START 0x80000000

extern task_manager_t task_manager;

// 创建相应的进程
void sys_execve(const char *path)
{
    char code_buf[TEMP_CODE_SIZE_MAX];
    file_cached_t file_cached;
    if (load_file_cached(path, &file_cached) == FILE_RESULT_SUCCESS)
    {
        read_all(&file_cached.file, code_buf);
        int code_swap_sector = swap_alloc();
        // printf("code_swap_sector=");
        // PRINT_HEX(code_swap_sector);
        write_disk(code_swap_sector, 8, code_buf); // 把刚刚读到的东西全部搬到磁盘交换区
        unsigned int pid = create_task(TASK_FLAG_KERNEL, TEMP_CODE_START, INIT_ESP_VMA, 0xFF);
        unsigned int cr3 = task_manager.tasks[pid].tss.cr3;
        // 因为swap_sector_set有一个前提条件是相应的页表存在,所以我要提前为它分配页表
        set_mmap(cr3, TEMP_CODE_START, 0, PAGE_TABLE_FLAG_KERNEL_READ_WRITE); // 最后两个参数可以随便填.调用set_mmap可以在尽可能少破坏的前提下为它分配相应的页表
        // 对进程页表的交换区方面进行初始化
        swap_sector_set(cr3, TEMP_CODE_START, code_swap_sector);
        // 怎么缺页中断处理程序没发挥作用啊?我决定直接分配物理页给进程
        // unsigned int code_physical = load_memory(cr3, TEMP_CODE_START, PAGE_TABLE_FLAG_KERNEL_READ_WRITE);
        // memcpy((void *)code_physical, (void *)code_buf, PAGE_SIZE);
        // 栈不需要什么初始化,所以直接分配给进程
        load_memory(cr3, INIT_ESP_VMA, PAGE_TABLE_FLAG_KERNEL_READ_WRITE);
    }
}