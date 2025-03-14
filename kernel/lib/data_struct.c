// 涉及数据结构的部分全部放到这个文件当中

// 对于队列这种极其简单的数据结构,我都搞得不清不楚的,足以见得我并没有把数据结构学好
// 我的408考不到110是纯粹的咎由自取!
#include "stdint.h"
#include "gdt.h"
#include "data_struct.h"
#include "console.h"

// 队列的基本操作
void pid_queue_init(pid_queue_t *p_pid_queue)
{
    for (uint16_t i = 0; i < MAX_GDT_TERMS; i++)
        p_pid_queue->pids[i] = NOT_FOUND;
    p_pid_queue->head = p_pid_queue->tail = 0;
}

void pid_enqueue(pid_queue_t *p_pid_queue, uint16_t pid)
{
    p_pid_queue->pids[p_pid_queue->tail] = pid;
    INC_PID_QUEUE_INDEX(p_pid_queue->tail);
}

uint16_t pid_dequeue(pid_queue_t *p_pid_queue)
{
    if (pid_queue_length(p_pid_queue) == 0)
        return NOT_FOUND;
    uint16_t temp = p_pid_queue->pids[p_pid_queue->head];
    INC_PID_QUEUE_INDEX(p_pid_queue->head);
    return temp;
}

// 返回找到的下标
uint16_t pid_queue_find(pid_queue_t *p_pid_queue, uint16_t pid)
{
    for (uint16_t i = p_pid_queue->head; i != p_pid_queue->tail; INC_PID_QUEUE_INDEX(i))
    {
        if (p_pid_queue->pids[i] == pid)
            return pid;
    }
    return NOT_FOUND;
}

void pid_queue_delete(pid_queue_t *p_pid_queue, uint16_t i)
{
    // 在i之后的所有元素前移!
    for (uint16_t j = i; j != p_pid_queue->tail; INC_PID_QUEUE_INDEX(j))
    {
        p_pid_queue->pids[j] = p_pid_queue->pids[(j + 1) % MAX_GDT_TERMS];
    }
    p_pid_queue->tail--;
}