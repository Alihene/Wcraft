// Implementation from https://nachtimwald.com/2019/04/12/thread-pool-in-c/
#include "thread_pool.h"

Task *create_task(task_func func, void *arg) {
    Task *task;

    if(!func) {
        return NULL;
    }

    task = malloc(sizeof(Task));
    task->func = func;
    task->arg = arg;
    task->next = NULL;
    return task;
}

void destroy_task(Task *task) {
    if(task) {
        free(task);
    }
}

static Task *next_task(ThreadPool *pool) {
    Task *task;

    if(!pool) {
        return NULL;
    }

    task = pool->last_task;
    if(!task) {
        return NULL;
    }

    if(task->next == NULL) {
        pool->last_task = NULL;
    } else {
        pool->last_task = task->next;
    }

    return task;
}

static void *thread_func(void *arg) {
    ThreadPool *pool = arg;
    Task *task;

    while(1) {
        pthread_mutex_lock(&pool->mutex);

        while(!pool->last_task && pool->active) {
            pthread_cond_wait(&pool->task_cond, &pool->mutex);
        }

        if(!pool->active) {
            break;
        }

        task = next_task(pool);
        pool->working_threads++;
        pthread_mutex_unlock(&pool->mutex);

        if(task) {
            task->func(task->arg);
            destroy_task(task);
        }

        pthread_mutex_lock(&pool->mutex);
        if(pool->active && pool->working_threads == 0 && !pool->last_task) {
            pthread_cond_signal(&pool->working_cond);
        }
        pthread_mutex_unlock(&pool->mutex);
    }

    pool->num_threads--;
    pthread_cond_signal(&pool->working_cond);
    pthread_mutex_unlock(&pool->mutex);
    return NULL;
}

void init_thread_pool(ThreadPool *thread_pool, u32 num_threads) {
    if(num_threads < 1) {
        return;
    }

    thread_pool->num_threads = num_threads;
    thread_pool->active = true;

    pthread_mutex_init(&thread_pool->mutex, NULL);
    pthread_cond_init(&thread_pool->task_cond, NULL);
    pthread_cond_init(&thread_pool->working_cond, NULL);

    thread_pool->last_task = NULL;
    thread_pool->first_task = NULL;

    thread_pool->threads = malloc(num_threads * sizeof(pthread_t));
    for(u32 i = 0; i < num_threads; i++) {
        if(pthread_create(&thread_pool->threads[i], NULL, thread_func, thread_pool) != 0) {
            fprintf(stderr, "Failed to create thread %u in thread pool\n", i);
            return;
        }
        pthread_detach(thread_pool->threads[i]);
    }
}

static void pool_wait(ThreadPool *pool) {
    if(!pool) {
        return;
    }

    pthread_mutex_lock(&pool->mutex);
    while(1) {
        if(!pool->last_task || (pool->active && pool->working_threads != 0) || (!pool->active && pool->num_threads != 0)) {
            pthread_cond_wait(&pool->working_cond, &pool->mutex);
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&pool->mutex);
}

void destroy_thread_pool(ThreadPool *pool) {
    Task *task1, *task2;

    if(!pool) {
        return;
    }

    pthread_mutex_lock(&pool->mutex);
    task1 = pool->last_task;
    while(task1) {
        task2 = task1->next;
        destroy_task(task1);
        task1 = task2;
    }
    pool->last_task = NULL;
    pool->active = false;

    pthread_cond_broadcast(&pool->task_cond);
    pthread_mutex_unlock(&pool->mutex);

    pool_wait(pool);

    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->task_cond);
    pthread_cond_destroy(&pool->working_cond);
}

bool push_task(ThreadPool *pool, task_func func, void *arg) {
    Task *task;

    if(!pool) {
        return false;
    }

    task = create_task(func, arg);
    if(!task) {
        return false;
    }

    pthread_mutex_lock(&pool->mutex);
    if(pool->first_task) {
        pool->first_task->next = task;
        pool->first_task = task;
    } else {
        pool->first_task = task;
        pool->last_task = task;
    }

    pthread_cond_broadcast(&pool->task_cond);
    pthread_mutex_unlock(&pool->mutex);

    return true;
}