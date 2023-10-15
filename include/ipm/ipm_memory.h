//
// Created by jan on 14.10.2023.
//

#ifndef IPM_IPM_MEMORY_H
#define IPM_IPM_MEMORY_H
#include "ipm_common.h"
#include "ipm_error.h"

typedef struct ipm_memory_T ipm_memory;

typedef struct ipm_memory_info_T ipm_memory_info;
struct ipm_memory_info_T
{
    ipm_context* p_ctx;                 //  Context of the memory object (can be modified)
    ipm_access_mode memory_access_mode; //  Memory access that is used for the mapping
    ipm_id access_id;                   //  Access id of the block
    const char* name;                   //  Block name
    size_t block_size;                  //  Size of the block
    void* mapping_address;              //  Address of the mapping
    size_t active_claims;               //  Number of active claims
};


ipm_result ipm_memory_create(const ipm_context* context, size_t block_size, const char* block_name, ipm_access_mode access, ipm_memory** p_memory);

ipm_result ipm_memory_open(const ipm_context* context, const char* block_name, ipm_access_mode access, ipm_memory** p_memory);

void ipm_memory_close(ipm_memory* memory);

void ipm_memory_clean(ipm_memory* memory);

ipm_memory_info ipm_memory_get_info(ipm_memory* memory);

ipm_result ipm_memory_sync(ipm_memory* memory);

ipm_result ipm_memory_resize_grow(ipm_memory* memory, size_t new_size);

ipm_result ipm_memory_change_access(ipm_memory* memory, ipm_access_mode access_mode);

unsigned ipm_memory_ref_count(const ipm_memory* memory);

ipm_result ipm_memory_claim_region(ipm_memory* memory, ipm_access_mode access, size_t offset, size_t count, ipm_id* p_claim_id);

ipm_result ipm_memory_release_region(ipm_memory* memory, ipm_id claim_id);

ipm_result ipm_memory_release_all(ipm_memory* memory);

ipm_result ipm_memory_remove_all_active_claims(ipm_memory* memory);

void* ipm_memory_pointer(ipm_memory* memory);


#endif //IPM_IPM_MEMORY_H
