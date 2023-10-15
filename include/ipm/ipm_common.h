//
// Created by jan on 9.10.2023.
//

#ifndef IPM_IPM_COMMON_H
#define IPM_IPM_COMMON_H
#include <stdint.h>
#include <stddef.h>

typedef uint64_t ipm_id;

enum
{
    IPM_MAX_NAME_LEN = 256,
    IPM_MEMORY_PAGE_SIZE = 4096,
    IPM_MEMORY_PAGE_SIZE_MASK = (4096 - 1),
    IPM_DEFAULT_CLAIM_CAPACITY = 64,
};

enum ipm_access_mode_T
{
    IPM_ACCESS_MODE_READ_ONLY = 0,
    IPM_ACCESS_MODE_READ_WRITE = 1,
};
typedef enum ipm_access_mode_T ipm_access_mode;

struct ipm_context_T
{
    /**
     * Used to report additional information about an error when it occurs
     * @param msg Error message
     * @param file The name of the file where the error was reported
     * @param line The line where the error was reported in the file
     * @param function The function where the error was reported from
     * @param param Value of ipm_context::report_param
     */
    void (*report_callback)(const char* msg, const char* file, int line, const char* function, void* param);
    /**
     * Value passed to ipm_context::report_callback as its last argument
     */
    void* report_param;
    /**
     * Callback that will be used to allocate memory ipm_memory needs internally, as well as for error reporting
     * @param alloc_param Value of ipm_context::alloc_param
     * @param size Size of the memory allocation that is required
     * @return A valid pointer to a region of memory of at least size bytes
     */
    void* (*alloc_callback)(void* alloc_param, size_t size);
    /**
     * Value passed to ipm_context::alloc_callback as its first argument
     */
    void* alloc_param;
    /**
     * Callback that will be used to deallocate memory ipm_memory no longer needs internally, as well as for error
     * reporting
     * @param free_param Value of ipm_context::free_param
     * @param ptr Pointer previously obtained from a call to ipm_context::alloc_callback
     */
    void (*free_callback)(void* free_param, void* ptr);
    /**
     * Value passed to ipm_context::free_callback as its first argument
     */
    void* free_param;
};
typedef struct ipm_context_T ipm_context;

#endif //IPM_IPM_COMMON_H
