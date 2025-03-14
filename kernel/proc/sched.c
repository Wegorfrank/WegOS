#include "task.h"
#include "sched.h"
#include "asm.h"
#include "console.h"
#include "bit.h"
#include "memory.h"

extern gdt_desc gdt[MAX_GDT_TERMS];
extern uint32_t ticks;
extern task_manager_t task_manager;

uint32_t idle_stack_0[PAGE_SIZE / sizeof(uint32_t)]; // 无论一个进程代码量有多小,为它配置的栈都至少要有一页
uint32_t idle_stack_1[PAGE_SIZE / sizeof(uint32_t)]; // 进程即使只负责闲逛,那也需要考虑到它的中断处理的栈

void idle(void);

void sched_init(void)
{
    // printf("&task_manager.pid_running=");
    // PRINT_HEX(&task_manager.pid_running);
    // println();
    // 进程调度的初始化
    task_manager_init(&task_manager);
    // 创建两个占位置的进程
    create_task(TASK_FLAG_KERNEL, (uint32_t)idle, (uint32_t)(&idle_stack_0[PAGE_SIZE / sizeof(uint32_t)]), 0xFF);
    create_task(TASK_FLAG_KERNEL, (uint32_t)idle, (uint32_t)(&idle_stack_1[PAGE_SIZE / sizeof(uint32_t)]), 0xFF);
}

void check_task_affairs(void)
{
    check_wakeup();
    check_schedule();
}

void check_schedule(void)
{
    // printf("esp of 3=");
    // PRINT_HEX(task_manager.tasks[3].tss.esp);
    // printf(",esp of 4=");
    // PRINT_HEX(task_manager.tasks[4].tss.esp);
    // println();
    if (--task_manager.schdule_counter <= 0)
        schedule(REASON_TIME_PIECE_OVER);
}

void schedule(scheduler_reason_t reason)
{
    // println();
    // println();
    task_manager.schdule_counter = TIME_PIECE;
    // 完成了:将值得运行的进程从就绪队列头部删除;切换进程这两大操作
    uint16_t best_pid = pid_dequeue(&task_manager.pid_queue_ready); // 从这个角度看,best_pid不可能刚好是当前进程!
#ifdef _DEBUG
    if (best_pid == NOT_FOUND)
        while (true)
            printf("best_pid calculated to be -1,error!");
#endif
    // printf("selected:best_pid=");
    // PRINT_HEX(best_pid);
    // println();
    switch_to(best_pid, reason);
    // best_pid==-1,pid_running==-1->什么也不做
    // best_pid==-1,pid_running!=-1->什么也不做
    // best_pid!=-1,pid_running==-1->best_pid上CPU
    // best_pid!=-1,pid_running!=-1->best_pid上CPU
}

// 经过理论和实践研究,jmpf函数调用不会导致栈不平衡
// 所以可以大胆的把switch_to定义为函数
void switch_to(uint16_t pid, scheduler_reason_t reason)
{
#ifdef _DEBUG
    if (pid == NOT_FOUND)
        while (true)
            printf("%Trying to switch to -1,error!\n");
#endif
    // printf("switch_to,pid=");
    // PRINT_HEX(pid);
    // println();
    // printf("this task is running:");
    // PRINT_HEX(task_manager.pid_running);
    // println();
    // printf("switch_to,pid=");
    // PRINT_HEX(pid);
    // println();
    // printf("jump to address:");
    // PRINT_HEX(task_manager.tasks[pid].tss.eip);
    // println();
    if (task_manager.pid_running != pid)
    {
        if (task_manager.pid_running != NOT_FOUND)
        {
            if (reason == REASON_TIME_PIECE_OVER)
                set_task_state(task_manager.pid_running, STATE_READY); // 无特殊需求,进程状态应当切换为就绪态
            else if (reason == REASON_TASK_SLEEP)
                set_task_state(task_manager.pid_running, STATE_SLEEPING); // 当前正在运行的进程应当不从属于任何一个队列,无需"批判旧事物"
            else if (reason == REASON_TASK_BLOCKED)
                set_task_state(task_manager.pid_running, STATE_BLOCKED);
        }
        set_task_state(pid, STATE_RUNNING);
    }
}

void task_manager_init(task_manager_t *p_task_manager)
{
    pid_queue_init(&p_task_manager->pid_queue_ready);
    pid_queue_init(&p_task_manager->pid_queue_blocked);
    p_task_manager->pid_running = NOT_FOUND;
    p_task_manager->schdule_counter = TIME_PIECE;
    // p_task_manager->tasks当中的元素会在每一次task_init时初始化,不需要现在专门去初始化
}

// 进程睡眠的系统调用(单位:毫秒)
void sys_sleep(uint32_t counter)
{
    task_manager.tasks[task_manager.pid_running].wakeup_counter = counter;
    schedule(REASON_TASK_SLEEP);
}

void sys_exit(uint32_t code)
{
    // 进程结束的系统调用极其简单
    memset(gdt + task_manager.pid_running, 0, sizeof(gdt_desc));
    // 正在运行的进程不属于任何一个队列,无需修改相应的队列
    schedule(REASON_TASK_EXIT);
}

void check_wakeup()
{
    for (uint16_t i = task_manager.pid_queue_sleeping.head; i != task_manager.pid_queue_sleeping.tail; INC_PID_QUEUE_INDEX(i))
    {
        uint16_t pid = task_manager.pid_queue_sleeping.pids[i];
        if (--task_manager.tasks[pid].wakeup_counter <= 0)
        {
            // printf("%WAKE UP NOW!\n");
            pid_queue_delete(&task_manager.pid_queue_sleeping, i);
            // 唤醒
            set_task_state(pid, STATE_READY);
            i--; // 由于顺序表的特性,删除之后要记得处理一下下标!
        }
    }
}