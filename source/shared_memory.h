//
// Created by jan on 9.10.2023.
//

#ifndef IPM_SHARED_MEMORY_H
#define IPM_SHARED_MEMORY_H
#include "../include/ipm/ipm_common.h"
#include "ipm_platform.h"

struct ipm_shared_memory_header_T
{
    ipm_mut segment_mutex;
    uint32_t refcount;
    size_t block_size;
    ipm_id id_counter;
    ipm_id block_id;
    char block_name[IPM_MAX_NAME_LEN + 1];
};
typedef struct ipm_shared_memory_header_T ipm_shared_memory_header;

struct ipm_shared_memory_block_T
{
    ipm_id access_id;
    size_t size;
    ipm_shared_memory_header* header;
    void* memory;
    ipm_bool has_ownership;
    ipm_access_mode access_mode;
    int mem_fd;
};
typedef struct ipm_shared_memory_block_T ipm_shared_memory_block;

IPM_INTERNAL_FUNCTION
ipm_result shared_memory_block_create(
        const ipm_context* context, const char* block_name, ipm_id id, size_t size, ipm_access_mode access,
        ipm_shared_memory_block* p_block);

IPM_INTERNAL_FUNCTION
ipm_result shared_memory_block_open(
        const ipm_context* context, const char* block_name, ipm_id id, ipm_access_mode access, ipm_shared_memory_block* p_block);

IPM_INTERNAL_FUNCTION
ipm_result shared_memory_block_close(
        const ipm_context* context, ipm_shared_memory_block* block,
        void (* callback)(void* param), void* param);

IPM_INTERNAL_FUNCTION
ipm_result shared_memory_block_clean(ipm_shared_memory_block* block);

IPM_INTERNAL_FUNCTION
ipm_result acquire_memory_block_whole(const ipm_context* context, ipm_shared_memory_block* block);

IPM_INTERNAL_FUNCTION
ipm_result release_memory_block_whole(const ipm_context* context, ipm_shared_memory_block* block);

IPM_INTERNAL_FUNCTION
ipm_result shared_memory_block_update_mapping(
        const ipm_context* context, ipm_shared_memory_block* block, ipm_access_mode access_mode);

IPM_INTERNAL_FUNCTION
ipm_result shared_memory_block_resize(const ipm_context* context, ipm_shared_memory_block* block, size_t new_size);

#endif //IPM_SHARED_MEMORY_H
