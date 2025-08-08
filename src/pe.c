#include "SearchAndCollect2/pe.h"
#include "SearchAndCollect2/common.h"
#include <windows.h>

#ifdef VERBOSE
#define DEBUG debug
#else
#define DEBUG(...) 
#endif

/**
 * Adapted from the original SearchAndCollect source code,
 * with Unicode support and some performance improvements.
 */
BOOL isPEFile(wchar_t const* const filePath) {
    HANDLE fileHandle;
    HANDLE fileMapping;
    LPVOID fileBase;
    PIMAGE_DOS_HEADER dosHeader;

    DEBUG(L"Analyzing file: %ls", filePath);
    
    fileHandle = CreateFileW(
        filePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0
    );
                    
    if (fileHandle == INVALID_HANDLE_VALUE) {
        DEBUG(L"ERROR: isPEFile(): Failed to open file: %ls", filePath);
        return FALSE; 
	}
    
    DWORD dwMaxSizeHi = 0;
    DWORD dwMaxSizeLo = sizeof(IMAGE_DOS_HEADER);
    fileMapping = CreateFileMappingW(fileHandle, NULL, PAGE_READONLY, dwMaxSizeHi, dwMaxSizeLo, NULL);
    if (fileMapping == NULL) {
        DWORD dwLastError = GetLastError();
		CloseHandle(fileHandle);
        if (dwLastError == ERROR_FILE_INVALID) {
            DEBUG(L"Skipping invalid file (maybe zero-sized): %ls", filePath);
        } else {
            DEBUG(L"ERROR: isPEFile(): Failed to create memory mapped to file: %ls", filePath);
        }
        return FALSE; 
	}
    
    fileBase = MapViewOfFile(fileMapping, FILE_MAP_READ, 0, 0, 0);
    if (fileBase == NULL) {
        DWORD dwLastError = GetLastError();
        CloseHandle(fileMapping);
        CloseHandle(fileHandle);
        DEBUG(L"ERROR: isPEFile(): call to MapViewOfFile() failed with GetLastError() = %lu", dwLastError);
        return FALSE;
    }
    
    dosHeader = (PIMAGE_DOS_HEADER)fileBase;
    BOOL result = (dosHeader->e_magic == IMAGE_DOS_SIGNATURE);

    UnmapViewOfFile(fileBase);
    CloseHandle(fileMapping);
    CloseHandle(fileHandle);

	return result;
}
