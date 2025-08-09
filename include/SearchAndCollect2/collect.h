#ifndef SearchAndCollect2_COLLECTORS_INCLUDED
#define SearchAndCollect2_COLLECTORS_INCLUDED

#include "SearchAndCollect2/common.h"
#include "SearchAndCollect2/hashing.h"
#include "SearchAndCollect2/config.h"
#include <wchar.h>

typedef struct {
    Hasher hasher;
    HashingContext hashingContext;
    wchar_t tempPath[MY_MAX_PATH_LENGTH];
    wchar_t tempPath2[MY_MAX_PATH_LENGTH];
    wchar_t tempHashBuf[MY_MAX_HASH_LENGTH];
} CollectContext;

ErrorCode Collector_Init(CollectContext* const ctx);

ErrorCode Collector_Run(CollectContext* const ctx, Config const* const config, wchar_t const* const inputFilePath, wchar_t const* const outputFileDir);

void Collector_Close(CollectContext* const ctx);

#endif // SearchAndCollect2_COLLECTORS_INCLUDED
