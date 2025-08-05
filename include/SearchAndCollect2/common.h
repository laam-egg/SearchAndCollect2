#ifndef SearchAndCollect2_COMMON_INCLUDED
#define SearchAndCollect2_COMMON_INCLUDED

#include <windows.h>
#include "SearchAndCollect2/error_codes.h"

#define MY_MAX_PATH_LENGTH 32767
#define MY_MAX_FILE_NAME_LENGTH MAX_PATH

void debug(wchar_t const* fmt, ...);

ErrorCode WaitMutexSafe(HANDLE hMutex, DWORD dwMilliseconds);

void joinPaths(wchar_t* dest, size_t const destSize, wchar_t const* part1, wchar_t const* part2);

#endif // SearchAndCollect2_COMMON_INCLUDED
