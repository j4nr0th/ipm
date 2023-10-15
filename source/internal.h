//
// Created by jan on 15.10.2023.
//

#ifndef IPM_INTERNAL_H
#define IPM_INTERNAL_H
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdatomic.h>
#include "../include/ipm/ipm_common.h"

#ifdef __GNUC__
    #define IPM_INTERNAL_FUNCTION __attribute__((visibility("hidden")))
#endif


#ifndef IPM_INTERNAL_FUNCTION
    #define IPM_INTERNAL_FUNCTION
#endif

typedef uint_fast8_t ipm_bool;


IPM_INTERNAL_FUNCTION void* ipm_alloc_real(const ipm_context* context, size_t size, const char* file, int line, const char* function);

IPM_INTERNAL_FUNCTION void ipm_free(const ipm_context* context, void* ptr);

#define ipm_alloc(callbacks, size) ipm_alloc_real((callbacks), (size), __FILE__, __LINE__, __func__)

#ifdef __GNUC__
__attribute__((format(printf, 2, 6)))
#endif
IPM_INTERNAL_FUNCTION
void ipm_report_error(const ipm_context* context, const char* msg, const char* file, int line, const char* function, ...);

#define IPM_ERROR(callbacks, fmt, ...) ipm_report_error((callbacks), (fmt), __FILE__, __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__)

#endif //IPM_INTERNAL_H
