#include "semaphore.h"
#include "task.h"
#include "sched.h"
#include "console.h"

extern task_manager_t task_manager;

void wait(semaphore_t *semaphore)
{
    // 教科书级别的写法
    semaphore->count--;
    if (semaphore->count < 0)
    {
        pid_enqueue(&semaphore->blocked_pids, task_manager.pid_running);
        schedule(REASON_TASK_BLOCKED);
    }
}

void signal(semaphore_t *semaphore)
{
    // printf("%signal!\n");
    semaphore->count++;
    uint16_t pid_wake_up = pid_dequeue(&semaphore->blocked_pids);
    if (pid_wake_up != NOT_FOUND)
    {
        // printf("%pid_wake_up=");
        // PRINT_HEX(pid_wake_up);
        // println();
        // pid_wake_up已经离开了信号量的阻塞队列,接下来还需要离开系统阻塞队列
        uint16_t i = pid_queue_find(&task_manager.pid_queue_blocked, pid_wake_up);
        pid_queue_delete(&task_manager.pid_queue_blocked, i);
#ifdef _DEBUG
        if (i == NOT_FOUND)
            while (true)
                printf("%Data in task_manager block queue and the semaphore block queue do not correspong,error!\n");
#endif
        set_task_state(pid_wake_up, STATE_READY);
    }
}

void semaphore_init(semaphore_t *semaphore, int16_t count)
{
    semaphore->count = count;
    pid_queue_init(&(semaphore->blocked_pids));
}