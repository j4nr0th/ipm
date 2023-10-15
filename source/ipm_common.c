//
// Created by jan on 9.10.2023.
//

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "../include/ipm/ipm_common.h"
#include "internal.h"

void* ipm_alloc_real(const ipm_context* context, size_t size, const char* file, int line, const char* function)
{
    void* const ptr = context->alloc_callback(context->alloc_param, size);
    if (!ptr)
    {
        char buffer[64];
        (void) snprintf(buffer, sizeof(buffer), "Could not allocate %zu bytes", size);
        if (context->report_callback)
        {
            context->report_callback(buffer, file, line, function, context->report_param);
        }
        return NULL;
    }
    return ptr;
}

void ipm_free(const ipm_context* context, void* ptr)
{
    if (!ptr)
    {
        return;
    }
    context->free_callback(context->free_param, ptr);
}

void
ipm_report_error(const ipm_context* context, const char* msg, const char* file, int line, const char* function, ...)
{
    if (context->report_callback == NULL)
    {
        return;
    }
    va_list args, cpy;
    va_start(args, function);
    va_copy(cpy, args);
    const int fmt_len = vsnprintf(NULL, 0, msg, cpy);
    va_end(cpy);
    if (fmt_len <= 0) return;
    char* const buffer = ipm_alloc(context, fmt_len + 1);
    if (!buffer)
    {
        va_end(args);
        return;
    }
    (void) vsnprintf(buffer, fmt_len + 1, msg, args);
    va_end(args);
    context->report_callback(buffer, file, line, function, context->report_param);
    ipm_free(context, buffer);
}
