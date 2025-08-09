#include "SearchAndCollect2/dfsfilescanner.h"
#include <assert.h>

LK_IMPLEMENT_STRUCT_TYPE(PathElement, PathElement_, , freePathElement)

void freePathElement(void* const data) {
    PathElement* element = (PathElement*)data;
    FileScanner_Close(&element->scanner);
}

#define TRAP(ONCE_REFERENCED_VALUE)             err = ONCE_REFERENCED_VALUE; if (err != ERR_NONE) return err;
#define TRAP_LK(ONCE_REFERENCED_VALUE)          lkSuccess = ONCE_REFERENCED_VALUE; if (!lkSuccess) return ERR_LKLIST;
#define TRAP_NULL(ONCE_REFERENCED_VALUE, ERR)   if (ONCE_REFERENCED_VALUE == NULL) return ERR;

ErrorCode DFSFileScanner_Init(
    DFSFileScanner* const scanner,
    wchar_t const* const startPath,
    DFSFileScannerConfig const* const lpConfig
) {
    ErrorCode err = ERR_NONE;
    int lkSuccess;

    {
        LkPathElement_List* lpPathElementStack = scanner->pathElementStack = lkPathElement_Init();
        TRAP_NULL(lpPathElementStack, ERR_OUT_OF_MEMORY)
        
        PathElement elem;
        TRAP(FileScanner_Init(&elem.scanner, startPath))
        elem.content[0] = L'\0';
        TRAP_LK(lkPathElement_Insert(lpPathElementStack, NULL, &elem))
    }

    {
        wcsncpy(scanner->currentPrefix, startPath, MY_MAX_PATH_LENGTH);
    }

    {
        wcsncpy(scanner->startPath, startPath, MY_MAX_PATH_LENGTH);
    }

    {
        scanner->rebuildCurrentPrefix = 0;
    }

    {
        scanner->config.flags = DFSFileScanner_DEFAULT;
        scanner->config.maxDepth = DFSFileScanner_NO_MAX_DEPTH;
        if (lpConfig != NULL) {
            memcpy(&scanner->config, lpConfig, sizeof(DFSFileScannerConfig));
        }
    }

    return err;
}

void DFSFileScanner_Close(DFSFileScanner* const scanner) {
    lkPathElement_Destroy(scanner->pathElementStack);
}

ErrorCode DFSFileScanner_Next(DFSFileScanner* const scanner, DFSScannedFile* const lpDFSScannedFile) {
    if (lkPathElement_Size(scanner->pathElementStack) == 0) {
        return STOP_ITERATION;
    }

    ErrorCode err = ERR_NONE;

    LkPathElement_List* lpPathElementStack = scanner->pathElementStack;
    TRAP_NULL(lpPathElementStack, ERR_INVALID_HANDLE)

    LkPathElement_Node* lastElementNode = lkPathElement_Tail(lpPathElementStack);
    TRAP_NULL(lastElementNode, ERR_INVALID_HANDLE)

    PathElement* lastPathElement = lkPathElement_GetNodeDataPtr(lpPathElementStack, lastElementNode);
    TRAP_NULL(lastPathElement, ERR_INVALID_HANDLE)

    ScannedFile* lpScannedFile = &scanner->tempScannedFile;
    FileScanner* lpFileScanner = &lastPathElement->scanner;
    err = FileScanner_Next(lpFileScanner, lpScannedFile);
    if (err == STOP_ITERATION) {
        // Done at current level, navigate to the upper directory
		err = ERR_NONE;
        lkPathElement_Remove(lpPathElementStack, lastElementNode);
        // Reconstruct path + recursion
        scanner->rebuildCurrentPrefix = 1;
		// TODO: if there's cleanup later, modify this code
        return DFSFileScanner_Next(scanner, lpDFSScannedFile);
    }
    // Handle any other error of FileScanner_Next()
    TRAP(err)

    // Rebuild current path prefix if demanded
    if (scanner->rebuildCurrentPrefix) {
        wchar_t* const currentPath = scanner->currentPrefix;
        wcsncpy(currentPath, scanner->startPath, MY_MAX_PATH_LENGTH);
        LkPathElement_Node* currentPtr = lkPathElement_Head(lpPathElementStack);
        while (currentPtr != NULL) {
            PathElement* current = lkPathElement_GetNodeDataPtr(lpPathElementStack, currentPtr);
            concatPaths(currentPath, MY_MAX_PATH_LENGTH, current->content);
            lkPathElement_Next(&currentPtr);
        }
        scanner->rebuildCurrentPrefix = 0;
    }

    {
        // Fill in result
        {
            wcsncpy(lpDFSScannedFile->filePath, scanner->currentPrefix, MY_MAX_PATH_LENGTH);
            concatPaths(lpDFSScannedFile->filePath, MY_MAX_PATH_LENGTH, lpScannedFile->fileName);
        }

        {
            lpDFSScannedFile->type = lpScannedFile->type;
        }
    }

    {
        int const maxDepth = scanner->config.maxDepth;
        int const ignoreAccessDeniedSubdirectories = (
            scanner->config.flags & DFSFileScanner_IGNORE_ACCESS_DENIED_SUBDIRECTORIES
        );

        // If encounter directory, go deeper next time
        if (
            lpDFSScannedFile->type == IS_DIRECTORY
            && !(wcscmp(L".", lpScannedFile->fileName) == 0)
            && !(wcscmp(L"..", lpScannedFile->fileName) == 0)
            && (maxDepth <= 0 || lkPathElement_Size(lpPathElementStack) < maxDepth)
        ) {
            PathElement newPathElement;
            wchar_t* const pathToSubdirectory = scanner->tempPath1;
            wcsncpy(pathToSubdirectory, lpDFSScannedFile->filePath, MY_MAX_PATH_LENGTH);
            err = FileScanner_Init(&newPathElement.scanner, pathToSubdirectory);
            if (ignoreAccessDeniedSubdirectories && err == ERR_ACCESS_DENIED) {
                debug(L"Access to directory denied, so skipped: %ls", pathToSubdirectory);
                err = ERR_NONE;
            } else {
                TRAP(err)
                wcsncpy(newPathElement.content, lpScannedFile->fileName, MY_MAX_FILE_NAME_LENGTH);
                wcsncpy(scanner->currentPrefix, pathToSubdirectory, MY_MAX_PATH_LENGTH);
                lkPathElement_Insert(lpPathElementStack, NULL, &newPathElement);
                scanner->rebuildCurrentPrefix = 0;
            }
        }
    }

    return err;
}

#undef TRAP
#undef TRAP_NULL
