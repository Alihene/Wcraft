// Implementation from https://nachtimwald.com/2019/04/12/thread-pool-in-c/
#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <pthread.h>

#include "util.h"

typedef void (*task_func)(void *arg);

// Next = next to be processed
typedef struct Task {
    struct Task *next;
    task_func func;
    void *arg;
} Task;

typedef struct {
    pthread_t *threads;
    bool active;
    u32 num_threads;
    u32 working_threads;

    Task *first_task;
    Task *last_task;
    pthread_mutex_t mutex;
    pthread_cond_t task_cond;
    pthread_cond_t working_cond;
} ThreadPool;

Task *create_task(task_func func, void *arg);
void destroy_task(Task *task);

void init_thread_pool(ThreadPool *thread_pool, u32 num_threads);
void destroy_thread_pool(ThreadPool *pool);
bool push_task(ThreadPool *pool, task_func func, void *arg);
void thread_pool_wait(ThreadPool *pool);

#endif