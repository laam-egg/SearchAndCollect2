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

ErrorCode DFSFileScanner_Init(DFSFileScanner* const scanner, wchar_t const* const startPath) {
    ErrorCode err = ERR_NONE;
    int lkSuccess;

    {
        LkPathElement_List* lpPathElementStack = scanner->pathElementStack = lkPathElement_Init();
        TRAP_NULL(lpPathElementStack, ERR_OUT_OF_MEMORY)
        
        PathElement elem;
        TRAP(FileScanner_Init(&elem.scanner, startPath))
        wcsncpy(elem.content, startPath, MY_MAX_PATH_LENGTH);
        TRAP_LK(lkPathElement_Insert(lpPathElementStack, NULL, &elem))
    }

    {
        wcsncpy(scanner->currentPrefix, startPath, MY_MAX_PATH_LENGTH);
    }

    {
        scanner->maxDepth = 0; // TODO: adjust
    }

    {
        scanner->rebuildCurrentPrefix = 0;
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
    debug(L"DFSFileScanner_Next called with stack size %d", lkPathElement_Size(scanner->pathElementStack));

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
        lkPathElement_Remove(lpPathElementStack, lastElementNode);
        // Reconstruct path + recursion
        scanner->rebuildCurrentPrefix = 1;
        return DFSFileScanner_Next(scanner, lpDFSScannedFile);
    }
    // Handle any other error of FileScanner_Next()
    TRAP(err)

    // Rebuild current path prefix if demanded
    if (scanner->rebuildCurrentPrefix) {
        wchar_t* const currentPath = scanner->currentPrefix;
        currentPath[0] = L'\0';
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
        // If encounter directory, go deeper next time
        if (
            lpDFSScannedFile->type == IS_DIRECTORY
            && !(wcscmp(L".", lpScannedFile->fileName) == 0)
            && !(wcscmp(L"..", lpScannedFile->fileName) == 0)
            && (scanner->maxDepth <= 0 || lkPathElement_Size(lpPathElementStack) < scanner->maxDepth)
        ) {
            PathElement newPathElement;
            wcsncpy(newPathElement.content, lpScannedFile->fileName, MY_MAX_FILE_NAME_LENGTH);
            wcsncpy(scanner->currentPrefix, lpDFSScannedFile->filePath, MY_MAX_PATH_LENGTH);
            TRAP(FileScanner_Init(&newPathElement.scanner, scanner->currentPrefix));
            lkPathElement_Insert(lpPathElementStack, NULL, &newPathElement);
            scanner->rebuildCurrentPrefix = 0;
        }
    }

    return err;
}

#undef TRAP
#undef TRAP_NULL
