#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

#include "gdt.h"

#define INC_PID_QUEUE_INDEX(i) (i = (i + 1) % MAX_GDT_TERMS)
#define DEC_PID_QUEUE_INDEX(i) (i = (i - 1) % MAX_GDT_TERMS)

// 我通过静态链表的方式制作一个队列...
typedef struct pid_queue_t
{
    uint16_t head, tail; // tail指示下一个应当加入的位置
    uint16_t pids[MAX_GDT_TERMS];
} pid_queue_t;

static inline uint16_t pid_queue_length(pid_queue_t *p_pid_queue)
{
    return (p_pid_queue->tail - p_pid_queue->head) % MAX_GDT_TERMS;
}

void pid_queue_init(pid_queue_t *p_pid_queue);
void pid_enqueue(pid_queue_t *p_pid_queue, uint16_t pid);
uint16_t pid_dequeue(pid_queue_t *p_pid_queue);
uint16_t pid_queue_find(pid_queue_t *p_pid_queue, uint16_t pid); // 返回需要查找的pid的对应的下标(此函数极其耦合!)
void pid_queue_delete(pid_queue_t *p_pid_queue, uint16_t i);

#endif