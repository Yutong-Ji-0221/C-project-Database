#include <semaphore.h>

void init_lock(sem_t *lock)
{
    sem_init(lock, 0, 1);
}

void take_lock(sem_t *lock)
{
    sem_wait(lock);
}

void release_lock(sem_t *lock)
{
    sem_post(lock);
}
