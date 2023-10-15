//
// Created by jan on 9.10.2023.
//

#ifndef IPM_IPM_ERROR_H
#define IPM_IPM_ERROR_H

typedef enum ipm_result_T ipm_result;
enum ipm_result_T
{
    IPM_RESULT_SUCCESS = 0,

    IPM_RESULT_ERR_ACCESS,
    IPM_RESULT_ERR_EXISTS,
    IPM_RESULT_ERR_BAD_VALUE,
    IPM_RESULT_ERR_MAX_FDS,
    IPM_RESULT_ERR_NAME_TOO_LONG,
    IPM_RESULT_ERR_MAX_FDS_SYS,
    IPM_RESULT_ERR_INVALID_FD,
    IPM_RESULT_ERR_BAD_FS,

    IPM_RESULT_ERR_OS_OUT_OF_MEMORY,
    IPM_RESULT_ERR_OS_UNEXPECTED,

    IPM_RESULT_ERR_BAD_ID,
    IPM_RESULT_ERR_BAD_SIZE,

    IPM_RESULT_ERR_NO_SEM,
    IPM_RESULT_ERR_NO_SEM_WAIT,

    IPM_RESULT_ERR_ALREADY_LOCKED,
    IPM_RESULT_ERR_NOT_LOCKED,

    IPM_RESULT_ERR_LIST_SIZE_MISMATCH,
    IPM_RESULT_ERR_INVALID_CLAIM,

    IPM_RESULT_ERR_BAD_INIT,
    IPM_RESULT_ERR_DEADLOCK,

    IPM_RESULT_INTERRUPTED,

    IPM_RESULT_ERR_BAD_ACCESS,
    IPM_RESULT_ERR_DOES_NOT_EXIST,

    IPM_RESULT_COUNT,
};

/**
 * Translates the enum value of ipm_result to a text representation
 * @param res Value to translate
 * @return Text representation of the parameter, or "Unknown" if it is not a valid value
 */
const char* ipm_result_to_str(ipm_result res);

/**
 * Translates the enum value of ipm_result to the full error message
 * @param res Value to translate
 * @return Full error message for the parameter, or "Unknown" if it is not a valid value
 */
const char* ipm_result_to_msg(ipm_result res);

#endif //IPM_IPM_ERROR_H
