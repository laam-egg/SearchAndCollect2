#ifndef SearchAndCollect2_ERROR_CODES_INCLUDED
#define SearchAndCollect2_ERROR_CODES_INCLUDED

#include <wchar.h>

typedef enum {
    ERR_NONE = 0,
    ERR_WINAPI,
    ERR_UNKNOWN,
    STOP_ITERATION,
    ERR_OUT_OF_MEMORY,
    ERR_INVALID_HANDLE,
    ERR_LKLIST,
    ERR_ACCESS_DENIED,
    ERR_INVALID_ARGUMENT
} ErrorCode;

extern wchar_t const* const ErrorMessage[];

extern size_t const NUM_ERROR_MESSAGES;

#endif // SearchAndCollect2_ERROR_CODES_INCLUDED
