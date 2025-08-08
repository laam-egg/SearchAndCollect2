#include "SearchAndCollect2/collect.h"
#include "SearchAndCollect2/common.h"
#include <windows.h>

#define THROW(ERROR_CODE) err = ERROR_CODE; goto cleanup;
#define TRAP(ONCE_REFERENCED_VALUE) err = ONCE_REFERENCED_VALUE; if (err != ERR_NONE) { THROW(err) }

#define BUF_SIZE 1048576 // 1 MB buffer size

ErrorCode Collector_Init(CollectContext* const ctx) {
    ErrorCode err = ERR_NONE;

    {
        HasherConfig config;
        memset(&config, 0, sizeof(HasherConfig));
        config.algorithm = Hasher_SHA256;
        TRAP(Hasher_Init(&ctx->hasher, &config));
    }

    cleanup:
    return err;
}

void Collector_Close(CollectContext* const ctx) {
    Hasher_Close(&ctx->hasher);
}

// 32 bytes
static wchar_t const* const TEMP_FILE_NAME = L"this-pe-is-being-processed.exe";

ErrorCode Collector_Run(CollectContext* const ctx, wchar_t const* const inputFilePath, wchar_t const* const outputFileDir) {
    ErrorCode err = ERR_NONE;

    wchar_t* const tempFilePath = ctx->tempPath;
    wchar_t* const finalFilePath = ctx->tempPath2;
    wchar_t* const hexHashOutputBuf = ctx->tempHashBuf;

    HANDLE hSrc = INVALID_HANDLE_VALUE;
    HANDLE hDest = INVALID_HANDLE_VALUE;
    BYTE* buf = NULL;
    HashingContext* lpHashingContext = NULL;
    size_t hashOutputSize = 0;
    BYTE* hashOutput = NULL;

    {
        // OPEN INPUT FILE
        hSrc = CreateFileW(
            inputFilePath,
            GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL
        );
        if (hSrc == INVALID_HANDLE_VALUE) {
            debug(L"ERROR: collect(): cannot open file; GetLastError() = %lu; skipping %ls", GetLastError(), inputFilePath);
            THROW(ERR_WINAPI)
        }
    }

    {
        // OPEN OUTPUT FILE
        wcsncpy(tempFilePath, outputFileDir, MY_MAX_PATH_LENGTH);
        // TODO: add long-path-prefix \\?\ here
        concatPaths(tempFilePath, MY_MAX_PATH_LENGTH, TEMP_FILE_NAME);

        hDest = CreateFileW(
            tempFilePath,
            GENERIC_WRITE,
            0, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL
        );
        if (hDest == INVALID_HANDLE_VALUE) {
            debug(L"ERROR: collect() cannot create temp file: %ls; GetLastError() = %lu", tempFilePath, GetLastError());
            THROW(ERR_WINAPI)
        }
    }

    {
        // ALLOCATE BUFFER FOR READING
        buf = (BYTE*)VirtualAlloc(NULL, BUF_SIZE, MEM_COMMIT, PAGE_READWRITE);
        if (!buf) {
            debug(L"ERROR: collect(): failed to allocate `buf` through VirtualAlloc(); GetLastError() = %lu");
            THROW(ERR_OUT_OF_MEMORY)
        }
    }

    {
        // OPEN HASHING CONTEXT
        lpHashingContext = &ctx->hashingContext;
        memset(lpHashingContext, 0, sizeof(HashingContext));
        err = Hasher_BeginHashing(&ctx->hasher, lpHashingContext, &hashOutputSize);
        if (err != ERR_NONE) {
            lpHashingContext = NULL;
            THROW(err)
        }
        if (hashOutputSize / sizeof(BYTE) > MY_MAX_HASH_LENGTH) {
            debug(
                L"ERROR: hashOutputSize = %zu IS GREATER THAN MY_MAX_HASH_LENGTH = %zu\nThis is unacceptable; please contact the developer.",
                hashOutputSize,
                MY_MAX_HASH_LENGTH
            );
        }
    }

    {
        // ALLOCATE BUFFER FOR STORING HASH OUTPUT
        hashOutput = (BYTE*)VirtualAlloc(NULL, hashOutputSize, MEM_COMMIT, PAGE_READWRITE);
        if (!hashOutput) {
            debug(L"ERROR: collect(): failed to allocate `hashOutput` through VirtualAlloc(); GetLastError() = %lu");
            THROW(ERR_OUT_OF_MEMORY)
        }
    }

    {
        // READ, CALCULATE HASH AND WRITE (COPY)
        DWORD bytesRead = 0;
        DWORD bytesWritten = 0;
        while (FALSE != ReadFile(hSrc, buf, BUF_SIZE, &bytesRead, NULL) && bytesRead > 0) {
            TRAP(HashingContext_DigestBlock(lpHashingContext, buf, bytesRead))

            if (FALSE == WriteFile(hDest, buf, bytesRead, &bytesWritten, NULL)) {
                debug(L"ERROR: Collector_Run(): failed to WriteFile(): GetLastError() = %lu", GetLastError());
                THROW(ERR_WINAPI)
            }

            if (bytesWritten != bytesRead) {
                debug(L"ERROR: Collector_Run(): number of bytes written is less than bytes read");
                THROW(ERR_UNKNOWN)
            }
        }
    }

    {
        // RENAME FILE AFTER THE HASH
        CloseHandle(hDest); hDest = INVALID_HANDLE_VALUE;
        CloseHandle(hSrc);  hSrc  = INVALID_HANDLE_VALUE;
        TRAP(HashingContext_FinishHashing(lpHashingContext, hashOutput, hashOutputSize));
        hashToHex(hashOutput, hashOutputSize, hexHashOutputBuf);
        wcsncpy(finalFilePath, outputFileDir, MY_MAX_PATH_LENGTH); // TODO: long-path-prefix
        concatPaths(finalFilePath, MY_MAX_PATH_LENGTH, hexHashOutputBuf);
        {
            wchar_t const* const fileExtension = getFileExtension(inputFilePath);
            if (wcsnlen(fileExtension, 1) > 0) {
                safeWstrcat(finalFilePath, MY_MAX_PATH_LENGTH, L".");
                safeWstrcat(finalFilePath, MY_MAX_PATH_LENGTH, fileExtension);
            }
        }

        if (FALSE == MoveFileExW(tempFilePath, finalFilePath, MOVEFILE_REPLACE_EXISTING)) {
            debug(L"ERROR: Collector_Run(): failed to MoveFileExW() from \"%ls\" to \"%ls\" ; GetLastError() = %lu",
                tempFilePath,
                finalFilePath,
                GetLastError()
            );
            THROW(ERR_WINAPI)
        }
    }

    wprintf(L"   [Written to] >> %ls\n", finalFilePath);

    cleanup:
        if (hashOutput != NULL) {
            VirtualFree(hashOutput, 0, MEM_RELEASE);
        }
        if (lpHashingContext != NULL) {
            HashingContext_CloseForcefully(lpHashingContext);
        }
        if (buf != NULL) {
            VirtualFree(buf, 0, MEM_RELEASE);
        }
        if (hDest != INVALID_HANDLE_VALUE) {
            CloseHandle(hDest);
        }
        if (hSrc != INVALID_HANDLE_VALUE) {
            CloseHandle(hSrc);
        }

    return err;
}
