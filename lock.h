#include <semaphore.h>

void init_lock(sem_t *lock);
void take_lock(sem_t *lock);
void release_lock(sem_t *lock);
