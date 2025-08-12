#ifndef SearchAndCollect2_COLLECTORS_INCLUDED
#define SearchAndCollect2_COLLECTORS_INCLUDED

#include "SearchAndCollect2/common.h"
#include "SearchAndCollect2/hashing.h"
#include "SearchAndCollect2/config.h"
#include <wchar.h>

/**
 * In case of --append-random-bytes,
 * append at least this many bytes.
 */
#define MIN_RANDOM_BYTES 16

typedef struct {
    Hasher hasher;
    HashingContext hashingContext;
    BYTE* buf;
    wchar_t tempPath[MY_MAX_PATH_LENGTH + 1];
    wchar_t tempPath2[MY_MAX_PATH_LENGTH + 1];
    BYTE tempHashOutputBuf[MY_MAX_HASH_LENGTH];
    wchar_t tempHashHexOutputBuf[MY_MAX_HASH_LENGTH * 2 + 1]; // multiply by 2 since each byte is denoted by 2 hexadecimal characters
    BYTE randomBytes[MIN_RANDOM_BYTES];
} CollectContext;

ErrorCode Collector_Init(CollectContext* const ctx);

ErrorCode Collector_Run(CollectContext* const ctx, wchar_t const* const inputFilePath, wchar_t const* const outputFileDir);

void Collector_Close(CollectContext* const ctx);

#endif // SearchAndCollect2_COLLECTORS_INCLUDED
