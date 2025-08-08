#include "SearchAndCollect2/hashing.h"

#pragma comment(lib, "bcrypt.lib")
#include <ntstatus.h>

#define THROW(ERROR_CODE) err = ERROR_CODE; goto cleanup;

ErrorCode Hasher_Init(Hasher* const hasher, HasherConfig* const lpConfig) {
    ErrorCode err = ERR_NONE;
    memset(hasher, 0, sizeof(Hasher));
    {
        memcpy(&hasher->config, lpConfig, sizeof(HasherConfig));
    }

    if (hasher->config.algorithm != Hasher_SHA256) {
        debug(L"ERROR: HasherInit(): unsupported algorithm: %ls", hasher->config.algorithm);
        THROW(ERR_INVALID_ARGUMENT)
    }

    BCRYPT_ALG_HANDLE* lphAlg = &hasher->hAlg;
    BCRYPT_ALG_HANDLE hAlg = NULL;
    DWORD dwHashLength = 0;
    DWORD dwObjLength = 0;
    DWORD cbData = 0;
    if (STATUS_SUCCESS != BCryptOpenAlgorithmProvider(lphAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0)) {
        debug(L"ERROR: HasherInit(): call to BCryptOpenAlgorithmProvider() failed");
        THROW(ERR_WINAPI)
    }
    hAlg = *lphAlg;

    if (STATUS_SUCCESS != BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&dwHashLength, sizeof(dwHashLength), &cbData, 0)) {
        debug(L"ERROR: HasherInit(): call to BCryptGetProperty(...BCRYPT_HASH_LENGTH...) failed");
        THROW(ERR_WINAPI)
    }

    if (STATUS_SUCCESS != BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&dwObjLength, sizeof(dwObjLength), &cbData, 0)) {
        debug(L"ERROR: HasherInit(): call to BCryptGetProperty(...BCRYPT_OBJECT_LENGTH...) failed");
        THROW(ERR_WINAPI)
    }
    hasher->dwHashLength = dwHashLength;
    hasher->dwObjLength = dwObjLength;

    cleanup:
    if (err != ERR_NONE) {
        if (hAlg != NULL) BCryptCloseAlgorithmProvider(hAlg, 0);
    }
    return err;
}

void Hasher_Close(Hasher* const hasher) {
    BCryptCloseAlgorithmProvider(hasher->hAlg, 0);
}

ErrorCode Hasher_BeginHashing(Hasher const* const hasher, HashingContext* const ctx, size_t* lpHashOutputSize) {
    ErrorCode err = ERR_NONE;
    NTSTATUS stt = STATUS_SUCCESS;

    memset(ctx, 0, sizeof(HashingContext));

    {
        ctx->pHasher = hasher;
    }

    BYTE* lpHashCNGObj = NULL;
    {
        lpHashCNGObj = ctx->lpHashCNGObj = (BYTE*)malloc(hasher->dwObjLength * sizeof(BYTE));
        if (lpHashCNGObj == NULL) {
            THROW(ERR_OUT_OF_MEMORY)
        }
    }

    {
        stt = BCryptCreateHash(hasher->hAlg, &ctx->hHash, lpHashCNGObj, hasher->dwObjLength * sizeof(BYTE), NULL, 0, 0);
        if (STATUS_SUCCESS != stt) {
            debug(L"ERROR: Hasher_BeginHashing(): call to BCryptCreateHash() failed with return code %lu", stt);
            THROW(ERR_WINAPI)
        }
    }

    lpHashOutputSize[0] = (size_t)(hasher->dwHashLength) * sizeof(BYTE);

    cleanup:
    if (err != ERR_NONE) {
        if (ctx->hHash != NULL) {
            BCryptDestroyHash(ctx->hHash);
        }
        if (lpHashCNGObj != NULL) {
            free((void*)lpHashCNGObj);
        }
    }
    return err;
}

ErrorCode HashingContext_DigestBlock(HashingContext* const ctx, void* data, size_t dataSize) {
    ErrorCode err = ERR_NONE;
    NTSTATUS stt = STATUS_SUCCESS;

    stt = BCryptHashData(ctx->hHash, data, dataSize, 0);
    if (STATUS_SUCCESS != stt) {
        debug(L"ERROR: HashingContext_DigestBlock(): call to BCryptHashData() failed with code %lu", stt);
        THROW(ERR_WINAPI)
    }

    cleanup:
    return err;
}

ErrorCode HashingContext_FinishHashing(HashingContext* const ctx, BYTE* const lpHashOutput, size_t const hashOutputSize) {
    ErrorCode err = ERR_NONE;

    if (STATUS_SUCCESS != BCryptFinishHash(ctx->hHash, lpHashOutput, hashOutputSize, 0)) {
        debug(L"ERROR: HashingContext_FinishHashing(): call to BCryptFinishHash() failed");
        THROW(ERR_WINAPI)
    }

    cleanup:
        HashingContext_CloseForcefully(ctx);
    return err;
}

void HashingContext_CloseForcefully(HashingContext* const ctx) {
    BCryptDestroyHash(ctx->hHash);
}

// Cre ChatGPT
void hashToHex(BYTE const* hash, DWORD len, wchar_t *out) {
    static const wchar_t hex[] = L"0123456789abcdef";
    for (DWORD i = 0; i < len; i++) {
        out[i*2]     = hex[hash[i] >> 4];
        out[i*2 + 1] = hex[hash[i] & 0xF];
    }
    out[len * 2] = 0;
}
