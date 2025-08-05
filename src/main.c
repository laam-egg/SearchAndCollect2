#include "SearchAndCollect2/filescanner.h"
#include "SearchAndCollect2/common.h"
#include <stdlib.h>
#include <wchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

void TRAP(ErrorCode err) {
    if (err != ERR_NONE) {
        debug(L"Error occurred - code %d", err);
        exit(1);
    }
}

#ifndef LK_RUN_TEST
int main() {
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U16TEXT);

    FileScanner scanner;
    FileScanner* lpScanner = &scanner;
    ErrorCode err;

    wchar_t const* rootPath = L"C:\\Users\\admin\\Desktop\\_\\OneDrive\\Documents\\Viettel\\";
    FileScannerInitConfig fileScannerConfig;
    fileScannerConfig.flags = FILESCANNER_DEFAULT;
    err = FileScanner_Init(lpScanner, rootPath, &fileScannerConfig);
    TRAP(err);

    ScannedFile scannedFile;
    wchar_t fullPathToFile[MY_MAX_PATH_LENGTH];
    for (;;) {
        err = FileScanner_Next(lpScanner, &scannedFile);
        if (err == STOP_ITERATION) break;
        TRAP(err);

        joinPaths(fullPathToFile, MY_MAX_PATH_LENGTH, rootPath, scannedFile.fileName);

        wprintf(L"Type: %ls | File: %ls\n",
            scannedFile.type == IS_DIRECTORY ? L"DIR" : (scannedFile.type == IS_FILE ? L"FIL" : L"(?)"),
            fullPathToFile
        );
    }

    wprintf(L"DONE.\n");

    return 0;
}
#endif // LK_RUN_TEST
