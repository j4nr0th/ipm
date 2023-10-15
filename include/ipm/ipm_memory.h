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

/**
 * Creates a new shared memory block, which should not exist before.
 * @param context Callbacks and associated state to use for memory allocation and error reporting.
 * @param block_size Size of the shared memory block. Must be non-zero.
 * @param block_name Identifier of the memory to block to create. Must not contain the '/' character.
 * @param access Desired access to the memory block mapping. Must be either IPM_ACCESS_MODE_READ_ONLY or
 * IPM_ACCESS_MODE_READ_WRITE.
 * @param p_memory Pointer which receives the created memory block info. Must be non-null.
 * @return IPM_RESULT_SUCCESS when successful, IPM_RESULT_ERR_EXISTS when a block already exists, or another value of
 * ipm_result enum for other errors.
 */
ipm_result ipm_memory_create(const ipm_context* context, size_t block_size, const char* block_name,
                             ipm_access_mode access, ipm_memory** p_memory);

/**
 * Opens an existing shared memory block.
 * @param context Callbacks and associated state to use for memory allocation and error reporting.
 * @param block_name Identifier of the memory to block to create. Must not contain the '/' character.
 * @param access Desired access to the memory block mapping. Must be either IPM_ACCESS_MODE_READ_ONLY or
 * IPM_ACCESS_MODE_READ_WRITE.
 * @param p_memory Pointer which receives the created memory block info. Must be non-null.
 * @return IPM_RESULT_SUCCESS when successful or another value of ipm_result enum for other errors.
 */
ipm_result ipm_memory_open(const ipm_context* context, const char* block_name,
                           ipm_access_mode access, ipm_memory** p_memory);

/**
 * Closes the shared memory block and performs all cleanup; removes memory claim associated with the memory object,
 * unmaps the shared memory, and destroys the shared memory block if it was the last reference to it.
 * @param memory Shared memory handle obtained from ipm_memory_open of ipm_memory_create.
 */
void ipm_memory_close(ipm_memory* memory);

/**
 * Closes the shared memory block and performs minimal cleanup; does NOT removes memory claim associated with the memory
 * object or destroy the shared memory block if it was the last reference to it. It only unmaps the shared memory. This
 * function's primary purpose is to be used after a call to fork(), where a process inherits a hande from a parent but
 * is not its owner.
 * @param memory Shared memory handle obtained from ipm_memory_open of ipm_memory_create.
 */
void ipm_memory_clean(ipm_memory* memory);

/**
 * Queries information about the shared memory block handle.
 * @param memory Shared memory handle obtained from ipm_memory_open of ipm_memory_create.
 * @return ipm_memory_info struct, that contains information about the memory block.
 */
ipm_memory_info ipm_memory_get_info(ipm_memory* memory);

/**
 * Updates information about the shared memory that the handle is associated with. In case a block was resized by a call
 * to ipm_memory_resize_grow, this causes memory to be remapped to a proper size. Any pointers to the memory associated
 * with the memory handle may be invalidated by a call to this function and should be updated.
 * @param memory Shared memory handle obtained from ipm_memory_open of ipm_memory_create.
 * @return IPM_RESULT_SUCCESS when successful or another value of ipm_result enum for other errors.
 */
ipm_result ipm_memory_sync(ipm_memory* memory);

/**
 * Resizes the shared memory that the handle is associated with. It also causes memory to be remapped to a proper size.
 * Any pointers to the memory associated with the memory handle may be invalidated by a call to this function and should
 * be updated.
 * @param memory Shared memory handle obtained from ipm_memory_open of ipm_memory_create.
 * @param new_size New desired size of the memory.
 * @return IPM_RESULT_SUCCESS when successful, IPM_RESULT_ERR_BAD_SIZE when the value of new_size is less than the
 * current size of the memory block, or another value of ipm_result enum for other errors.
 */
ipm_result ipm_memory_resize_grow(ipm_memory* memory, size_t new_size);

/**
 * The function causes causes memory to be remapped to a proper access. Any pointers to the memory associated
 * with the memory handle may be invalidated by a call to this function and should be updated.
 * @param memory Shared memory handle obtained from ipm_memory_open of ipm_memory_create.
 * @param access_mode New desired access mode. Must be either IPM_ACCESS_MODE_READ_ONLY of IPM_ACCESS_MODE_READ_WRITE.
 * @return IPM_RESULT_SUCCESS when successful or another value of ipm_result enum for other errors.
 */
ipm_result ipm_memory_change_access(ipm_memory* memory, ipm_access_mode access_mode);

/**
 * The function returns the number of memory handles currently referencing the shared memory block. A value of 1
 * indicates that the current process is the only one that has the memory currently opened.
 * @param memory Shared memory handle obtained from ipm_memory_open of ipm_memory_create.
 * @return Number of handles currently referencing the shared memory block.
 */
unsigned ipm_memory_ref_count(const ipm_memory* memory);

/**
 * Claims a region of the shared memory at specified offset for count bytes with specified access. When a region is
 * already claimed by another process in a way that would conflict with this claim, the process will wait on a
 * conditional variable that will be signaled when a claim is removed from the list of active claims.
 * @param memory Shared memory handle obtained from ipm_memory_open of ipm_memory_create.
 * @param access Desired access mode. Must be either IPM_ACCESS_MODE_READ_ONLY of IPM_ACCESS_MODE_READ_WRITE.
 * @param offset Offset in the memory region where the claim is to be made.
 * @param count The number of bytes to claim from the offset.
 * @param p_claim_id Pointer that receives the ID associated with the claim. This is used to release the claim.
 * @return IPM_RESULT_SUCCESS when successful or another value of ipm_result enum for other errors.
 */
ipm_result ipm_memory_claim_region(ipm_memory* memory, ipm_access_mode access, size_t offset, size_t count,
                                   ipm_id* p_claim_id);

/**
 * Releases a claim on a region of the shared memory associated with the given claim_id. Releasing a claim will also
 * signal a conditional variable and allow other sleeping processes to try and make new claims.
 * @param memory Shared memory handle obtained from ipm_memory_open of ipm_memory_create.
 * @param claim_id ID of the claim, returned from a previous call to ipm_memory_claim_region.
 * @return IPM_RESULT_SUCCESS when successful, IPM_RESULT_ERR_INVALID_CLAIM when a claim_id is not valid,
 * or another value of ipm_result enum for other errors.
 */
ipm_result ipm_memory_release_region(ipm_memory* memory, ipm_id claim_id);

/**
 * Releases all active claims associated with the shared memory handle.
 * @param memory Shared memory handle obtained from ipm_memory_open of ipm_memory_create.
 * @return IPM_RESULT_SUCCESS when successful or another value of ipm_result enum for other errors.
 */
ipm_result ipm_memory_release_all(ipm_memory* memory);

/**
 * Releases ALL active claims associated with the shared memory, not just this handle.
 * @param memory Shared memory handle obtained from ipm_memory_open of ipm_memory_create.
 * @return IPM_RESULT_SUCCESS when successful or another value of ipm_result enum for other errors.
 */
ipm_result ipm_memory_remove_all_active_claims(ipm_memory* memory);

/**
 * Returns the mapped pointer to the shared memory region.
 * @param memory Shared memory handle obtained from ipm_memory_open of ipm_memory_create.
 * @return Pointer to the shared memory region, or NULL if remapping of the memory failed at some point.
 */
void* ipm_memory_pointer(ipm_memory* memory);


#endif //IPM_IPM_MEMORY_H
