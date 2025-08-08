#define DFSFileScanner_RUN_TEST

#include "SearchAndCollect2/dfsfilescanner.h"
#include "SearchAndCollect2/common.h"
#include "SearchAndCollect2/pe.h"
#include "SearchAndCollect2/collect.h"
#include <stdlib.h>
#include <wchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

BOOL trap(ErrorCode const err, char const* const FILE, int const LINE) {
    if (err != ERR_NONE) {
        DWORD dwLastError;
        if (err == ERR_WINAPI) {
            dwLastError = GetLastError();
        }
        wchar_t wFILE[MY_MAX_PATH_LENGTH];
        mbstowcs(wFILE, FILE, MY_MAX_PATH_LENGTH);
        debug(L"Error occurred at file \%ls\", line %d : error code %d", wFILE, LINE, err);
        if (err == ERR_WINAPI) {
            debug(L"GetLastError() returned %lu", dwLastError);
        }
        if (err >= 0 && err < NUM_ERROR_MESSAGES) {
            debug(L"Detailed message: %ls", ErrorMessage[err]);
        } else {
            debug(L"Attention: error number out of bound: %d", err);
        }
        return FALSE;
    }
    return TRUE;
}

#define TRAP(ERR) err = ERR; if (!trap(err, __FILE__, __LINE__)) goto cleanup;
#define THROW(ERR) err = ERR; goto cleanup;

#ifndef LK_RUN_TEST

int argc;
wchar_t** argv;

void freeArgv() {
    LocalFree(argv);
}

int main(void) {
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U16TEXT);

    debug(L"Starting");

    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) {
        debug(L"ERROR: CommandLineToArgvW() failed with GetLastError() = %lu", GetLastError());
        return EXIT_FAILURE;
    }
    if (0 != atexit(freeArgv)) {
        debug(L"ERROR: atexit() failed");
        LocalFree(argv);
        return EXIT_FAILURE;
    }

    #ifdef FileScanner_RUN_TEST
    FileScanner scanner;
    FileScanner* lpScanner = &scanner;
    ErrorCode err;

    wchar_t const* rootPath = L"C:\\Users\\admin\\Desktop\\_\\OneDrive\\Documents\\Viettel\\";
    TRAP(FileScanner_Init(lpScanner, rootPath));

    ScannedFile scannedFile;
    wchar_t fullPathToFile[MY_MAX_PATH_LENGTH];
    for (;;) {
        err = FileScanner_Next(lpScanner, &scannedFile);
        if (err == STOP_ITERATION) break;
        TRAP(err)

        joinPaths(fullPathToFile, MY_MAX_PATH_LENGTH, rootPath, scannedFile.fileName);

        wprintf(L"Type: %ls | File: %ls\n",
            scannedFile.type == IS_DIRECTORY ? L"DIR" : (scannedFile.type == IS_FILE ? L"FIL" : L"(?)"),
            fullPathToFile
        );
    }

    FileScanner_Close(lpScanner);

    wprintf(L"DONE.\n");

    #elif defined(DFSFileScanner_RUN_TEST)

    if (argc < 3) {
        wprintf(
            L"Usage:\n"
            L"    SearchAndCollect2 <inputdir> <outputdir>\n"
            L"e.g.\n"
            L"    SearchAndCollect2 C:\\Windows\\System32 D:\\PE\n"
            L"would (recursively) copy every PE file in System32 to the specified PE folder.\n"
            L"\n"
        );
        return EXIT_FAILURE;
    }

    ErrorCode err;

    DFSFileScanner dfsScanner;
    DFSFileScanner* lpDFSScanner = NULL;
    CollectContext collectContext;
    CollectContext* lpCollectContext = NULL;

    // wchar_t const* inputPath = L"C:\\Users\\admin\\Desktop\\_\\OneDrive\\Documents\\Viettel\\_\\Papers\\VCS aJiant\\User guide";
    // wchar_t const* inputPath = L"C:\\Users\\admin\\Desktop\\_\\OneDrive\\Documents\\Viettel\\";
    // wchar_t const* inputPath = L"C:\\Windows\\System32";
    wchar_t const* const inputPath = argv[1];
    wchar_t const* const outputDirPath = argv[2];

    {
        // CHECK IF OUTPUT DIR EXISTS
        DWORD attr = GetFileAttributesW(outputDirPath);
        if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            debug(L"ERROR: Cannot open path - it might not exist or we don't have permission to access it: %ls", outputDirPath);
            THROW(ERR_INVALID_ARGUMENT)
        }
    }

    DFSFileScannerConfig config;
    config.flags = DFSFileScanner_IGNORE_ACCESS_DENIED_SUBDIRECTORIES;

    TRAP(DFSFileScanner_Init(&dfsScanner, inputPath, &config))
    lpDFSScanner = &dfsScanner;

    TRAP(Collector_Init(&collectContext));
    lpCollectContext = &collectContext;

    DFSScannedFile dfsScannedFile;
    for (;;) {
        err = DFSFileScanner_Next(lpDFSScanner, &dfsScannedFile);
        if (err == STOP_ITERATION) break;
        TRAP(err)

        if (dfsScannedFile.type == IS_FILE && isPEFile(dfsScannedFile.filePath)) {
            wprintf(L"[Reading from] << %ls\n", dfsScannedFile.filePath);
            Collector_Run(lpCollectContext, dfsScannedFile.filePath, outputDirPath);
        }
    }

    wprintf(L"DONE.\n");

    cleanup:
        if (lpCollectContext != NULL) {
            Collector_Close(lpCollectContext);
        }
        if (lpDFSScanner != NULL) {
            DFSFileScanner_Close(lpDFSScanner);
        }

    #endif

    debug(L"Stopping");

    return EXIT_SUCCESS;
}
#endif // LK_RUN_TEST
