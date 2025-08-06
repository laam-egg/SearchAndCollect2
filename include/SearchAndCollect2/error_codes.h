#ifndef SearchAndCollect2_ERROR_CODES_INCLUDED
#define SearchAndCollect2_ERROR_CODES_INCLUDED

typedef enum {
    ERR_NONE = 0,
    ERR_WINAPI,
    STOP_ITERATION,
    ERR_OUT_OF_MEMORY,
    ERR_INVALID_HANDLE,
    ERR_LKLIST
} ErrorCode;

#endif // SearchAndCollect2_ERROR_CODES_INCLUDED
