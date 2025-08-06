#define DFSFileScanner_RUN_TEST

#include "SearchAndCollect2/dfsfilescanner.h"
#include "SearchAndCollect2/common.h"
#include <stdlib.h>
#include <wchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

void trap(ErrorCode const err, char const* const FILE, int const LINE) {
    if (err != ERR_NONE) {
        wchar_t wFILE[MY_MAX_PATH_LENGTH];
        mbstowcs(wFILE, FILE, MY_MAX_PATH_LENGTH);
        debug(L"Error occurred at file \%ls\", line %d : error code %d", wFILE, LINE, err);
        exit(1);
    }
}

#define TRAP(err) trap(err, __FILE__, __LINE__);

#ifndef LK_RUN_TEST
int main() {
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U16TEXT);

    debug(L"Starting");

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

    wprintf(L"DONE.\n");

    #elif defined(DFSFileScanner_RUN_TEST)
    DFSFileScanner dfsScanner;
    DFSFileScanner* lpDFSScanner = &dfsScanner;
    ErrorCode err;

    wchar_t const* rootPath = L"C:\\Users\\admin\\Desktop\\_\\OneDrive\\Documents\\Viettel\\";
    TRAP(DFSFileScanner_Init(lpDFSScanner, rootPath))

    DFSScannedFile dfsScannedFile;
    for (;;) {
        err = DFSFileScanner_Next(lpDFSScanner, &dfsScannedFile);
        if (err == STOP_ITERATION) break;
        TRAP(err)

        wprintf(L"Type: %ls | File: %ls\n",
            dfsScannedFile.type == IS_DIRECTORY ? L"DIR" : (dfsScannedFile.type == IS_FILE ? L"FIL" : L"(?)"),
            dfsScannedFile.filePath
        );
    }

    wprintf(L"DONE.\n");

    #endif

    debug(L"Stopping");

    return 0;
}
#endif // LK_RUN_TEST
