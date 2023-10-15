//
// Created by jan on 14.10.2023.
//

#include "ipm_memory_internal.h"
#include "../include/ipm/ipm_memory.h"
#include "internal.h"


static inline size_t round_size(size_t size)
{
    const size_t remainder = size % IPM_MEMORY_PAGE_SIZE;
    if (remainder)
    {
        return size + (IPM_MEMORY_PAGE_SIZE - remainder);
    }
    return size;
}

ipm_result ipm_memory_create(
        const ipm_context* context, size_t block_size, const char* block_name, ipm_access_mode access,
        ipm_memory** p_memory)
{
    //  Check parameters
    assert(context);
    assert(block_size > 0);
    assert(strchr(block_name, '/') == NULL);
    assert(access == IPM_ACCESS_MODE_READ_ONLY || access == IPM_ACCESS_MODE_READ_WRITE);
    assert(p_memory);
    assert(strlen(block_name) <= IPM_MAX_NAME_LEN);
    ipm_memory* const this = ipm_alloc(context, sizeof(*this));
    if (!this)
    {
        return IPM_RESULT_ERR_OS_OUT_OF_MEMORY;
    }

    this->ctx = *context;
    const size_t proper_size = round_size(block_size);
    assert(proper_size > 0);
    assert((proper_size & IPM_MEMORY_PAGE_SIZE_MASK) == 0);
    strncpy(this->block_name, block_name, sizeof(this->block_name) - 1);

    const size_t claim_size = round_size(sizeof(ipm_claim_list) + IPM_DEFAULT_CLAIM_CAPACITY * sizeof(ipm_memory_claim));
    ipm_result res = shared_memory_block_create(
            context, block_name, IPM_MEMORY_BLOCK_ACTIVE_CALIMS, claim_size, IPM_ACCESS_MODE_READ_WRITE, &this->active_claims);
    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(context, "Could not create the shared memory block claim list %s, reason: %s (%s)", block_name,
                  ipm_result_to_str(res), ipm_result_to_msg(res));
        ipm_free(context, this);
        return res;
    }
    const size_t real_claim_capacity = (claim_size - sizeof(ipm_claim_list)) / sizeof(ipm_memory_claim);
    res = claim_list_init(this->active_claims.memory, real_claim_capacity);
    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(context, "Could not initialize the claims list for the memory block %s, reason: %s (%s)", block_name,
                  ipm_result_to_str(res), ipm_result_to_msg(res));
        shared_memory_block_close(context, &this->active_claims, 0, NULL);
        ipm_free(context, this);
        return res;
    }

    res = shared_memory_block_create(
            context, block_name, IPM_MEMORY_BLOCK_REAL_MEMORY, proper_size, access, &this->real_memory);
    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(context, "Could not create the shared memory block %s, reason: %s (%s)", block_name,
                  ipm_result_to_str(res), ipm_result_to_msg(res));
        claim_list_uninit(this->active_claims.memory);
        shared_memory_block_close(context, &this->active_claims, 0, NULL);
        ipm_free(context, this);
        return res;
    }

    *p_memory = this;
    return IPM_RESULT_SUCCESS;
}

ipm_result
ipm_memory_open(const ipm_context* context, const char* block_name, ipm_access_mode access, ipm_memory** p_memory)
{
    //  Check parameters
    assert(context);
    assert(strchr(block_name, '/') == NULL);
    assert(access == IPM_ACCESS_MODE_READ_ONLY || access == IPM_ACCESS_MODE_READ_WRITE);
    assert(p_memory);
    assert(strlen(block_name) <= IPM_MAX_NAME_LEN);
    ipm_memory* const this = ipm_alloc(context, sizeof(*this));
    if (!this)
    {
        return IPM_RESULT_ERR_OS_OUT_OF_MEMORY;
    }

    this->ctx = *context;
    strncpy(this->block_name, block_name, sizeof(this->block_name) - 1);

    ipm_result res = shared_memory_block_open(
            context, block_name, IPM_MEMORY_BLOCK_REAL_MEMORY, access, &this->real_memory);
    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(context, "Could not open the shared memory block %s, reason: %s (%s)", block_name,
                  ipm_result_to_str(res), ipm_result_to_msg(res));
        ipm_free(context, this);
        return res;
    }

    res = shared_memory_block_open(
            context, block_name, IPM_MEMORY_BLOCK_ACTIVE_CALIMS, IPM_ACCESS_MODE_READ_WRITE, &this->active_claims);
    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(context, "Could not open the shared memory block claim list %s, reason: %s (%s)", block_name,
                  ipm_result_to_str(res), ipm_result_to_msg(res));
        shared_memory_block_close(context, &this->real_memory, 0, NULL);
        ipm_free(context, this);
        return res;
    }
    *p_memory = this;
    return IPM_RESULT_SUCCESS;
}

