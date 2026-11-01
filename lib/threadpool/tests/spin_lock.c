#include <pthread.h>
#include <stdio.h>

#include "../spin.h"

SpinLock spin_lock;

void*
_spin1(void* _)
{
    for (int i = 0; i < COUNTER_MAX; i++)
    {
        thpl_spin_enter(&spin_lock);
        counter += 1;
        thpl_spin_exit(&spin_lock);
    }

    return NULL;
}

int
spin1(void)
{
    pthread_t threads[NWORKERS];
    thpl_spin_init(&spin_lock);
    for (int i = 0; i < NWORKERS; i++)
    {
        pthread_create(&threads[i], NULL, _spin1, NULL);
    }

    for (int i = 0; i < NWORKERS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return COUNTER_MAX * NWORKERS != counter;
}

int
spin2(void)
{
    thpl_spin_init(&spin_lock);
    if (spin_lock != 1) return -1;

    thpl_spin_enter(&spin_lock);
    if (spin_lock != 0) return -1;

    int r = thpl_spin_try(&spin_lock);
    if (r != 0) return -1;

    thpl_spin_exit(&spin_lock);
    if (spin_lock != 1) return -1;

    r = thpl_spin_try(&spin_lock);
    if (r != 1) return -1;

    r = thpl_spin_try(&spin_lock);
    if (r != 0) return -1;

    return 0;
}
