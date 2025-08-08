#ifndef SearchAndCollect2_HASHING_TOOLS_INCLUDED
#define SearchAndCollect2_HASHING_TOOLS_INCLUDED

#include "SearchAndCollect2/common.h"
#include <windows.h>
#include <bcrypt.h>

typedef enum {
    Hasher_SHA256
} HasherAlgorithm;

typedef struct {
    HasherAlgorithm algorithm;
} HasherConfig;

typedef struct {
    BCRYPT_ALG_HANDLE hAlg;
    DWORD dwHashLength;
    DWORD dwObjLength;
    HasherConfig config;
} Hasher;

typedef struct {
    Hasher const* pHasher;
    BCRYPT_HASH_HANDLE hHash;
    BYTE* lpHashCNGObj;
} HashingContext;

ErrorCode Hasher_Init(Hasher* const hasher, HasherConfig* const config);

void Hasher_Close(Hasher* const hasher);

ErrorCode Hasher_BeginHashing(Hasher const* const hasher, HashingContext* const lpHashingContext, size_t* hashOutputSize);

ErrorCode HashingContext_DigestBlock(HashingContext* const ctx, void* data, size_t dataSize);

ErrorCode HashingContext_FinishHashing(HashingContext* const ctx, BYTE* const lpHashOutput, size_t const hashOutputSize);

void HashingContext_CloseForcefully(HashingContext* const ctx);

// Convert binary hash to lowercase hex string
void hashToHex(BYTE const* hash, DWORD len, wchar_t *out);

#endif // SearchAndCollect2_HASHING_TOOLS_INCLUDED