static void claim_list_dtor_wrapper(void* ptr)
{
    ipm_claim_list* const list = ptr;
    claim_list_uninit(list);
}

void ipm_memory_close(ipm_memory* memory)
{
    ipm_memory_release_all(memory);
    shared_memory_block_close(&memory->ctx, &memory->active_claims, claim_list_dtor_wrapper, memory->active_claims.memory);
    shared_memory_block_close(&memory->ctx, &memory->real_memory, 0, NULL);
    ipm_free(&memory->ctx, memory);
}

void ipm_memory_clean(ipm_memory* memory)
{
    shared_memory_block_clean(&memory->active_claims);
    shared_memory_block_clean(&memory->real_memory);
    ipm_free(&memory->ctx, memory);
}

ipm_memory_info ipm_memory_get_info(ipm_memory* memory)
{
    const ipm_memory_info result =
            {
            .active_claims = ((ipm_claim_list*)memory->active_claims.memory)->count,
            .block_size = memory->real_memory.size,
            .mapping_address = memory->real_memory.memory,
            .access_id = memory->real_memory.access_id,
            .name = memory->block_name,
            .p_ctx = &memory->ctx,
            .memory_access_mode = memory->real_memory.access_mode,
            };
    return result;
}

ipm_result ipm_memory_sync(ipm_memory* memory)
{
    ipm_result res = shared_memory_block_update_mapping(&memory->ctx, &memory->real_memory, memory->real_memory.access_mode);
    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(&memory->ctx, "Memory sync for block \"%s\" failed, reason: %s (%s)", memory->block_name, ipm_result_to_str(res), ipm_result_to_msg(res));
    }
    return res;
}

ipm_result ipm_memory_resize_grow(ipm_memory* memory, size_t new_size)
{
    new_size = round_size(new_size);
    if (new_size < memory->real_memory.size)
    {
        IPM_ERROR(&memory->ctx, "Can not decrease the size of the memory block from %zu to %zu", memory->real_memory.size, new_size);
        return IPM_RESULT_ERR_BAD_VALUE;
    }
    const ipm_result res = shared_memory_block_resize(&memory->ctx, &memory->real_memory, new_size);
    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(&memory->ctx, "Memory resizing to %zu bytes for block \"%s\" failed, reason: %s (%s)", memory->real_memory.size, memory->block_name, ipm_result_to_str(res), ipm_result_to_msg(res));
        if (memory->real_memory.memory == NULL)
        {
            IPM_ERROR(&memory->ctx, "Failure with resizing made it so that the block mapping is no longer valid");
        }
        return res;
    }
    return IPM_RESULT_SUCCESS;
}

ipm_result ipm_memory_change_access(ipm_memory* memory, ipm_access_mode access_mode)
{
    ipm_result res = shared_memory_block_update_mapping(&memory->ctx, &memory->real_memory, access_mode);
    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(&memory->ctx, "Memory access change for block \"%s\" failed, reason: %s (%s)", memory->block_name, ipm_result_to_str(res), ipm_result_to_msg(res));
    }
    return res;
}

unsigned ipm_memory_ref_count(const ipm_memory* memory)
{
    return memory->real_memory.header->refcount;
}

