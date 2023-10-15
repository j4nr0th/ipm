//
// Created by jan on 9.10.2023.
//

#ifndef IPM_IPM_PLATFORM_H
#define IPM_IPM_PLATFORM_H

#if defined(__unix__) || (defined(__APPLE__) || defined(__MACH__))
    #define IPM_PLATFORM_POSIX
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include "../include/ipm/ipm_error.h"
#include "internal.h"

typedef sem_t ipm_sem;
typedef pthread_mutex_t ipm_mut;
typedef pthread_cond_t ipm_cnd;

#endif

#if  defined(_WIN32) || defined(__CYGWIN__)
    #define IPM_PLATFORM_WINDOWS
#endif

IPM_INTERNAL_FUNCTION
ipm_result ipm_semaphore_init(ipm_sem* p_sem, unsigned val);

IPM_INTERNAL_FUNCTION
void ipm_semaphore_destroy(ipm_sem* p_sem);

IPM_INTERNAL_FUNCTION
void ipm_semaphore_post(ipm_sem* p_sem);

IPM_INTERNAL_FUNCTION
ipm_result ipm_semaphore_wait(ipm_sem* p_sem);

IPM_INTERNAL_FUNCTION
unsigned ipm_semaphore_count(ipm_sem* p_sem);

IPM_INTERNAL_FUNCTION
ipm_result ipm_mutex_init(ipm_mut* p_mtx);

IPM_INTERNAL_FUNCTION
ipm_result ipm_mutex_lock(ipm_mut* mtx);

IPM_INTERNAL_FUNCTION
ipm_result ipm_mutex_unlock(ipm_mut* mtx);

IPM_INTERNAL_FUNCTION
ipm_result ipm_mutex_destroy(ipm_mut* p_mtx);

IPM_INTERNAL_FUNCTION
ipm_result ipm_condition_init(ipm_cnd* p_cnd);

IPM_INTERNAL_FUNCTION
ipm_result ipm_condition_wait(ipm_cnd* cnd, ipm_mut* mtx);

IPM_INTERNAL_FUNCTION
ipm_result ipm_condition_broadcast(ipm_cnd* cnd);

IPM_INTERNAL_FUNCTION
ipm_result ipm_condition_signal(ipm_cnd* cnd);

IPM_INTERNAL_FUNCTION
ipm_result ipm_condition_destroy(ipm_cnd* p_cnd);


#endif //IPM_IPM_PLATFORM_H
