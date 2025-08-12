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

// Cre ChatGPT
wchar_t const* getFileExtension(wchar_t const* const path) {
    const wchar_t *dot    = wcsrchr(path, L'.');      // last '.'
    const wchar_t *slash1 = wcsrchr(path, L'/');      // last '/' (Unix)
    const wchar_t *slash2 = wcsrchr(path, L'\\');     // last '\' (Windows)
    // Find the last slash
    const wchar_t *slash  = (slash1 > slash2) ? slash1 : slash2;

    if (!dot || (slash && dot < slash)) {
        return L"";  // no extension or dot is in a directory name
    }
    return dot + 1;  // skip '.'
}

// Cre ChatGPT
wchar_t const* getFileNameFromPath(wchar_t const* const path) {
    const wchar_t* last_slash1 = wcsrchr(path, L'\\'); // Windows backslash
    const wchar_t* last_slash2 = wcsrchr(path, L'/');  // Unix forward slash

    // Pick whichever slash is later in the string
    const wchar_t* last_slash = last_slash1;
    if (last_slash2 && (!last_slash || last_slash2 > last_slash)) {
        last_slash = last_slash2;
    }

    // If found, return the part after the slash
    return last_slash ? last_slash + 1 : path;
}

// Cre Gemini
int randomNumberInRangeInclusive(int min, int max) {
	return (rand() % (max - min + 1)) + min;
}
