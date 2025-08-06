#ifndef SearchAndCollect2_DFS_FILE_SCANNER_INCLUDED
#define SearchAndCollect2_DFS_FILE_SCANNER_INCLUDED

#include "SearchAndCollect2/filescanner.h"
#include "lklist.h"

typedef struct {
    FileScanner scanner;
    wchar_t content[MY_MAX_FILE_NAME_LENGTH];
} PathElement;

void freePathElement(void* const data);

LK_DECLARE_STRUCT_TYPE(PathElement, PathElement_, , freePathElement)

typedef struct {
    wchar_t filePath[MY_MAX_PATH_LENGTH];
    ScannedFileType type;
} DFSScannedFile;

/**
 * Represents a FileScanner object.
 * Public use, however all members are private.
 */
typedef struct {
    LkPathElement_List* pathElementStack;
    ScannedFile tempScannedFile;
    int maxDepth;
    int rebuildCurrentPrefix;
    wchar_t currentPrefix[MY_MAX_PATH_LENGTH];
} DFSFileScanner;

#define DFSFileScanner_NO_MAX_DEPTH 0

ErrorCode DFSFileScanner_Init(DFSFileScanner* const scanner, wchar_t const* const startPath);

ErrorCode DFSFileScanner_Next(DFSFileScanner* const scanner, DFSScannedFile* const scannedFile);

void DFSFileScanner_Close(DFSFileScanner* const scanner);

#endif // SearchAndCollect2_DFS_FILE_SCANNER_INCLUDED
