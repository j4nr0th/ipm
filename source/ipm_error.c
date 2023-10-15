//
// Created by jan on 9.10.2023.
//

#include "../include/ipm/ipm_error.h"

static const struct
{
    const char* str;
    const char* msg;
} RESULT_INFO[IPM_RESULT_COUNT] =
        {
        [IPM_RESULT_SUCCESS] = {.str = "IPM_RESULT_SUCCESS", .msg = "Success"},

        [IPM_RESULT_ERR_EXISTS] = {.str = "IPM_RESULT_ERR_EXISTS", .msg = "Memory object already exists"},

        [IPM_RESULT_ERR_ACCESS] = {.str = "IPM_RESULT_ERR_ACCESS", .msg = "Don't have sufficient permissions"},
        [IPM_RESULT_ERR_BAD_VALUE] = {.str = "IPM_RESULT_ERR_BAD_VALUE", .msg = "Parameter had an invalid value"},
        [IPM_RESULT_ERR_MAX_FDS] = {.str = "IPM_RESULT_ERR_MAX_FDS", .msg = "Process has maximum number of FDs"},
        [IPM_RESULT_ERR_NAME_TOO_LONG] = {.str = "IPM_RESULT_ERR_NAME_TOO_LONG", .msg = "Name of memory object was greater than PATH_MAX"},
        [IPM_RESULT_ERR_MAX_FDS_SYS] = {.str = "IPM_RESULT_ERR_MAX_FDS_SYS", .msg = "System has maximum number of FDs"},
        [IPM_RESULT_ERR_INVALID_FD] = {.str = "IPM_RESULT_ERR_INVALID_FD", .msg = "Invalid FD was passed to a function"},
        [IPM_RESULT_ERR_BAD_FS] = {.str = "IPM_RESULT_ERR_BAD_FS", .msg = "Operation not allowed by the filesystem"},

        [IPM_RESULT_ERR_OS_OUT_OF_MEMORY] = {.str = "IPM_RESULT_ERR_OS_OUT_OF_MEMORY", .msg = "Operating system ran out of memory"},
        [IPM_RESULT_ERR_OS_UNEXPECTED] = {.str = "IPM_RESULT_ERR_OS_UNEXPECTED", .msg = "OS error occurred, but type was not expected"},

        [IPM_RESULT_ERR_BAD_ID] = {.str = "IPM_RESULT_ERR_BAD_ID", .msg = "ID did not match real block ID"},
        [IPM_RESULT_ERR_BAD_SIZE] = {.str = "IPM_RESULT_ERR_BAD_SIZE", .msg = "Size did not match real block size"},

        [IPM_RESULT_ERR_NO_SEM] = {.str = "IPM_RESULT_ERR_NO_SEM", .msg = "Could not create synchronization semaphore"},
        [IPM_RESULT_ERR_NO_SEM_WAIT] = {.str = "IPM_RESULT_ERR_NO_SEM_WAIT", .msg = "Waiting on semaphore failed"},

        [IPM_RESULT_ERR_ALREADY_LOCKED] = {.str = "IPM_RESULT_ERR_ALREADY_LOCKED", .msg = "Lock was already acquired"},
        [IPM_RESULT_ERR_NOT_LOCKED] = {.str = "IPM_RESULT_ERR_NOT_LOCKED", .msg = "Lock was not acquired"},
        [IPM_RESULT_ERR_LIST_SIZE_MISMATCH] = {.str = "IPM_RESULT_ERR_LIST_SIZE_MISMATCH", .msg = "Length of the list does not match between semaphores and internal data"},
        [IPM_RESULT_ERR_INVALID_CLAIM] = {.str = "IPM_RESULT_ERR_INVALID_CLAIM", .msg = "Claim does not exist"},
        [IPM_RESULT_ERR_BAD_INIT] = {.str = "IPM_RESULT_ERR_BAD_INIT", .msg = "Initialization was done in an incorrect way"},
        [IPM_RESULT_ERR_DEADLOCK] = {.str = "IPM_RESULT_ERR_DEADLOCK", .msg = "Locking would cause a deadlock"},

        [IPM_RESULT_INTERRUPTED] = {.str = "IPM_RESULT_INTERRUPTED", .msg = "Function was interrupted by user"},

        [IPM_RESULT_ERR_BAD_ACCESS] = {.str = "IPM_RESULT_ERR_BAD_ACCESS", .msg = "Desire access is incompatible with the memory block"},
        [IPM_RESULT_ERR_DOES_NOT_EXIST] = {.str = "IPM_RESULT_ERR_DOES_NOT_EXIST", .msg = "Memory block does not exist"},
        };

const char* ipm_result_to_str(ipm_result res)
{
    if (res < IPM_RESULT_SUCCESS || res >= IPM_RESULT_COUNT)
    {
        return "Unknown";
    }
    return RESULT_INFO[res].str;
}

const char* ipm_result_to_msg(ipm_result res)
{
    if (res < IPM_RESULT_SUCCESS || res >= IPM_RESULT_COUNT)
    {
        return "Unknown";
    }
    return RESULT_INFO[res].msg;
}
