#include "SearchAndCollect2/common.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <wchar.h>

void debug(wchar_t const* fmt, ...) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    fwprintf(
        stderr, L"[%04d/%02d/%02d %02d:%02d:%02d] DEBUG: ",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond
    );

    {
        va_list args;
        va_start(args, fmt);
        vfwprintf(stderr, fmt, args);
        va_end(args);
    }

    fwprintf(stderr, L"\n");
}

ErrorCode WaitMutexSafe(HANDLE hMutex, DWORD dwMilliseconds) {
    DWORD result = WaitForSingleObject(hMutex, dwMilliseconds);
    if (result != WAIT_OBJECT_0) {
        debug(L"WaitMutex failed, closing the mutex and reporting upstream handler");
        CloseHandle(hMutex);
        return ERR_WINAPI;
    }

    return ERR_NONE;
}

void joinPaths(wchar_t* dest, size_t const destSize, wchar_t const* part1, wchar_t const* part2) {
    swprintf(dest, destSize, L"%ls\\%ls", part1, part2);
}

void safeWstrcat(wchar_t* dest, size_t const destSize, wchar_t const* tail) {
    size_t capacityOfDest = destSize / sizeof(wchar_t);
    size_t numCharsInDest = wcslen(dest);
    size_t maxNumCharsToAppend = capacityOfDest - numCharsInDest - 1;

    wcsncat(dest, tail, maxNumCharsToAppend);
}

void concatPaths(wchar_t* dest, size_t const destSize, wchar_t const* tail) {
    size_t destCount = wcslen(dest);
    if (destCount > 0 && dest[destCount - 1] != L'\\') {
        safeWstrcat(dest, destSize, L"\\");
    }
    safeWstrcat(dest, destSize, tail);
}
