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

typedef enum {
    DFSFileScanner_DEFAULT = 0,
    /**
     * If set, DFSFileScanner_Next() will not enter subdirectories
     * to which access is denied. However, DFSFileScanner_Init()
     * would still fail if access to `startPath` is denied; also,
     * DFSFileScanner_Next() would still fail if access to the
     * enumerated file at the time is denied. In both cases, the
     * functions would return ERR_ACCESS_DENIED. You might just
     * ignore the error then.
     */
    DFSFileScanner_IGNORE_ACCESS_DENIED_SUBDIRECTORIES
} DFSFileScannerFlags;

typedef struct {
    DFSFileScannerFlags flags;
    int maxDepth;
} DFSFileScannerConfig;

/**
 * Represents a FileScanner object.
 * Public use, however all members are private.
 */
typedef struct {
    LkPathElement_List* pathElementStack;
    ScannedFile tempScannedFile;
    DFSFileScannerConfig config;
    int rebuildCurrentPrefix;
    wchar_t startPath[MY_MAX_PATH_LENGTH];
    wchar_t currentPrefix[MY_MAX_PATH_LENGTH];
    wchar_t tempPath1[MY_MAX_PATH_LENGTH];
} DFSFileScanner;

#define DFSFileScanner_NO_MAX_DEPTH 0

ErrorCode DFSFileScanner_Init(DFSFileScanner* const scanner, wchar_t const* const startPath, DFSFileScannerConfig const* const config);

ErrorCode DFSFileScanner_Next(DFSFileScanner* const scanner, DFSScannedFile* const scannedFile);

void DFSFileScanner_Close(DFSFileScanner* const scanner);

#endif // SearchAndCollect2_DFS_FILE_SCANNER_INCLUDED
