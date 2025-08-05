#include "SearchAndCollect2/filescanner.h"
#include "SearchAndCollect2/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winstring.h>

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

ErrorCode FileScanner_Init(FileScanner* const scanner, wchar_t const* const _in_startPath, FileScannerInitConfig const* const _in_config) {
    memset(scanner, 0, sizeof(FileScanner));

    if (_in_config->flags & FILESCANNER_MULTITHREADED) {
        scanner->hMutex = CreateMutexW(NULL, FALSE, NULL);
        if (scanner->hMutex == NULL) {
            debug(L"FileScanner_Init: failed to initialize mutex");
            return ERR_WINAPI;
        }
    } else {
        scanner->hMutex = NULL;
    }

    wchar_t startPath[MY_MAX_PATH_LENGTH];
    {
        static wchar_t const* LONG_PATH_PREFIX = L"\\\\?\\";

        wchar_t const* prefix = L"";
        if (0 != wcsncmp(_in_startPath, LONG_PATH_PREFIX, wcslen(LONG_PATH_PREFIX))) {
            prefix = LONG_PATH_PREFIX;
        }

        wchar_t lastCharIn_in_startPath = _in_startPath[wcslen(_in_startPath) - 1];
        int _in_startPath_hasTrailingSlashAlready = (lastCharIn_in_startPath == L'\\');
        wchar_t const* suffix = L"";

        if (_in_startPath_hasTrailingSlashAlready) {
            suffix = L"*";
        } else {
            suffix = L"\\*";
        }

        swprintf(startPath, MY_MAX_PATH_LENGTH, L"%ls%ls%ls", prefix, _in_startPath, suffix);
    }

    {
        LPWIN32_FIND_DATAW lpFindData = &scanner->tempFindData;
        scanner->hSearch = FindFirstFileW(startPath, lpFindData);
        if (scanner->hSearch == INVALID_HANDLE_VALUE) {
            if (ERROR_FILE_NOT_FOUND == GetLastError()) {
                debug(L"FileScanner_Init: No such file or directory: %ls", startPath);
                return ERR_NONE;
            } else {
                debug(L"FileScanner_Init: failed to open path: %ls",startPath);
                return ERR_WINAPI;
            }
        }
        
        scanner->firstScannedFile = (ScannedFile*)malloc(sizeof(ScannedFile));
        if (scanner->firstScannedFile == NULL) {
            return ERR_OUT_OF_MEMORY;
        }
        _private_assignScannedFile(scanner->firstScannedFile, &scanner->tempFindData);
    }

    {
        memcpy(&scanner->config, _in_config, sizeof(FileScannerInitConfig));
    }

    return ERR_NONE;
}

void FileScanner_Close(FileScanner* scanner) {
    if (scanner->hMutex != NULL) {
        if (ERR_NONE != WaitMutexSafe(scanner->hMutex, INFINITE)) {
            return; // erroneous state, abort
        }
    }

    {
        FindClose(scanner->hSearch);
        scanner->hSearch = NULL;
    }

    {
        if (scanner->firstScannedFile != NULL) {
            free((void*)scanner->firstScannedFile);
            scanner->firstScannedFile = NULL;
        }
    }

    if (scanner->hMutex != NULL) {
        ReleaseMutex(scanner->hMutex);
        CloseHandle(scanner->hMutex);
    }
    scanner->hMutex = NULL;
}

#define FileScanner_WRAP(UNSAFE_FUNCTION_CALL) \
    if (scanner->hMutex != NULL) { \
        if (ERR_NONE != WaitMutexSafe(scanner->hMutex, INFINITE)) { \
            return ERR_WAITMUTEX; \
        } \
    } \
    \
    ErrorCode err = UNSAFE_FUNCTION_CALL; \
    \
    if (scanner->hMutex != NULL) { \
        ReleaseMutex(scanner->hMutex); \
    } \
    return err;

ErrorCode _unsafe_FileScanner_Next(FileScanner* scanner, ScannedFile* lpScannedFile) {
    if (scanner->hSearch == INVALID_HANDLE_VALUE) {
        return STOP_ITERATION;
    }

    if (scanner->firstScannedFile != NULL) {
        memcpy(lpScannedFile, scanner->firstScannedFile, sizeof(ScannedFile));
        free((void*)scanner->firstScannedFile);
        scanner->firstScannedFile = NULL;
    } else {
        LPWIN32_FIND_DATAW lpFindData = &scanner->tempFindData;
        WINBOOL ok = FindNextFileW(scanner->hSearch, lpFindData);
        if (!ok) {
            if (GetLastError() == ERROR_NO_MORE_FILES) {
                return STOP_ITERATION;
            } else {
                return ERR_WINAPI;
            }
        } else {
            _private_assignScannedFile(lpScannedFile, lpFindData);
        }
        if (lpScannedFile->type == IS_DIRECTORY) {
            
        }
    }
    return ERR_NONE;
}
ErrorCode FileScanner_Next(FileScanner* scanner, ScannedFile* lpScannedFile) {
    FileScanner_WRAP(_unsafe_FileScanner_Next(scanner, lpScannedFile));
}
