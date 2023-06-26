#include "dao.h"



/*================兼容性选项==================*/
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

typedef SRWLOCK rwlock_t;
static inline void rwlock_init(rwlock_t *lock){
    InitializeSRWLock(lock);
}
static inline void rwlock_rdlock(rwlock_t *lock){
    AcquireSRWLockShared(lock);
}
static inline void rwlock_wrlock(rwlock_t *lock){
    AcquireSRWLockExclusive(lock);
}
static inline void rwlock_unlock(rwlock_t *lock){
    ReleaseSRWLockShared(lock);
}
static inline void rwlock_wrunlock(rwlock_t *lock){
    ReleaseSRWLockExclusive(lock);
}
static inline void rwlock_destroy(rwlock_t *lock){
    // do nothing
}
static inline void r
#elif defined(__linux__)
#include <pthread.h>
#include <sys/prctl.h>

typedef pthread_rwlock_t rwlock_t;

static inline void rwlock_init(rwlock_t *lock){
    pthread_rwlock_init(lock,NULL);
}

static inline void rwlock_rdlock(rwlock_t *lock){
    pthread_rwlock_rdlock(lock);
}

static inline void rwlock_wrlock(rwlock_t *lock){
    pthread_rwlock_wrlock(lock);
}

static inline void rwlock_unlock(rwlock_t *lock){
    pthread_rwlock_unlock(lock);
}

static inline void rwlock_wrunlock(rwlock_t *lock){
    pthread_rwlock_unlock(lock);
}

static inline void rwlock_destroy(rwlock_t *lock){
    pthread_rwlock_destroy(lock);
}
#endif