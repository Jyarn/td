#include "../threadpool.h"
#include "../lock.h"

#include <stdio.h>

Lock lock;

void
_lock1(void* _)
{
    Thread* self = get_tcb();
    for (int i = 0; i < COUNTER_MAX; i++)
    {
        thpl_lock_enter(&lock, self);
        counter += 1;
        thpl_lock_exit(&lock);
    }
}

int
lock1(void)
{
    counter = 0;
    thpl_lock_init(&lock);
    for (int i = 0; i < NWORKERS; i++)
    {
        thpl_push(_lock1, 0);
    }

    thpl_yield();
    printf("%d\n", counter);
    return COUNTER_MAX * NWORKERS != counter;
}
