
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dao.h"
#include "trie.h"
#include "db.h"


/*================兼容性选项==================*/
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

typedef SRWLOCK rwlock_t;

/// @brief 初始化读写锁
/// @param lock 
static inline void rwlock_init(rwlock_t *lock){
    InitializeSRWLock(lock);
}
/// @brief 读锁
/// @param lock
static inline void rwlock_rdlock(rwlock_t *lock){
    AcquireSRWLockShared(lock);
}

/// @brief 写锁
/// @param lock
static inline void rwlock_wrlock(rwlock_t *lock){
    AcquireSRWLockExclusive(lock);
}

/// @brief 解读锁
/// @param lock
static inline void rwlock_unlock(rwlock_t *lock){
    ReleaseSRWLockShared(lock);
}

/// @brief 解写锁
/// @param lock
static inline void rwlock_wr_unlock(rwlock_t *lock){
    ReleaseSRWLockExclusive(lock);
}

/// @brief 销毁读写锁
/// @param lock
static inline void rwlock_destroy(rwlock_t *lock){
    // do nothing
}
static inline void r
#elif defined(__linux__)
#include <pthread.h>
#include <sys/prctl.h>

typedef pthread_rwlock_t rwlock_t;

/// @brief 初始化读写锁
/// @param lock 
static inline void rwlock_init(rwlock_t *lock){
    pthread_rwlock_init(lock,NULL);
}

/// @brief 读锁
/// @param lock
static inline void rwlock_rdlock(rwlock_t *lock){
    pthread_rwlock_rdlock(lock);
}

/// @brief 写锁
/// @param lock
static inline void rwlock_wrlock(rwlock_t *lock){
    pthread_rwlock_wrlock(lock);
}

/// @brief 解读锁
/// @param lock
static inline void rwlock_unlock(rwlock_t *lock){
    pthread_rwlock_unlock(lock);
}

/// @brief 解写锁
/// @param lock
static inline void rwlock_wr_unlock(rwlock_t *lock){
    pthread_rwlock_unlock(lock);
}

/// @brief 销毁读写锁
/// @param lock
static inline void rwlock_destroy(rwlock_t *lock){
    pthread_rwlock_destroy(lock);
}
#endif

/*==============静态函数原型====================*/
static int add_record_nolock(DNSRecord *record);
static int query_record_nolock(const char *domain, uint16 record_type, DNSRecord *record);

/*===============全局变量=========================*/
static rwlock_t rwlock;
static trie_node *cache_root;


/*=================正常函数====================*/

void init_dao(){
    rwlock_init(&rwlock);
    cache_root = trie_create();
    init_db();
    LOG_INFO("DAO init success");
}

int add_record(DNSRecord *record){
    rwlock_wrlock(&rwlock);
    int ret = add_record_nolock(record);
    rwlock_wr_unlock(&rwlock);
    return ret;
}

int query_record(const char *domain, uint16 record_type, DNSRecord *record){
    rwlock_rdlock(&rwlock);
    int ret = query_record_nolock(domain, record_type, record);
    rwlock_unlock(&rwlock);
    return ret;
}


static int add_record_nolock(DNSRecord *record){
    // 先插入缓存
    

}

static int query_record_nolock(const char *domain, uint16 record_type, DNSRecord *record){
    // 先查缓存
    struct record_info *info = trie_search(cache_root, domain, record_type);
    // 缓存没有再查数据库
    if(info == NULL){
        struct record_dto = query_by_domin_name(domain, record_type);
        if(record_dto == NULL){
            return -1;
        }
        // 插入缓存
        info = (struct record_info *)malloc(sizeof(struct record_info));
        info->record_type = record_type;
        info->expire_time = record_dto->expire_time;
        memcpy(info->record_data, record_dto->record_data, record_dto->record_len);
        info->domain_name = (char*)malloc(256);
        memcpy(info->domain_name, domain, 256);
        int ret = trie_insert(cache_root, domain, record_type, info);
        if(ret != 0){
            LOG_ERROR("insert cache failed");
            return -1;
        }
    }
}