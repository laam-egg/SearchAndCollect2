#ifndef SearchAndCollect2_COMMON_INCLUDED
#define SearchAndCollect2_COMMON_INCLUDED

#include <windows.h>
#include "SearchAndCollect2/error_codes.h"

#define MY_MAX_PATH_LENGTH 32767
#define MY_MAX_FILE_NAME_LENGTH MAX_PATH
#define MY_MAX_HASH_LENGTH 2048 // ATTENTION: may need to increase this in the future?

#define STATIC_ASSERT(cond, msg) typedef char static_assertion_##msg[(cond) ? 1 : -1]

void debug(wchar_t const* fmt, ...);

ErrorCode WaitMutexSafe(HANDLE hMutex, DWORD dwMilliseconds);

void joinPaths(wchar_t* dest, size_t const destSize, wchar_t const* part1, wchar_t const* part2);

void safeWstrcat(wchar_t* dest, size_t const destSize, wchar_t const* tail);

void concatPaths(wchar_t* dest, size_t const destSize, wchar_t const* tail);

wchar_t const* getFileExtension(wchar_t const* const path);

#endif // SearchAndCollect2_COMMON_INCLUDED
