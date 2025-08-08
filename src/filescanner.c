#include "SearchAndCollect2/filescanner.h"
#include "SearchAndCollect2/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

void _private_assignScannedFile(ScannedFile* lpScannedFile, LPWIN32_FIND_DATAW lpFindData) {
    memset(lpScannedFile, 0, sizeof(ScannedFile));

    {
        wcsncpy(lpScannedFile->fileName, lpFindData->cFileName, MY_MAX_FILE_NAME_LENGTH);
    }

    {
        DWORD dwFileAttributes = lpFindData->dwFileAttributes;
        ScannedFileType fileType = IS_UNKNOWN;
        if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            fileType = IS_DIRECTORY;
        } else if (!(dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
            fileType = IS_FILE;
        }
        lpScannedFile->type = fileType;
    }
}

ErrorCode FileScanner_Init(FileScanner* const scanner, wchar_t const* const _in_startPath) {
    memset(scanner, 0, sizeof(FileScanner));

    wchar_t startPath[MY_MAX_PATH_LENGTH];
    {
        static wchar_t const* LONG_PATH_PREFIX = L"\\\\?\\";

        wchar_t const* prefix = L"";
        if (0 != wcsncmp(_in_startPath, LONG_PATH_PREFIX, wcslen(LONG_PATH_PREFIX))) {
            prefix = LONG_PATH_PREFIX;
        }

        wchar_t lastCharIn_in_startPath = _in_startPath[wcslen(_in_startPath) - 1];
        swprintf(startPath, MY_MAX_PATH_LENGTH, L"%ls%ls", prefix, _in_startPath);
        concatPaths(startPath, MY_MAX_PATH_LENGTH, L"*");
    }

    {
        LPWIN32_FIND_DATAW lpFindData = &scanner->tempFindData;
        scanner->hSearch = FindFirstFileW(startPath, lpFindData);
        if (scanner->hSearch == INVALID_HANDLE_VALUE) {
            DWORD dwLastError = GetLastError();
            switch (dwLastError) {
                case ERROR_FILE_NOT_FOUND:
                    debug(L"FileScanner_Init: No such file or directory: %ls", startPath);
                    return ERR_NONE;
                
                case ERROR_ACCESS_DENIED:
                    debug(L"FileScanner_Init: Access to directory denied: %ls", startPath);
                    return ERR_ACCESS_DENIED;
                
                default:
                    debug(L"FileScanner_Init: failed to open path: %ls", startPath);
                    return ERR_WINAPI;
            }
        }
    }

    {
        scanner->firstFileIterated = 0;
    }

    return ERR_NONE;
}

void FileScanner_Close(FileScanner* const scanner) {
    if (scanner->hSearch != INVALID_HANDLE_VALUE) {
        FindClose(scanner->hSearch);
        scanner->hSearch = INVALID_HANDLE_VALUE;
    }
}

ErrorCode FileScanner_Next(FileScanner* const scanner, ScannedFile* const lpScannedFile) {
    if (scanner->hSearch == INVALID_HANDLE_VALUE) {
        return STOP_ITERATION;
    }

    WIN32_FIND_DATAW* const lpFindData = &scanner->tempFindData;
    if (!scanner->firstFileIterated) {
        scanner->firstFileIterated = 1;
    } else {
        BOOL ok = FindNextFileW(scanner->hSearch, lpFindData);
        if (!ok) {
            DWORD dwLastError = GetLastError();
            switch (dwLastError) {
                case ERROR_NO_MORE_FILES:
                    FindClose(scanner->hSearch);
                    scanner->hSearch = INVALID_HANDLE_VALUE;
                    return STOP_ITERATION;
                
                case ERROR_ACCESS_DENIED:
                    return ERR_ACCESS_DENIED;
                
                default:
                    return ERR_WINAPI;
            }
        }
    }

    _private_assignScannedFile(lpScannedFile, lpFindData);
    return ERR_NONE;
}
