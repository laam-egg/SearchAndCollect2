/**
 * (Non-recursive) filesystem traversal under a single directory.
 */

#ifndef SearchAndCollect2_FILE_SCANNER_INCLUDED
#define SearchAndCollect2_FILE_SCANNER_INCLUDED

#include <stddef.h>
#include <windows.h>
#include "SearchAndCollect2/error_codes.h"
#include "SearchAndCollect2/common.h"

/**
 * Used in ScannedFile
 */
typedef enum {
    IS_UNKNOWN = 0,
    IS_FILE,
    IS_DIRECTORY
} ScannedFileType;

/**
 * All members are public
 */
typedef struct {
    wchar_t fileName[MY_MAX_FILE_NAME_LENGTH];
    ScannedFileType type;
} ScannedFile;

/**
 * Represents a FileScanner object.
 * Public use, however all members are private.
 */
typedef struct {
    HANDLE hSearch;
    WIN32_FIND_DATAW tempFindData;
    int firstFileIterated;
} FileScanner;

ErrorCode FileScanner_Init(FileScanner* const scanner, wchar_t const* const startPath);

ErrorCode FileScanner_Next(FileScanner* const scanner, ScannedFile* const scannedFile);

void FileScanner_Close(FileScanner* const scanner);

#endif // SearchAndCollect2_FILE_SCANNER_INCLUDED
