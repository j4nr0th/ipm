//
// Created by jan on 14.10.2023.
//

#include "test_common.h"
#include <inttypes.h>
#include <malloc.h>
#include "../source/ipm_memory_internal.h"

void print_claims(const ipm_memory* memory)
{
    ipm_claim_list* const list = memory->active_claims.memory;
    assert(list);
    for (unsigned i = 0; i < list->count; ++i)
    {
        const ipm_memory_claim claim = list->claims[i];
        printf("(ipm_memory_claim) {.claim_id = %"PRIu64", .proc_id = %"PRIu64", access = %s, .offset = %zu, .size = %zu}\n", claim.claim_id, claim.proc_id, claim.access == IPM_ACCESS_MODE_READ_WRITE ? "RW" : "RO", claim.offset, claim.size);
    }
}

void common_error_report_fn(const char* msg, const char* file, int line, const char* func, void* param)
{
    (void) param;
    fprintf(stderr, "IPM Error %s:%d - %s: \"%s\"\n", file, line, func, msg);
}

void* const state_ptr = (void*)0xB16B00B135;

void* allocate_callback(void* state, size_t size)
{
    assert(state == state_ptr);
    (void)state;
    return malloc(size);
}

void deallocate_callback(void* state, void* ptr)
{
    assert(state == state_ptr);
    (void)state;
    free(ptr);
}
