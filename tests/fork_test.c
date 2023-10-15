#include <stdio.h>
#include "test_common.h"
#include <ipm/ipm_memory.h>
#include <unistd.h>
#include <wait.h>


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
    

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Could not fork");
        ASSERT(pid != -1);
    }
    else if (pid == 0)
    {
        //  Child
        ipm_memory_clean(mem);
        mem = NULL;
        ipm_id claim_id;
        ipm_memory* mem_child;
        res = ipm_memory_open(&ctx, "cool_block", IPM_ACCESS_MODE_READ_WRITE, &mem_child);
        ASSERT(res == IPM_RESULT_SUCCESS);
        char* const buffer = ipm_memory_pointer(mem_child);
        res = ipm_memory_claim_region(mem_child, IPM_ACCESS_MODE_READ_WRITE, 0, 32, &claim_id);
        ASSERT(res == IPM_RESULT_SUCCESS);
        snprintf(buffer, 32, "Hello my guy");
        sleep(3);
        res = ipm_memory_release_region(mem_child, claim_id);
        ASSERT(res == IPM_RESULT_SUCCESS);
        ipm_memory_close(mem_child);
        exit(EXIT_SUCCESS);
    }
    else
    {
        //  Parent
        void* const ptr = ipm_memory_pointer(mem);
        char* buffer = ptr;
        int ret_v;
        ipm_id claim_id;
        sleep(1);
        res = ipm_memory_claim_region(mem, IPM_ACCESS_MODE_READ_ONLY, 0, 32, &claim_id);
        ASSERT(res == IPM_RESULT_SUCCESS);
        printf("Message was: \"%.*s\"\n", 32, buffer);
        res = ipm_memory_release_region(mem, claim_id);
        ASSERT(res == IPM_RESULT_SUCCESS);
        pid_t child_id = wait(&ret_v);
        ASSERT(child_id == pid);
    }

    ipm_memory_close(mem);
    //  This should report error message if correct, since the block has to get cleaned up
    res = ipm_memory_open(&ctx, "cool_block", IPM_ACCESS_MODE_READ_WRITE, &mem);
    ASSERT(res == IPM_RESULT_ERR_DOES_NOT_EXIST);


    return 0;
}
