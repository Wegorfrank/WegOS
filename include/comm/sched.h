#pragma once
#ifndef SCHED_H
#define SCHED_H

#include "stdint.h"
#include "data_struct.h"
#include "task.h"
#define TIME_PIECE 100

// 进程管理的总的数据结构
typedef struct task_manager_t
{
    task_t tasks[MAX_GDT_TERMS];
    // 以下信息全是为了方便而设置,必须保证它们和tasks的数据一致性
    uint32_t schdule_counter; // 时间类参数一律用32位
    uint16_t pid_running;     // -1表示当前没有任何进程在运行
    // 就绪队列和阻塞队列
    pid_queue_t pid_queue_ready, pid_queue_blocked, pid_queue_sleeping;
} task_manager_t;

// 408选择题:
// 下列事件中,可能引发进程调度的有
typedef enum scheduler_reason_t
{
    REASON_TIME_PIECE_OVER, // 时间片到
    REASON_TASK_YIELD,      // 主动出让CPU
    REASON_TASK_SLEEP,      // 进程进入睡眠态
    REASON_TASK_BLOCKED,    // 阻塞
    REASON_TASK_EXIT,       // 进程正常结束
} scheduler_reason_t;

void schedule(scheduler_reason_t reason);
void sched_init(void);
void switch_to(uint16_t pid, scheduler_reason_t reason);
void task_manager_init(task_manager_t *p_task_manager);
void sys_sleep(uint32_t counter);
void sys_exit(uint32_t code);
void check_task_affairs(void);
void check_wakeup(void);
void check_schedule(void);

#endif