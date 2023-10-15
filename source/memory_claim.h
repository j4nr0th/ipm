//
// Created by jan on 12.10.2023.
//

#ifndef IPM_MEMORY_CLAIM_H
#define IPM_MEMORY_CLAIM_H
#include "../include/ipm/ipm_common.h"
#include "ipm_platform.h"


struct ipm_memory_claim_T
{
    ipm_id claim_id;        //  ID of the claim made
    ipm_id proc_id;         //  ID of the process claiming it
    ipm_access_mode access; //  Access type
    size_t offset;          //  Offset of region
    size_t size;            //  Size of region
};
typedef struct ipm_memory_claim_T ipm_memory_claim;

struct ipm_claim_list_T
{
//    ipm_sem free_sem;           //  Semaphore which counts the number of free entries
    ipm_mut list_mutex;         //  Mutex for the buffer
    ipm_cnd list_cnd;           //  Conditional variable used to indicate the state was updated
    size_t count;               //  Number of claims (redundant)
    size_t capacity;            //  Capacity of claims (redundant)
    size_t claim_counter;       //  Counts the number of claims made
    ipm_memory_claim claims[];  //  The claims themselves
};
typedef struct ipm_claim_list_T ipm_claim_list;

ipm_bool claim_encompasses_other(const ipm_memory_claim* claim_1, const ipm_memory_claim* claim_2);

ipm_bool claims_conflict(const ipm_memory_claim* claim_1, const ipm_memory_claim* claim_2);

ipm_result claim_add_to_list(const ipm_memory_claim* claim, ipm_claim_list* list);

ipm_result claim_remove_from_list(ipm_id claim_id, ipm_claim_list* list);

ipm_result claims_iterate(ipm_claim_list* list, int(*callback)(const ipm_memory_claim* claim, void* param), void* param);

ipm_result claim_list_init(ipm_claim_list* list, size_t capacity);

void claim_list_uninit(ipm_claim_list* list);

#endif //IPM_MEMORY_CLAIM_H
