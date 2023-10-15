//
// Created by jan on 14.10.2023.
//

#ifndef IPM_MEMORY_INTERNAL_H
#define IPM_MEMORY_INTERNAL_H
#include "../include/ipm/ipm_memory.h"
#include "shared_memory.h"
#include "memory_claim.h"

struct ipm_memory_T
{
    ipm_context ctx;
    char block_name[IPM_MAX_NAME_LEN + 1];
    ipm_shared_memory_block real_memory;
    ipm_shared_memory_block active_claims;
//    ipm_shared_memory_block queued_claims;
};

enum ipm_memory_block_T
{
    IPM_MEMORY_BLOCK_REAL_MEMORY = 1,
    IPM_MEMORY_BLOCK_ACTIVE_CALIMS = 2,
//    IPM_MEMORY_BLOCK_QUEUD_CLAIMS = 3,
};
typedef enum ipm_memory_block_T ipm_memory_block;

ipm_claim_list* internal_ipm_memory_clam_list(ipm_memory* memory);

#endif //IPM_MEMORY_INTERNAL_H
