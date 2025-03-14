#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "stdint.h"
#include "data_struct.h"

typedef struct semaphore_t
{
    int16_t count; // 信号量的计数要用有符号数
    pid_queue_t blocked_pids;
} semaphore_t;

void wait(semaphore_t *semaphore);
void signal(semaphore_t *semaphore);
void semaphore_init(semaphore_t *semaphore, int16_t count);

#endif