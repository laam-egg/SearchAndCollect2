#ifndef SearchAndCollect2_ERROR_CODES_INCLUDED
#define SearchAndCollect2_ERROR_CODES_INCLUDED

typedef enum {
    ERR_NONE = 0,
    ERR_WINAPI,
    ERR_WAITMUTEX,
    STOP_ITERATION,
    ERR_OUT_OF_MEMORY
} ErrorCode;

#endif // SearchAndCollect2_ERROR_CODES_INCLUDED
