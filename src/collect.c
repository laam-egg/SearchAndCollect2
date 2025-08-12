#include "SearchAndCollect2/collect.h"
#include "SearchAndCollect2/common.h"
#include <windows.h>

#define THROW(ERROR_CODE) err = ERROR_CODE; goto cleanup;
#define TRAP(ONCE_REFERENCED_VALUE) err = ONCE_REFERENCED_VALUE; if (err != ERR_NONE) { THROW(err) }

#define BUF_SIZE 1048576 // 1 MB buffer size

ErrorCode Collector_Init(CollectContext* const ctx) {
    ErrorCode err = ERR_NONE;

    memset(ctx, 0, sizeof(CollectContext));

    {
        HasherConfig config;
        memset(&config, 0, sizeof(HasherConfig));
        config.algorithm = Hasher_SHA256;
        TRAP(Hasher_Init(&ctx->hasher, &config));
    }

    {
        size_t hashOutputSize = Hasher_GetHashOutputSize(&ctx->hasher);
        if (hashOutputSize / sizeof(BYTE) > MY_MAX_HASH_LENGTH) {
            debug(
                L"ERROR: hashOutputSize = %zu IS GREATER THAN MY_MAX_HASH_LENGTH = %zu",
                hashOutputSize,
                MY_MAX_HASH_LENGTH
            );
            THROW(ERR_DEVELOPER_FAULT)
        }
    }

    {
        ctx->buf = NULL;
        // ALLOCATE BUFFER FOR READING FILE CONTENT
        ctx->buf = (BYTE*)VirtualAlloc(NULL, BUF_SIZE, MEM_COMMIT, PAGE_READWRITE);
        if (!ctx->buf) {
            debug(L"ERROR: collect(): failed to allocate `buf` through VirtualAlloc(); GetLastError() = %lu");
            THROW(ERR_OUT_OF_MEMORY)
        }
    }

    cleanup:
        if (err != ERR_NONE) {
            Collector_Close(ctx);
        }

    return err;
}

void Collector_Close(CollectContext* const ctx) {
    Hasher_Close(&ctx->hasher); // null-safe

    if (ctx->buf != NULL) {
        VirtualFree(ctx->buf, 0, MEM_RELEASE);
    }

    memset(ctx, 0, sizeof(CollectContext));
}

