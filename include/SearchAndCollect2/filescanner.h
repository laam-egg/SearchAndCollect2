/**
 * Supports path traversal in a DFS fashion.
 */

#ifndef SearchAndCollect2_FILE_SCANNER_INCLUDED
#define SearchAndCollect2_FILE_SCANNER_INCLUDED

#include <stddef.h>
#include <windows.h>
#include "SearchAndCollect2/error_codes.h"
#include "SearchAndCollect2/common.h"

/**
 * Used in FileScannerInitConfig
 */
typedef enum {
    FILESCANNER_DEFAULT = 0,
    FILESCANNER_MULTITHREADED = 1
} FileScannerInitFlag;

/**
 * Used in FileScanner_Init()
 */
typedef struct {
    FileScannerInitFlag flags;
} FileScannerInitConfig;


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
 * Public use, however all members are private - not meant to be accessed from outside
 */
typedef struct {
    HANDLE hMutex;
    HANDLE hSearch;
    ScannedFile* firstScannedFile;
    WIN32_FIND_DATAW tempFindData;
    FileScannerInitConfig config;
} FileScanner;

ErrorCode FileScanner_Init(FileScanner* const scanner, wchar_t const* const startPath, FileScannerInitConfig const* const config);

ErrorCode FileScanner_Next(FileScanner* scanner, ScannedFile* scannedFile);

void FileScanner_Close(FileScanner* scanner);

#endif // SearchAndCollect2_FILE_SCANNER_INCLUDED
