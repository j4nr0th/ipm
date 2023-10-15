//
// Created by jan on 14.10.2023.
//

#ifndef IPM_TEST_COMMON_H
#define IPM_TEST_COMMON_H
#include <ipm/ipm_memory.h>
#include <stdlib.h>
#include <stdint.h>

void print_claims(const ipm_memory* memory);

void common_error_report_fn(const char* msg, const char* file, int line, const char* func, void* param);

void* allocate_callback(void* state, size_t size);

void deallocate_callback(void* state, void* ptr);

extern void* const state_ptr;

#ifndef NDEBUG
    #ifdef __GNUC__
        #define ASSERTION_FAILED __builtin_trap()
    #endif
#else
    #define ASSERTION_FAILED exit(EXIT_FAILURE)
#endif
#define ASSERT(x) if ((x) == 0) {fprintf(stderr, "Failed assertion \"%s\" at %s:%d\n", #x, __FILE__, __LINE__); ASSERTION_FAILED; }(void)0

#endif //IPM_TEST_COMMON_H
