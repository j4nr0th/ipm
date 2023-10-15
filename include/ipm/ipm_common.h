//
// Created by jan on 9.10.2023.
//

#ifndef IPM_IPM_COMMON_H
#define IPM_IPM_COMMON_H
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdatomic.h>

typedef uint64_t ipm_id;
typedef uint_fast8_t ipm_bool;


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
    void (*report_callback)(const char* msg, const char* file, int line, const char* function, void* param);
    void* report_param;
    void* (*alloc_callback)(void* alloc_param, size_t size);
    void* alloc_param;
    void (*free_callback)(void* free_param, void* ptr);
    void* free_param;
};
typedef struct ipm_context_T ipm_context;

void* ipm_alloc_real(const ipm_context* context, size_t size, const char* file, int line, const char* function);

void ipm_free(const ipm_context* context, void* ptr);

#define ipm_alloc(callbacks, size) ipm_alloc_real((callbacks), (size), __FILE__, __LINE__, __func__)

#ifdef __GNUC__
__attribute__((format(printf, 2, 6)))
#endif
void ipm_report_error(const ipm_context* context, const char* msg, const char* file, int line, const char* function, ...);

#define IPM_ERROR(callbacks, fmt, ...) ipm_report_error((callbacks), (fmt), __FILE__, __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__)

#endif //IPM_IPM_COMMON_H
