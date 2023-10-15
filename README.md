# IPM - simple Inter-Process Memory library

## Aim of the Library
The purpose of this library is to allow for straight forward way to share memory buffers between processes with separate virtual memory mappings. It also offers a way to synchronize access to regions of that memory between the different processes/threads.

## Usage

### Allocating and Freeing
The library exposes a type `ipm_memory`, through which the shared memory is accessed. It can be created when it does not exist by a call to `ipm_memory_create` or opened once it exists with a call to `ipm_memory_open`. Both `ipm_memory_create` and `ipm_memory_open` take a record of callbacks to use for memory allocation and error reporting as one of their parameters. It can be opened as having read-only access or as read-write access. It can then be properly closed with `ipm_memory_close` or just cleared without destroying it (like what should be done after a call to `fork`) with `ipm_memory_clean`. In case of a severe error during the creation of a shared memory block, a process that is waiting for the creation to finish may end up deadlocked. 

### Managing Shared Memory
The pointer to the shared memory region is accessed through a call to `ipm_memory_pointer`. Once a memory block is created with a specified size, it can not be shrunken to a smaller size. It can however be resized with a call to `ipm_memory_resize_grow`. If another process resized a block after it was open, the change to the size increase won't be visible before a call to `ipm_memory_sync`. This will likely invalidate the memory mapping, similar to what a call to `realloc` would do. Besides a change to a block's size, its access mode could be changed between read-write and read-only. This will also likely invalidate any pointers to the shared memory region.

General information about the shared memory object can be queried by a call to `ipm_memory_get_info`, which returns information about the current block `size` and `access`, as well as a pointer to the callback `struct` used by the `ipm_memory` object for memory allocation/deallocation and error reporting, which can be changed, given that the pointers from previous calls to previous callbacks can be safely passed to the new callbacks.

### Controlling Memory Access
In order to ensure that memory access to the shared memory region is coherent, synchronization based on reader-writer access is used. A process may issue a claim through a `ipm_memory` object using `ipm_memory_claim_region` to a region with an `offset` and a `size` for specific `access`. Each claim returns an associated `claim_id`, which is used to release the claim with a call to `ipm_memory_release_region`.

In case another `ipm_memory` object has write access to a part of that region, the process requesting access will sleep until the list of active claims will be updated, at which point it will attempt to claim the memory section again. This should be done carefully, as to not cause deadlocks. A process can also deadlock itself by attempting to access overlapping regions of memory from different `ipm_memory` objects on the same thread, since the access is tied to a specific `ipm_memory` instance.

Destroying an `ipm_memory` object releases all of its active claims when done with `ipm_memory_close`, but not when using `ipm_memory_clean`. Same holds for the case of abnormal termination. In that case, calling `ipm_memory_remove_all_active_claims` can be used to remove every active claim that is active for a shared block.

### Error Handling
Functions in this library fall in one of two categories: those that may fail when given valid parameters and those that may not. If the only way that the function can fail is by receiving invalid parameters is by receiving invalid arguments, it will return its result directly (such as `ipm_memory_get_pointer` or `ipm_memory_get_info`) or return nothing at all (such as `ipm_memory_close` or `ipm_memory_clean`).

The second kind of functions are the ones that may fail even if the parameters given to them are correct (such as `ipm_memory_claim_region` or `ipm_memory_create`). These all have the return type `ipm_result` and return any other values through pointers that they take as arguments. The type `ipm_result` is an enum, the value of which gives more information about why the error occurred. The value can be translated into a string with a call to `ipm_result_to_str` and the meaning of it is given by `ipm_result_to_msg`. As an example, a call to `ipm_result_str(IPM_RESULT_ERR_BAD_VALUE)` would return "IPM_RESULT_ERR_BAD_VALUE" and a call to `ipm_result_msg(IPM_RESULT_ERR_BAD_VALUE)` would return "Parameter had an invalid value". All error codes have the prefix `IPM_RESULT_ERR_*` and the general success code is `IPM_RESULT_SUCCESS`, the value of which is `0`.


