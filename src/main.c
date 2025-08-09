#define VERSION L"0.9.0"

#include "SearchAndCollect2/dfsfilescanner.h"
#include "SearchAndCollect2/common.h"
#include "SearchAndCollect2/pe.h"
#include "SearchAndCollect2/collect.h"
#include "SearchAndCollect2/config.h"
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

BOOL trap(ErrorCode const err, char const* const FILE, int const LINE) {
    if (err != ERR_NONE) {
        DWORD dwLastError = GetLastError();
        wchar_t wFILE[MY_MAX_PATH_LENGTH];
        mbstowcs(wFILE, FILE, MY_MAX_PATH_LENGTH);
        debug(L"Error occurred at file \"%ls\", line %d : error code %d", wFILE, LINE, err);
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

int main(void) {
    SetConsoleOutputCP(CP_UTF8);
    (void)_setmode(_fileno(stdout), _O_U16TEXT);
	srand(time(NULL));

	wprintf(L"SearchAndCollect2.exe VERSION %ls\n", VERSION);
	wprintf(L"https://github.com/laam-egg/SearchAndCollect2\n");

	ErrorCode err = ERR_NONE;

	Config config;
	Config* lpConfig = NULL;
	DFSFileScanner dfsScanner;
	DFSFileScanner* lpDFSScanner = NULL;
	CollectContext collectContext;
	CollectContext* lpCollectContext = NULL;

	{
		// debug(L"INIT: Parsing config");

		err = Config_Init(&config);
		if (err == ERR_COMMAND_LINE_ARGS) {
			printHelp();
			THROW(ERR_COMMAND_LINE_ARGS)
		}
		lpConfig = &config;
	}

    wchar_t const* const inputPath = config.inputPath;
    wchar_t const* const outputDirPath = config.outputDirPath;

    {
		debug(L"INIT: Checking if output directory exists...");
        DWORD attr = GetFileAttributesW(outputDirPath);
        if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            debug(L"ERROR: Cannot open path - it might not exist or we don't have permission to access it: %ls", outputDirPath);
            THROW(ERR_INVALID_ARGUMENT)
        }
    }

	{
		debug(L"INIT: Initializing directory traversal...");
		DFSFileScannerConfig dfsConfig;
		memset(&dfsConfig, 0, sizeof(DFSFileScannerConfig));
		dfsConfig.flags = DFSFileScanner_IGNORE_ACCESS_DENIED_SUBDIRECTORIES;

		TRAP(DFSFileScanner_Init(&dfsScanner, inputPath, &dfsConfig))
		lpDFSScanner = &dfsScanner;
	}

	{
		debug(L"INIT: Initializing file collector...");
		TRAP(Collector_Init(&collectContext));
		lpCollectContext = &collectContext;
	}


	{
		debug(L"STARTING...");
		DFSScannedFile dfsScannedFile;
		for (;;) {
			err = DFSFileScanner_Next(lpDFSScanner, &dfsScannedFile);
			if (err == STOP_ITERATION) {
				err = ERR_NONE;
				break;
			}
			TRAP(err)
			if (dfsScannedFile.type == IS_FILE && isPEFile(dfsScannedFile.filePath)) {
				wprintf(L"[Reading from] << %ls\n", dfsScannedFile.filePath);
				Collector_Run(lpCollectContext, lpConfig, dfsScannedFile.filePath, outputDirPath);
			}
		}
	}

	debug(L"CLEANING UP...");

    cleanup:
        if (lpCollectContext != NULL) {
            Collector_Close(lpCollectContext);
        }
        if (lpDFSScanner != NULL) {
            DFSFileScanner_Close(lpDFSScanner);
        }
		if (lpConfig != NULL) {
			Config_Close(lpConfig);
		}

	if (err != ERR_COMMAND_LINE_ARGS) {
		debug(L"Stopping with error code %d", err);
	}

	if (err != ERR_NONE) {
		return EXIT_FAILURE;
	}
    return EXIT_SUCCESS;
}
#endif // LK_RUN_TEST
