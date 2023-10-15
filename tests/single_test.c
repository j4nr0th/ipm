#include <stdio.h>
#include "test_common.h"
#include <ipm/ipm_memory.h>

int main()
{
    const ipm_context ctx =
            {
            .report_param = NULL,
            .report_callback = common_error_report_fn,
            .alloc_callback = allocate_callback,
            .free_callback = deallocate_callback,
            .alloc_param = state_ptr,
            .free_param = state_ptr,
            };

    ipm_memory* mem = NULL;
    ipm_result res = ipm_memory_create(&ctx, 69, "cool_block", IPM_ACCESS_MODE_READ_WRITE, &mem);
    ASSERT(res == IPM_RESULT_SUCCESS);

    const ipm_memory_info block_info = ipm_memory_get_info(mem);
    ASSERT(block_info.active_claims == 0);
    ASSERT(block_info.memory_access_mode == IPM_ACCESS_MODE_READ_WRITE);
    ASSERT(block_info.block_size >= 69);
    ipm_id claim_id1, claim_id2;
    res = ipm_memory_claim_region(mem, IPM_ACCESS_MODE_READ_WRITE, 16, 16, &claim_id1);
    ASSERT(res == IPM_RESULT_SUCCESS);
    printf("\nClaims at point 1:\n");
    print_claims(mem);

    res = ipm_memory_claim_region(mem, IPM_ACCESS_MODE_READ_WRITE, 24, 16, &claim_id2);
    ASSERT(res == IPM_RESULT_SUCCESS);
    printf("\nClaims at point 2:\n");
    print_claims(mem);

    res = ipm_memory_release_region(mem, claim_id2);
    ASSERT(res == IPM_RESULT_SUCCESS);
    printf("\nClaims at point 3:\n");
    print_claims(mem);

    res = ipm_memory_release_region(mem, claim_id1);
    ASSERT(res == IPM_RESULT_SUCCESS);
    printf("\nClaims at point 4:\n");
    print_claims(mem);

    ipm_memory_close(mem);
    res = ipm_memory_open(&ctx, "cool_block", IPM_ACCESS_MODE_READ_WRITE, &mem);
    ASSERT(res == IPM_RESULT_ERR_DOES_NOT_EXIST);


    return 0;
}