ipm_result ipm_memory_claim_region(ipm_memory* memory, ipm_access_mode access, size_t offset, size_t count, ipm_id* p_claim_id)
{
    assert(access == IPM_ACCESS_MODE_READ_WRITE || access == IPM_ACCESS_MODE_READ_ONLY);
    assert(count > 0);
    assert(p_claim_id);
    if (memory->real_memory.access_mode == IPM_ACCESS_MODE_READ_ONLY && access == IPM_ACCESS_MODE_READ_WRITE)
    {
        IPM_ERROR(&memory->ctx, "Memory block was opened as read-only and can not be claimed for read only access");
        return IPM_RESULT_ERR_BAD_ACCESS;
    }
    if (memory->real_memory.size < offset + count)
    {
        IPM_ERROR(&memory->ctx, "Memory block has the size of %zu, so region [%zu, %zu) can not be claimed", memory->real_memory.size, offset, offset + count);
        return IPM_RESULT_ERR_BAD_VALUE;
    }

    //  Lock access list
    ipm_claim_list* const list = memory->active_claims.memory;
    assert(list);
    ipm_result res = ipm_mutex_lock(&list->list_mutex);
    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(&memory->ctx, "Could not lock access list mutex, reason: %s (%s)", ipm_result_to_str(res),
                  ipm_result_to_msg(res));
        return res;
    }

    //  Check if there are any conflicting claims currently active
    ipm_id id = atomic_fetch_add(&list->claim_counter, 1);
    const ipm_memory_claim claim =
            {
            .offset = offset,
            .size = count,
            .access = access,
            .claim_id = id,
            .proc_id = memory->real_memory.access_id,
            };
    for (;;)
    {
        unsigned i;
        for (i = 0; i < list->count; ++i)
        {
            if (claims_conflict(&claim, list->claims + i))
            {
                break;
            }
        }
        if (i == list->count && list->count != list->capacity)
        {
            //  No claims conflict and there's enough space
            break;
        }
        res = ipm_condition_wait(&list->list_cnd, &list->list_mutex);
        if (res != IPM_RESULT_SUCCESS)
        {
            IPM_ERROR(&memory->ctx, "Could not wait on condition variable, reason: %s (%s)", ipm_result_to_str(res), ipm_result_to_msg(res));
            ipm_mutex_unlock(&list->list_mutex);
            return IPM_RESULT_SUCCESS;
        }
    }

    res = claim_add_to_list(&claim, list);
    ipm_mutex_unlock(&list->list_mutex);

    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(&memory->ctx, "Could not add memory claim to list, reason: %s (%s)", ipm_result_to_str(res), ipm_result_to_msg(res));
    }
    *p_claim_id = claim.claim_id;

    return res;
}

ipm_result ipm_memory_release_region(ipm_memory* memory, ipm_id claim_id)
{
    //  Lock access list
    ipm_claim_list* const list = memory->active_claims.memory;
    assert(list);
    ipm_result res = ipm_mutex_lock(&list->list_mutex);
    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(&memory->ctx, "Could not lock access list mutex, reason: %s (%s)", ipm_result_to_str(res),
                  ipm_result_to_msg(res));
        return res;
    }

    //  Check if there are any conflicting claims currently active
    res = claim_remove_from_list(claim_id, list);
    ipm_mutex_unlock(&list->list_mutex);
    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(&memory->ctx, "Could not add memory claim to list, reason: %s (%s)", ipm_result_to_str(res), ipm_result_to_msg(res));
    }
    else
    {
        //  Signal that a claim was removed from the list, so threads should check if they can now add any of their claims
        ipm_condition_signal(&list->list_cnd);
    }

    return res;
}

ipm_result ipm_memory_release_all(ipm_memory* memory)
{
    ipm_claim_list* const list = memory->active_claims.memory;
    ipm_result res = ipm_mutex_lock(&list->list_mutex);
    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(&memory->ctx, "Could not lock the claim list mutex, reason: %s (%s)", ipm_result_to_str(res), ipm_result_to_msg(res));
        return res;
    }

    for (int i = 0; i < (int)list->count; ++i)
    {
        if (list->claims[i].proc_id == memory->real_memory.access_id)
        {
            (void)claim_remove_from_list(list->claims[i].claim_id, list);
            i -= 1;
        }
    }

    ipm_mutex_unlock(&list->list_mutex);
    return res;
}

void* ipm_memory_pointer(ipm_memory* memory)
{
    return memory->real_memory.memory;
}

ipm_result ipm_memory_remove_all_active_claims(ipm_memory* memory)
{
    ipm_claim_list* const list = memory->active_claims.memory;
    ipm_result res = ipm_mutex_lock(&list->list_mutex);
    if (res != IPM_RESULT_SUCCESS)
    {
        IPM_ERROR(&memory->ctx, "Could not lock the claim list mutex, reason: %s (%s)", ipm_result_to_str(res), ipm_result_to_msg(res));
        return res;
    }

    while (list->count)
    {
        (void)claim_remove_from_list(list->claims[list->count - 1].claim_id, list);
    }

    ipm_mutex_unlock(&list->list_mutex);
    return res;
}

ipm_claim_list* internal_ipm_memory_clam_list(ipm_memory* memory)
{
    return memory->active_claims.memory;
}