ErrorCode Collector_Run(CollectContext* const ctx, wchar_t const* const inputFilePath, wchar_t const* const outputFileDir) {
    ErrorCode err = ERR_NONE;

    wchar_t* const tempFilePath = ctx->tempPath;
    wchar_t* const renamedFilePath = ctx->tempPath2;

    HANDLE hSrc = INVALID_HANDLE_VALUE;
    HANDLE hDest = INVALID_HANDLE_VALUE;
    HashingContext* lpHashingContext = NULL;

    DWORD totalRandomBytesWritten = 0;

    {
        // OPEN INPUT FILE
        hSrc = CreateFileW(
            inputFilePath,
            GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL
        );
        if (hSrc == INVALID_HANDLE_VALUE) {
            debug(L"ERROR: Collector_Run(): cannot open file; GetLastError() = %lu; skipping %ls", GetLastError(), inputFilePath);
            THROW(ERR_WINAPI)
        }
    }

    {
        // OPEN OUTPUT FILE
        // Currently: use the original file name; if file in use then number it from 1

        for (int i = 0; ; ++i) {
            {
                // BEGIN CONSTRUCTING tempFilePath
                wcsncpy(tempFilePath, L"\\\\?\\", MY_MAX_PATH_LENGTH); // support for long paths
                safeWstrcat(tempFilePath, MY_MAX_PATH_LENGTH, outputFileDir);
                wchar_t const* originalFileName = getFileNameFromPath(inputFilePath);
                concatPaths(tempFilePath, MY_MAX_PATH_LENGTH, originalFileName);
                if (i > 0) {
                    // Prior conflict (see below) so append a number into file name, keep the extension
                    wchar_t const* originalFileExtension = getFileExtension(inputFilePath);
                    size_t numCharsContainingFileExtension = wcsnlen(originalFileExtension, MY_MAX_PATH_LENGTH);
                    // Print the number into the position of the dot before the extension (if there is any extension).
                    size_t N = wcsnlen(tempFilePath, MY_MAX_PATH_LENGTH);
                    size_t offset = N;
                    if (N > numCharsContainingFileExtension) { // elaborate check A>B instead of max(0, A-B) to avoid integer overflow
                        offset = N - numCharsContainingFileExtension;
                    }
                    size_t remainingBufSize = 0;
                    if (MY_MAX_PATH_LENGTH > offset + 1) {
                        remainingBufSize = MY_MAX_PATH_LENGTH - offset - 1;
                    }
                    swprintf(tempFilePath + offset, remainingBufSize, L"%d", i);
                    // Append the extension, if any
                    if (wcsnlen(originalFileExtension, 1) != 0) {
                        safeWstrcat(tempFilePath, MY_MAX_PATH_LENGTH, L".");
                        safeWstrcat(tempFilePath, MY_MAX_PATH_LENGTH, originalFileExtension);
                    }
                }
                // END CONSTRUCTING tempFilePath
            }

            hDest = CreateFileW(
                tempFilePath,
                GENERIC_WRITE,
                0, NULL,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
            );
            if (hDest == INVALID_HANDLE_VALUE) {
                DWORD dwLastError = GetLastError();
                if (dwLastError == ERROR_SHARING_VIOLATION) {
                    debug(L"Collector_Run(): choosing alternative name for file in use: %ls", tempFilePath);
                } else {
                    debug(L"ERROR: Collector_Run(): cannot create temp file: %ls; GetLastError() = %lu", tempFilePath, dwLastError);
                    THROW(ERR_WINAPI)
                }
            } else {
                break;
            }
        }
    }

    {
        // Pre-allocate size to avoid disk defragmentation.
        // Note that the target file size is equal OR GREATER THAN that of the original file,
        // in case --append-random-bytes is set.

        LARGE_INTEGER originalFileSize;
        if (0 == GetFileSizeEx(hSrc, &originalFileSize)) {
            DWORD dwLastError = GetLastError();
            debug(L"ERROR: isPEFile(): GetLastError() = %lu after calling GetFileSizeEx() on file: %ls", dwLastError, inputFilePath);
            THROW(ERR_WINAPI)
        }

        SetFilePointerEx(hDest, originalFileSize, NULL, FILE_BEGIN);
        SetEndOfFile(hDest);
        // Reset file pointer to the beginning to start writing afterwards
        LARGE_INTEGER zero = { 0 };
        SetFilePointerEx(hDest, zero, NULL, FILE_BEGIN);
    }
    
    int const NAME_AFTER_HASHES = !(Config_Get()->flags & Config_KEEP_ORIGINAL_NAMES);
    if (NAME_AFTER_HASHES) {
        // OPEN HASHING CONTEXT
        lpHashingContext = &ctx->hashingContext;
        memset(lpHashingContext, 0, sizeof(HashingContext));
        err = Hasher_BeginHashing(&ctx->hasher, lpHashingContext);
        if (err != ERR_NONE) {
            lpHashingContext = NULL;
            THROW(err)
        }
    }

    {
        BYTE* const buf = ctx->buf;
        // READ, CALCULATE HASH AND WRITE (COPY)
        DWORD bytesRead = 0;
        DWORD bytesWritten = 0;
        for (;;) {
            if (FALSE == ReadFile(hSrc, buf, BUF_SIZE, &bytesRead, NULL)) {
                DWORD dwLastError = GetLastError();
                if (dwLastError == ERROR_HANDLE_EOF) {
                    break;
                }
                debug(L"ERROR: Collector_Run(): call to ReadFile() failed with GetLastError() = %lu", dwLastError);
                THROW(ERR_WINAPI)
            }

            if (bytesRead == 0) {
                break;
            }

            if (NAME_AFTER_HASHES) {
                TRAP(HashingContext_DigestBlock(lpHashingContext, buf, bytesRead))
            }

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
		// APPEND RANDOM BYTES IF REQUESTED
		if (Config_Get()->flags & Config_APPEND_RANDOM_BYTES) {
			DWORD bytesWritten = 0;
            BYTE* const randomBytes = ctx->randomBytes;
            memset(randomBytes, 0, MIN_RANDOM_BYTES * sizeof(BYTE));
			while (totalRandomBytesWritten < MIN_RANDOM_BYTES || randomNumberInRangeInclusive(0, 100) > 50) {
				for (int i = 0; i < MIN_RANDOM_BYTES; ++i) {
					randomBytes[i] = (BYTE)randomNumberInRangeInclusive(32, 254);
					// Safety measure, might not really need this
					if (randomBytes[i] == 0 || randomBytes[i] == 26) {
						debug(L"WARNING: Collector_Run(): unexpected code path");
						randomBytes[i] = 1;
					}
				}

				if (FALSE == WriteFile(hDest, randomBytes, MIN_RANDOM_BYTES, &bytesWritten, NULL)) {
					debug(L"ERROR: Collector_Run(): failed to WriteFile(): GetLastError() = %lu", GetLastError());
					THROW(ERR_WINAPI)
				}
				if (bytesWritten != MIN_RANDOM_BYTES) {
					debug(L"ERROR: Collector_Run(): number of bytes written is less than bytes read");
					THROW(ERR_UNKNOWN)
				}

				totalRandomBytesWritten += bytesWritten;
			}
		}
	}

    CloseHandle(hDest); hDest = INVALID_HANDLE_VALUE;
    CloseHandle(hSrc);  hSrc = INVALID_HANDLE_VALUE;

    wchar_t const* finalFilePath;
    if (NAME_AFTER_HASHES) {
        // RENAME FILE AFTER THE HASH
        TRAP(HashingContext_FinishHashing(lpHashingContext, ctx->tempHashOutputBuf, MY_MAX_HASH_LENGTH));
        hashToHex(ctx->tempHashOutputBuf, Hasher_GetHashOutputSize(&ctx->hasher), ctx->tempHashHexOutputBuf);
        wcsncpy(renamedFilePath, L"\\\\?\\", MY_MAX_PATH_LENGTH); // long path prefix
        safeWstrcat(renamedFilePath, MY_MAX_PATH_LENGTH, outputFileDir);
        concatPaths(renamedFilePath, MY_MAX_PATH_LENGTH, ctx->tempHashHexOutputBuf);
        {
            wchar_t const* const fileExtension = getFileExtension(inputFilePath);
            if (wcsnlen(fileExtension, 1) > 0) {
                safeWstrcat(renamedFilePath, MY_MAX_PATH_LENGTH, L".");
                safeWstrcat(renamedFilePath, MY_MAX_PATH_LENGTH, fileExtension);
            }
        }

        if (FALSE == MoveFileExW(tempFilePath, renamedFilePath, MOVEFILE_REPLACE_EXISTING)) {
            debug(L"ERROR: Collector_Run(): failed to MoveFileExW() from \"%ls\" to \"%ls\" ; GetLastError() = %lu",
                tempFilePath,
                renamedFilePath,
                GetLastError()
            );
            THROW(ERR_WINAPI)
        }
        finalFilePath = renamedFilePath;
    } else {
        // KEEP ORIGINAL FILE NAME (with appended numbers, if there were conflicts)
        finalFilePath = tempFilePath;
    }

    wprintf(L"   [Written to] >> %ls [+%d random bytes]\n", finalFilePath, totalRandomBytesWritten);

    cleanup:
        if (lpHashingContext != NULL) {
            HashingContext_CloseForcefully(lpHashingContext);
        }
        if (hDest != INVALID_HANDLE_VALUE) {
            CloseHandle(hDest);
        }
        if (hSrc != INVALID_HANDLE_VALUE) {
            CloseHandle(hSrc);
        }

    return err;
}
