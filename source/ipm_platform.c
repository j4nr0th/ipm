//
// Created by jan on 9.10.2023.
//

#include <errno.h>
#include "ipm_platform.h"

#ifdef IPM_PLATFORM_POSIX


ipm_result ipm_semaphore_init(ipm_sem* p_sem, unsigned val)
{
    const int ret = sem_init(p_sem, 1, val);
    if (ret != 0)
    {
        return IPM_RESULT_ERR_NO_SEM;
    }
    return IPM_RESULT_SUCCESS;
}

void ipm_semaphore_destroy(ipm_sem* p_sem)
{
    sem_destroy(p_sem);
}

void ipm_semaphore_post(ipm_sem* p_sem)
{
    const int ret = sem_post(p_sem);
    (void) ret;
}

ipm_result ipm_semaphore_wait(ipm_sem* p_sem)
{
    const int res = sem_wait(p_sem);
    if (res < 0)
    {
        return IPM_RESULT_ERR_NO_SEM_WAIT;
    }
    return IPM_RESULT_SUCCESS;
}

unsigned ipm_semaphore_count(ipm_sem* p_sem)
{
    int out = 0;
    if (sem_getvalue(p_sem, &out) < 0)
        return -1;
    return out;
}


ipm_result ipm_mutex_init(ipm_mut* p_mtx)
{
    pthread_mutexattr_t attribs;
    int res = pthread_mutexattr_init(&attribs);
    if (res != 0)
    {
        return IPM_RESULT_ERR_OS_UNEXPECTED;
    }
    res = pthread_mutexattr_setpshared(&attribs, PTHREAD_PROCESS_SHARED);
    if (res != 0)
    {
        pthread_mutexattr_destroy(&attribs);
        return IPM_RESULT_ERR_OS_UNEXPECTED;
    }
    res = pthread_mutexattr_settype(&attribs, PTHREAD_MUTEX_ERRORCHECK);
    if (res != 0)
    {
        pthread_mutexattr_destroy(&attribs);
        return IPM_RESULT_ERR_OS_UNEXPECTED;
    }
    res = pthread_mutexattr_setrobust(&attribs, PTHREAD_MUTEX_ROBUST);
    if (res != 0)
    {
        pthread_mutexattr_destroy(&attribs);
        return IPM_RESULT_ERR_OS_UNEXPECTED;
    }
    res = pthread_mutex_init(p_mtx, &attribs);
    if (res != 0)
    {
        pthread_mutexattr_destroy(&attribs);
        switch (errno)
        {
        case EAGAIN:
            //  Lacking resources
        case ENOMEM:
            //  Lacking memory (how!?)
            return IPM_RESULT_ERR_OS_OUT_OF_MEMORY;
        case EPERM:
            //  Lacking permission
            return IPM_RESULT_ERR_ACCESS;
        case EBUSY:
            //  Tried to reinitialized
            return IPM_RESULT_ERR_BAD_INIT;
        case EINVAL:
            //  Bad attributes
            return IPM_RESULT_ERR_BAD_VALUE;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }
    pthread_mutexattr_destroy(&attribs);
    return IPM_RESULT_SUCCESS;
}

ipm_result ipm_mutex_lock(ipm_mut* mtx)
{
    const int res = pthread_mutex_lock(mtx);
    if (res == EOWNERDEAD)
    {
        pthread_mutex_consistent(mtx);
    }
    else if (res != 0)
    {
        switch (errno)
        {
        case EINVAL:
            //  mtx is not an initialized mutex
            return IPM_RESULT_ERR_BAD_VALUE;
        case EDEADLK:
            //  mutex was already locked, would cause a deadlock
            return IPM_RESULT_ERR_DEADLOCK;
        case EOWNERDEAD:
            //  Owner died, this means we're ok, just make the mutex consistent
            pthread_mutex_consistent(mtx);
            break;
        case EPERM:
            //  lack of permissions
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }
    return IPM_RESULT_SUCCESS;
}

ipm_result ipm_mutex_unlock(ipm_mut* mtx)
{
    const int res = pthread_mutex_unlock(mtx);
    if (res != 0)
    {
        switch (errno)
        {
        case EPERM:
            //  lack of permissions
            return IPM_RESULT_ERR_NOT_LOCKED;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }
    return IPM_RESULT_SUCCESS;
}

ipm_result ipm_mutex_destroy(ipm_mut* p_mtx)
{
    const int res = pthread_mutex_destroy(p_mtx);
    if (res != 0)
    {
        switch (errno)
        {
        case EBUSY:
            //  Currently locked by another thread
            return IPM_RESULT_ERR_ALREADY_LOCKED;
        case EINVAL:
            return IPM_RESULT_ERR_BAD_VALUE;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }
    return IPM_RESULT_SUCCESS;
}


ipm_result ipm_condition_init(ipm_cnd* p_cnd)
{
    pthread_condattr_t attribs;
    int res = pthread_condattr_init(&attribs);
    if (res != 0)
    {
        return IPM_RESULT_ERR_OS_UNEXPECTED;
    }
    res = pthread_condattr_setpshared(&attribs, PTHREAD_PROCESS_SHARED);
    if (res != 0)
    {
        (void)pthread_condattr_destroy(&attribs);
        return IPM_RESULT_ERR_OS_UNEXPECTED;
    }
    res = pthread_cond_init(p_cnd, &attribs);
    if (res != 0)
    {
        (void)pthread_condattr_destroy(&attribs);
        switch (errno)
        {
        case EAGAIN:
        case ENOMEM:
            //  Out of resources/memory
            return IPM_RESULT_ERR_OS_OUT_OF_MEMORY;
        case EBUSY:
            //  Already created
            return IPM_RESULT_ERR_BAD_INIT;
        case EINVAL:
            return IPM_RESULT_ERR_BAD_VALUE;
        }
    }

    (void)pthread_condattr_destroy(&attribs);
    return IPM_RESULT_SUCCESS;
}

ipm_result ipm_condition_wait(ipm_cnd* cnd, ipm_mut* mtx)
{
    const int res = pthread_cond_wait(cnd, mtx);
    if (res != 0)
    {
        switch (errno)
        {
        case EINVAL:
            //  Invalid value by either cnd or mtx
            return IPM_RESULT_ERR_BAD_VALUE;
        case EPERM:
            //  Mutex is not locked
            return IPM_RESULT_ERR_NOT_LOCKED;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }
    return IPM_RESULT_SUCCESS;
}

ipm_result ipm_condition_broadcast(ipm_cnd* cnd)
{
    const int res = pthread_cond_broadcast(cnd);
    if (res != 0)
    {
        switch (errno)
        {
        case EINVAL:
            //  Not a condition variable
            return IPM_RESULT_ERR_BAD_VALUE;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }
    return IPM_RESULT_SUCCESS;
}

ipm_result ipm_condition_signal(ipm_cnd* cnd)
{
    const int res = pthread_cond_signal(cnd);
    if (res != 0)
    {
        switch (errno)
        {
        case EINVAL:
            //  Not a condition variable
            return IPM_RESULT_ERR_BAD_VALUE;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }
    return IPM_RESULT_SUCCESS;
}

ipm_result ipm_condition_destroy(ipm_cnd* p_cnd)
{
    const int res = pthread_cond_destroy(p_cnd);
    if (res != 0)
    {
        switch (errno)
        {
        case EBUSY:
            //  Currently locked by another thread
            return IPM_RESULT_ERR_ALREADY_LOCKED;
        case EINVAL:
            return IPM_RESULT_ERR_BAD_VALUE;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }
    return IPM_RESULT_SUCCESS;
}


#endif

