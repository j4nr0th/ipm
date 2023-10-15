//
// Created by jan on 12.10.2023.
//
#include "internal.h"
#include "memory_claim.h"

ipm_bool claims_conflict(const ipm_memory_claim* claim_1, const ipm_memory_claim* claim_2)
{
    if (claim_1->proc_id == claim_2->proc_id) return 0;
    if (claim_1->access == IPM_ACCESS_MODE_READ_ONLY && claim_2->access == IPM_ACCESS_MODE_READ_ONLY) return 0;
    if (claim_1->offset > claim_2->offset)
    {
        if (claim_2->offset < claim_1->offset + claim_1->size)
        {
            return 1;
        }
        return 0;
    }
    else if (claim_1->offset < claim_2->offset)
    {
        if (claim_1->offset < claim_2->offset + claim_2->size)
        {
            return 1;
        }
        return 0;
    }
    return 1;
}

ipm_bool claim_encompasses_other(const ipm_memory_claim* claim_1, const ipm_memory_claim* claim_2)
{
    if (claim_1->proc_id != claim_2->proc_id || claim_1->access != claim_2->access) return 0;
    if (claim_1->offset >= claim_2->offset && claim_1->offset + claim_1->size >= claim_2->offset + claim_2->size) return 1;
    return 0;
}

ipm_result claim_add_to_list(const ipm_memory_claim* claim, ipm_claim_list* list)
{
    //  Take free space
//    ipm_result wait_res = ipm_semaphore_wait(&list->free_sem);
//    if (wait_res != IPM_RESULT_SUCCESS)
//    {
//        return wait_res;
//    }

    //  Take ownership of the list for a moment
//    ipm_result wait_res = ipm_mutex_lock(&list->list_mutex);
//    if (wait_res != IPM_RESULT_SUCCESS)
//    {
//        //  Return free space
//        ipm_semaphore_post(&list->free_sem);
//    }

    //  Check we're not out of bounds
    assert(list->capacity > list->count);
    if (list->count >= list->capacity)
    {
//        ipm_semaphore_post(&list->free_sem);
        ipm_mutex_unlock(&list->list_mutex);
        return IPM_RESULT_ERR_LIST_SIZE_MISMATCH;
    }

    //  Find where to insert in the sorted list
    unsigned pos;
    for (pos = 0; pos < list->count; ++pos)
    {
        if (list->claims[pos].offset > claim->offset)
        {
            break;
        }
    }

    //  Shift other elements
    if (pos != list->count)
    {
        memmove(list->claims + pos + 1, list->claims + pos, sizeof(*list->claims) * (list->count - pos - 1));
    }

    //  Insert in list
    list->claims[pos] = *claim;
    const size_t prev_count = atomic_fetch_add(&list->count, 1);
    (void) prev_count;
    assert(prev_count < list->capacity);

    //  Post to relevant semaphores
//    ipm_mutex_unlock(&list->list_mutex);

    return IPM_RESULT_SUCCESS;
}

ipm_result claim_remove_from_list(ipm_id claim_id, ipm_claim_list* list)
{
    //  Lock for exclusive access
//    ipm_result res = ipm_mutex_lock(&list->list_mutex);
//    if (res != IPM_RESULT_SUCCESS)
//    {
//        return res;
//    }
//    unsigned count = ipm_semaphore_count(&list->free_sem);
    unsigned count = atomic_fetch_sub(&list->count, 1);
    assert(count < list->capacity && count > 0);
//    if (count != list->count)
//    {
//        res = IPM_RESULT_ERR_LIST_SIZE_MISMATCH;
//        goto end;
//    }
    ipm_result res = IPM_RESULT_SUCCESS;
    if (count == 0)
    {
        //  No claims are currently active
        res = IPM_RESULT_ERR_INVALID_CLAIM;
        goto end;
    }

    unsigned pos;
    for (pos = 0; pos < count; ++pos)
    {
        if (claim_id == list->claims[pos].claim_id)
        {
            break;
        }
    }

    if (pos > count)
    {
        //  Claim was not found in the list
        res = IPM_RESULT_ERR_INVALID_CLAIM;
        goto end;
    }

//    const ipm_memory_claim claim = list->claims[pos];
    //  Remove claim from the list
    if (pos + 1 != count)
    {
        memmove(list->claims + pos, list->claims + pos + 1, sizeof(*list->claims) * ((count - 1) - pos - 1));
    }
    //  Post to free semaphore to indicate released claim in the list
//    ipm_semaphore_post(&list->free_sem);

end:
    //  Release exclusive access
//    ipm_mutex_unlock(&list->list_mutex);
    return res;
}

ipm_result claims_iterate(ipm_claim_list* list, int (* callback)(const ipm_memory_claim*, void*), void* param)
{
//    unsigned count = ipm_semaphore_count(&list->free_sem);
    unsigned count = list->count;
    assert(count <= list->capacity);
    if (count != list->count)
    {
        return IPM_RESULT_ERR_LIST_SIZE_MISMATCH;
    }
    if (count == list->capacity)
    {
        //  No claims are currently active
        return IPM_RESULT_SUCCESS;
    }

    ipm_result wait_res = ipm_mutex_lock(&list->list_mutex);
    if (wait_res != IPM_RESULT_SUCCESS)
    {
        return wait_res;
    }


    for (unsigned i = 0; i < count; ++i)
    {
        const ipm_memory_claim* p_claim = list->claims + i;
        if (callback(p_claim, param))
        {
            return IPM_RESULT_INTERRUPTED;
        }
    }

    ipm_mutex_unlock(&list->list_mutex);
    return IPM_RESULT_SUCCESS;
}

ipm_result claim_list_init(ipm_claim_list* list, size_t capacity)
{
    ipm_result res;// = ipm_semaphore_init(&list->free_sem, list->capacity);
//    if (res != IPM_RESULT_SUCCESS)
//    {
//        return res;
//    }
    res = ipm_mutex_init(&list->list_mutex);
    if (res != IPM_RESULT_SUCCESS)
    {
        return res;
    }
    res = ipm_condition_init(&list->list_cnd);
    if (res != IPM_RESULT_SUCCESS)
    {
        return res;
    }
    list->capacity = capacity;
    list->count = 0;
    list->claim_counter = 0;
    return res;
}

void claim_list_uninit(ipm_claim_list* list)
{
//    ipm_semaphore_destroy(&list->free_sem);
    ipm_mutex_destroy(&list->list_mutex);
    ipm_condition_destroy(&list->list_cnd);
}

