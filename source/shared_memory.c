//
// Created by jan on 9.10.2023.
//

#include "shared_memory.h"

#ifdef IPM_PLATFORM_POSIX

static inline void make_block_name_based_on_id(char* buffer, size_t buffer_size, const char* block_name, ipm_id id)
{
    (void) snprintf(buffer, buffer_size, "/%.*s-%#016lX", IPM_MAX_NAME_LEN, block_name, id);
}

ipm_result shared_memory_block_create(
        const ipm_context* context, const char* block_name, ipm_id id, size_t size, ipm_access_mode access,
        ipm_shared_memory_block* p_block)
{
    const size_t name_len = strlen(block_name);
    assert((size & (IPM_MEMORY_PAGE_SIZE_MASK)) == 0);
    assert(size > 0);
    assert(name_len <= IPM_MAX_NAME_LEN);
    char name_buffer[IPM_MAX_NAME_LEN + 32];
    make_block_name_based_on_id(name_buffer, sizeof(name_buffer), block_name, id);
    const int fd = shm_open(name_buffer, O_RDWR | O_CREAT | O_EXCL,  //  Need write permission to set size with ftruncate
                            S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if (fd < 0)
    {
        if (errno == EEXIST)
        {
            return IPM_RESULT_ERR_EXISTS;
        }
        IPM_ERROR(context, "Could not open block with id %#016lX, reason: %s", id, strerror(errno));
        switch (errno)
        {
        case EACCES:
            return IPM_RESULT_ERR_ACCESS;
        case EINVAL:
            return IPM_RESULT_ERR_BAD_VALUE;
        case EMFILE:
            return IPM_RESULT_ERR_MAX_FDS;
        case ENFILE:
            return IPM_RESULT_ERR_MAX_FDS_SYS;
        case ENAMETOOLONG:
            return IPM_RESULT_ERR_NAME_TOO_LONG;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }

    const int truc_res = ftruncate(fd, (off_t) (size + IPM_MEMORY_PAGE_SIZE));
    if (truc_res < 0)
    {
        IPM_ERROR(context, "Could not truncate shared memory's FD to %zu bytes, reason: %s", (size + IPM_MEMORY_PAGE_SIZE), strerror(errno));
        close(fd);
        (void)shm_unlink(name_buffer);
        switch (errno)
        {
        case EACCES:
            return IPM_RESULT_ERR_ACCESS;
        case EBADF:
        case EINVAL:
            return IPM_RESULT_ERR_INVALID_FD;
        case EPERM:
        case EROFS:
            return IPM_RESULT_ERR_BAD_FS;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }
    void* const block_memory = mmap(NULL, (size), (access == IPM_ACCESS_MODE_READ_ONLY ? PROT_READ : PROT_READ|PROT_WRITE), MAP_SHARED, fd, IPM_MEMORY_PAGE_SIZE);
    if (block_memory == MAP_FAILED)
    {
        close(fd);
        IPM_ERROR(context, "Could not map shared memory to memory, reason: %s", strerror(errno));
        shm_unlink(name_buffer);
        switch (errno)
        {
        case EACCES:
            return IPM_RESULT_ERR_ACCESS;
        case EAGAIN:
        case ENOMEM:
            return IPM_RESULT_ERR_OS_OUT_OF_MEMORY;
        case EINVAL:
            return IPM_RESULT_ERR_BAD_VALUE;
        case ENFILE:
            return IPM_RESULT_ERR_MAX_FDS_SYS;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }
    ipm_shared_memory_header* const header = mmap(NULL, IPM_MEMORY_PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (!header)
    {
        close(fd);
        IPM_ERROR(context, "Could not map shared memory to memory, reason: %s", strerror(errno));
        (void)munmap(block_memory, size);
        shm_unlink(name_buffer);
        switch (errno)
        {
        case EACCES:
            return IPM_RESULT_ERR_ACCESS;
        case EAGAIN:
        case ENOMEM:
            return IPM_RESULT_ERR_OS_OUT_OF_MEMORY;
        case EINVAL:
            return IPM_RESULT_ERR_BAD_VALUE;
        case ENFILE:
            return IPM_RESULT_ERR_MAX_FDS_SYS;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }

    header->block_id = id;
    header->block_size = size;
    header->id_counter = 1;

    const ipm_result res = ipm_mutex_init(&header->segment_mutex);
    if (res != IPM_RESULT_SUCCESS)
    {
        close(fd);
        IPM_ERROR(context, "Could not create block mutex, reason: %s", strerror(errno));
        munmap(header, IPM_MEMORY_PAGE_SIZE);
        munmap(block_memory, size);
        shm_unlink(name_buffer);
        return res;
    }

    memcpy(header->block_name, block_name, name_len);
    header->block_name[name_len] = 0;

    p_block->mem_fd = fd;
    p_block->header = header;
    p_block->memory = block_memory;
    p_block->access_id = atomic_fetch_add(&header->id_counter, 1);
    p_block->access_mode = access;
    p_block->size = size;
    p_block->has_ownership = 0;

    header->refcount = 1;
    return IPM_RESULT_SUCCESS;
}

ipm_result shared_memory_block_open(
        const ipm_context* context, const char* block_name, ipm_id id, ipm_access_mode access, ipm_shared_memory_block* p_block)
{
    const size_t name_len = strlen(block_name);
    assert(name_len <= IPM_MAX_NAME_LEN);
    char name_buffer[IPM_MAX_NAME_LEN + 32];
    make_block_name_based_on_id(name_buffer, sizeof(name_buffer), block_name, id);
    const int fd = shm_open(name_buffer, O_RDWR,  //  Need write permission to set size with ftruncate
                            S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if (fd < 0)
    {
        if (errno == EEXIST)
        {
            return IPM_RESULT_ERR_EXISTS;
        }
        IPM_ERROR(context, "Could not open block with id %#016lX, reason: %s", id, strerror(errno));
        switch (errno)
        {
        case EACCES:
            return IPM_RESULT_ERR_ACCESS;
        case EINVAL:
            return IPM_RESULT_ERR_BAD_VALUE;
        case EMFILE:
            return IPM_RESULT_ERR_MAX_FDS;
        case ENFILE:
            return IPM_RESULT_ERR_MAX_FDS_SYS;
        case ENAMETOOLONG:
            return IPM_RESULT_ERR_NAME_TOO_LONG;
        case ENOENT:
            return IPM_RESULT_ERR_DOES_NOT_EXIST;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }
    ipm_shared_memory_header* const header = mmap(NULL, IPM_MEMORY_PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (!header)
    {
        IPM_ERROR(context, "Could not map shared memory to memory, reason: %s", strerror(errno));
        switch (errno)
        {
        case EACCES:
            return IPM_RESULT_ERR_ACCESS;
        case EAGAIN:
        case ENOMEM:
            return IPM_RESULT_ERR_OS_OUT_OF_MEMORY;
        case EINVAL:
            return IPM_RESULT_ERR_BAD_VALUE;
        case ENFILE:
            return IPM_RESULT_ERR_MAX_FDS_SYS;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }

    const size_t size = header->block_size;
    void* const block_memory = mmap(NULL, size, (access == IPM_ACCESS_MODE_READ_ONLY ? PROT_READ : PROT_READ | PROT_WRITE), MAP_SHARED, fd, IPM_MEMORY_PAGE_SIZE);
    if (block_memory == MAP_FAILED)
    {
        close(fd);
        (void)munmap(header, IPM_MEMORY_PAGE_SIZE);
        IPM_ERROR(context, "Could not map shared memory to memory, reason: %s", strerror(errno));
        switch (errno)
        {
        case EACCES:
            return IPM_RESULT_ERR_ACCESS;
        case EAGAIN:
        case ENOMEM:
            return IPM_RESULT_ERR_OS_OUT_OF_MEMORY;
        case EINVAL:
            return IPM_RESULT_ERR_BAD_VALUE;
        case ENFILE:
            return IPM_RESULT_ERR_MAX_FDS_SYS;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }

    while (header->refcount == 0)
    {
        //  Wait for the refcount to increase (set by creator thread when it is done initializing
        sched_yield();  //  If the condition is not true, yield
    }

    if (header->block_id != id)
    {
        close(fd);
        IPM_ERROR(context, "Block id (%#016lX) did not match the specified id (%#016lX)", header->block_id, id);
        (void)munmap(header, IPM_MEMORY_PAGE_SIZE);
        (void)munmap(block_memory, size);
        return IPM_RESULT_ERR_BAD_ID;
    }
    if (memcmp(header->block_name, block_name, name_len) != 0)
    {
        close(fd);
        IPM_ERROR(context, "Block id (%s) did not match the specified id (%.*s)", block_name, IPM_MAX_NAME_LEN, header->block_name);
        (void)munmap(header, IPM_MEMORY_PAGE_SIZE);
        (void)munmap(block_memory, size);
        return IPM_RESULT_ERR_BAD_ID;
    }

    (void)atomic_fetch_add(&header->refcount, 1);

    p_block->header = header;
    p_block->memory = block_memory;
    p_block->access_id = atomic_fetch_add(&header->id_counter, 1);
    p_block->access_mode = access;
    p_block->mem_fd = fd;
    p_block->size = size;
    p_block->has_ownership = 0;

    return IPM_RESULT_SUCCESS;
}

ipm_result shared_memory_block_close(
        const ipm_context* context, ipm_shared_memory_block* block,
        void (* callback)(void* param), void* param)
{
    assert(block->has_ownership == 0);
    ipm_shared_memory_header* header = block->header;
    void* const mem = block->memory;
    close(block->mem_fd);

    memset(block, 0xCC, sizeof(*block));

    const uint32_t refs = atomic_fetch_sub(&header->refcount, 1);
    if (refs == 1)
    {
        if (callback)
        {
            callback(param);
        }
        ipm_mutex_destroy(&header->segment_mutex);
        char name_buffer[IPM_MAX_NAME_LEN + 32];
        const ipm_id id = header->block_id;
        make_block_name_based_on_id(name_buffer, sizeof(name_buffer), header->block_name, id);
        (void)munmap(mem, header->block_size);
        (void) munmap(header, IPM_MEMORY_PAGE_SIZE);
        header = NULL;
        //  This was the last block (meaning, UNLINK THIS)
        const int res = shm_unlink(name_buffer);
        if (res < 0)
        {
            IPM_ERROR(context, "Could not unlink memory block %s-%#016lX, reason: %s", name_buffer, id, strerror(errno));
            switch (errno)
            {
            case EACCES:
                return IPM_RESULT_ERR_ACCESS;
            default:
                return IPM_RESULT_ERR_OS_UNEXPECTED;
            }
        }
    }
    else
    {
        (void)munmap(mem, header->block_size);
        (void) munmap(header, IPM_MEMORY_PAGE_SIZE);
        header = NULL;
    }

    return IPM_RESULT_SUCCESS;
}

ipm_result acquire_memory_block_whole(const ipm_context* context, ipm_shared_memory_block* block)
{
    if (block->has_ownership)
    {
        IPM_ERROR(context, "Already has block ownership");
        return IPM_RESULT_ERR_ALREADY_LOCKED;
    }
    const ipm_result wait_res = ipm_mutex_lock(&block->header->segment_mutex);
    if (wait_res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(context, "Could not wait on block mutex, reason: %s", strerror(errno));
        return wait_res;
    }
    block->has_ownership = 1;
    return IPM_RESULT_SUCCESS;
}

ipm_result release_memory_block_whole(const ipm_context* context, ipm_shared_memory_block* block)
{
    if (block->has_ownership)
    {
        IPM_ERROR(context, "Does not have ownership of the block!");
        return IPM_RESULT_ERR_NOT_LOCKED;
    }
    ipm_mutex_unlock(&block->header->segment_mutex);
    block->has_ownership = 0;
    return IPM_RESULT_SUCCESS;
}

ipm_result shared_memory_block_update_mapping(
        const ipm_context* context, ipm_shared_memory_block* block, ipm_access_mode access_mode)
{
    const size_t new_size = block->header->block_size;
    if (block->size == new_size && access_mode == block->access_mode)
    {
        //  Block size has not changed
        return IPM_RESULT_SUCCESS;
    }

    void* const old_ptr = block->memory;
    munmap(block->memory, block->size);
    block->memory = NULL;
    void* const new_ptr = mmap(old_ptr, new_size, (access_mode == IPM_ACCESS_MODE_READ_ONLY ? PROT_READ : PROT_READ|PROT_WRITE), MAP_SHARED, block->mem_fd, IPM_MEMORY_PAGE_SIZE);
    if (new_ptr == MAP_FAILED)
    {
        IPM_ERROR(context, "Could not map the updated shared memory block, reason: %s", strerror(errno));
        block->size = 0;
        block->access_mode = 0;
        return IPM_RESULT_ERR_OS_OUT_OF_MEMORY;
    }
    block->access_mode = access_mode;
    block->size = new_size;
    block->memory = NULL;

    return IPM_RESULT_SUCCESS;
}

ipm_result shared_memory_block_resize(const ipm_context* context, ipm_shared_memory_block* block, size_t new_size)
{
    //  NO SHRINKING!!!
    assert(new_size >= block->header->block_size);
    assert((new_size & IPM_MEMORY_PAGE_SIZE_MASK) == 0);
    assert(new_size > 0);
    //  Lock access to file
    const ipm_result sem_res = ipm_mutex_lock(&block->header->segment_mutex);
    if (sem_res != IPM_RESULT_SUCCESS)
    {
        //  Could not lock the semaphore
        IPM_ERROR(context, "Could not wait for semaphore to lock the memory segment, reason: %s", strerror(errno));
        return sem_res;
    }

    if (new_size == block->header->block_size)
    {
        //  Block was already truncated to the correct size
        ipm_mutex_unlock(&block->header->segment_mutex);
        if (new_size != block->size)
        {
            return shared_memory_block_update_mapping(context, block, block->access_mode);
        }
        return IPM_RESULT_SUCCESS;
    }

    const int res = ftruncate(block->mem_fd, (off_t) (IPM_MEMORY_PAGE_SIZE + new_size));
    if (res < 0)
    {
        //  Failed truncation
        ipm_mutex_unlock(&block->header->segment_mutex);
        IPM_ERROR(context, "Could not truncate file to %zu bytes, reason: %s", IPM_MEMORY_PAGE_SIZE + new_size,
                  strerror(errno));
        switch (errno)
        {
        case EACCES:
            return IPM_RESULT_ERR_ACCESS;
        case EFBIG:
        case EINVAL:
            return IPM_RESULT_ERR_OS_OUT_OF_MEMORY;
        case EPERM:
        case EROFS:
            return IPM_RESULT_ERR_BAD_FS;
        case EBADF:
            return IPM_RESULT_ERR_BAD_VALUE;
        default:
            return IPM_RESULT_ERR_OS_UNEXPECTED;
        }
    }
    //  Update internal size
    block->header->block_size = new_size;
    ipm_mutex_unlock(&block->header->segment_mutex);

    return shared_memory_block_update_mapping(context, block, block->access_mode);
}

ipm_result shared_memory_block_clean(ipm_shared_memory_block* block)
{
    assert(block->has_ownership == 0);
    ipm_shared_memory_header* header = block->header;
    void* const mem = block->memory;
    close(block->mem_fd);

    memset(block, 0xCC, sizeof(*block));

    (void)munmap(mem, header->block_size);
    (void)munmap(header, IPM_MEMORY_PAGE_SIZE);
    header = NULL;

    return IPM_RESULT_SUCCESS;
}

#endif
